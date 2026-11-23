param(
    [string]$Address = "192.168.1.114",
    [int]$Port = 2000,
    [int]$TimeoutMs = 2000
)

function ConvertTo-HexString {
    param([byte[]]$Bytes)
    if ($Bytes.Length -eq 0) {
        return "(empty)"
    }
    return [BitConverter]::ToString($Bytes)
}

function Invoke-ByteEcho {
    param([byte[]]$Payload)

    $client = [System.Net.Sockets.TcpClient]::new()
    $timer = [System.Diagnostics.Stopwatch]::StartNew()
    $received = New-Object System.Collections.Generic.List[byte]
    $stage = "connect"
    $errorText = $null

    try {
        $client.Connect($Address, $Port)
        $stream = $client.GetStream()
        $stage = "send"
        $stream.Write($Payload, 0, $Payload.Length)
        $stage = "receive"
        $buffer = New-Object byte[] 4096
        $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
        while ([DateTime]::UtcNow -lt $deadline) {
            if ($stream.DataAvailable) {
                $count = $stream.Read($buffer, 0, $buffer.Length)
                if ($count -eq 0) {
                    break
                }
                for ($index = 0; $index -lt $count; $index++) {
                    $received.Add($buffer[$index])
                }
            }
            if ($received.Count -ge $Payload.Length) {
                break
            }
            Start-Sleep -Milliseconds 5
        }
    } catch {
        $errorText = $_.Exception.Message
    } finally {
        $timer.Stop()
        $client.Close()
    }

    $bytes = $received.ToArray()
    $match = $bytes.Length -ge $Payload.Length
    if ($match) {
        for ($index = 0; $index -lt $Payload.Length; $index++) {
            if ($bytes[$index] -ne $Payload[$index]) {
                $match = $false
                break
            }
        }
    }

    [PSCustomObject]@{
        Pass = $match
        Stage = $stage
        DurationMs = $timer.ElapsedMilliseconds
        SentHex = ConvertTo-HexString $Payload
        ReceivedHex = ConvertTo-HexString $bytes
        Error = $errorText
    }
}

$cases = @(
    [PSCustomObject]@{ Name = "single ASCII A"; Bytes = [byte[]](0x41) },
    [PSCustomObject]@{ Name = "single NUL"; Bytes = [byte[]](0x00) },
    [PSCustomObject]@{ Name = "single 0xFF"; Bytes = [byte[]](0xFF) },
    [PSCustomObject]@{ Name = "two bytes"; Bytes = [byte[]](0x41, 0x42) },
    [PSCustomObject]@{ Name = "four bytes"; Bytes = [byte[]](0x41, 0x42, 0x43, 0x44) },
    [PSCustomObject]@{ Name = "32 bytes"; Bytes = [byte[]](1..32 | ForEach-Object { 0x41 }) }
)

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("## Test 3 byte-level diagnosis")
$lines.Add("")
$lines.Add("- Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$lines.Add("- TCP target: ${Address}:${Port}; timeout: ${TimeoutMs} ms")
$lines.Add("")

foreach ($case in $cases) {
    $result = Invoke-ByteEcho -Payload $case.Bytes
    $status = if ($result.Pass) { "PASS" } else { "FAIL" }
    $line = "- $($case.Name): $status; sent $($result.SentHex), received $($result.ReceivedHex), $($result.DurationMs) ms"
    if ($result.Error) {
        $line += "; stage $($result.Stage), error $($result.Error)"
    }
    $lines.Add($line)
    Write-Output $line
}

$singleBytePasses = 0
for ($iteration = 1; $iteration -le 30; $iteration++) {
    $result = Invoke-ByteEcho -Payload ([byte[]](0x41))
    if ($result.Pass) {
        $singleBytePasses++
        $line = "- Isolated 1-byte A trial ${iteration}: PASS; received $($result.ReceivedHex), $($result.DurationMs) ms"
    } else {
        $line = "- Isolated 1-byte A trial ${iteration}: FAIL; stage $($result.Stage), sent $($result.SentHex), received $($result.ReceivedHex), $($result.DurationMs) ms"
        if ($result.Error) {
            $line += "; error $($result.Error)"
        }
    }
    $lines.Add($line)
    Write-Output $line
}
$lines.Add("- Isolated 1-byte A summary: $singleBytePasses/30 echoed")
Write-Output ("Isolated one-byte A trials: {0}/30 echoed" -f $singleBytePasses)

$client = [System.Net.Sockets.TcpClient]::new()
$stream = $null
$sessionResults = New-Object System.Collections.Generic.List[string]
try {
    $client.Connect($Address, $Port)
    $stream = $client.GetStream()
    for ($iteration = 1; $iteration -le 10; $iteration++) {
        $payload = [byte[]](0x41)
        $stream.Write($payload, 0, $payload.Length)
        $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
        $received = New-Object System.Collections.Generic.List[byte]
        $buffer = New-Object byte[] 64
        while ([DateTime]::UtcNow -lt $deadline -and $received.Count -lt 1) {
            if ($stream.DataAvailable) {
                $count = $stream.Read($buffer, 0, $buffer.Length)
                for ($index = 0; $index -lt $count; $index++) {
                    $received.Add($buffer[$index])
                }
            }
            if ($received.Count -lt 1) {
                Start-Sleep -Milliseconds 5
            }
        }
        if ($received.Count -gt 0 -and $received[0] -eq 0x41) {
            $sessionResults.Add("PASS")
        } else {
            $sessionResults.Add("FAIL")
        }
    }
} catch {
    $sessionResults.Add(("session error: " + $_.Exception.Message))
} finally {
    $client.Close()
}

$sessionPass = @($sessionResults | Where-Object { $_ -eq "PASS" }).Count
$lines.Add("")
$lines.Add("- Ten sequential one-byte writes in one TCP session: $sessionPass/10 echoed")
Write-Output ("Same-session one-byte writes: {0}/10 echoed" -f $sessionPass)
$lines.Add("")

[System.IO.File]::AppendAllText(
    (Join-Path $PSScriptRoot "test_results.md"),
    [Environment]::NewLine + [Environment]::NewLine +
        ($lines -join [Environment]::NewLine) + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false)
)
