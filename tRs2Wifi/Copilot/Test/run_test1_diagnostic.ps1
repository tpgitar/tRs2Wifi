param(
    [string]$Address = "192.168.1.114",
    [int]$Port = 2000,
    [int]$Iterations = 10,
    [int]$TimeoutMs = 4000
)

function Connect-WithTimeout {
    param(
        [System.Net.Sockets.TcpClient]$Client,
        [int]$Milliseconds
    )

    $async = $Client.BeginConnect($Address, $Port, $null, $null)
    if (-not $async.AsyncWaitHandle.WaitOne($Milliseconds)) {
        $Client.Close()
        throw [System.TimeoutException]::new(
            "TCP connect timed out after ${Milliseconds}ms"
        )
    }
    $Client.EndConnect($async)
}

function Invoke-EchoAttempt {
    param(
        [string]$Payload,
        [int]$Milliseconds
    )

    $client = [System.Net.Sockets.TcpClient]::new()
    $timer = [System.Diagnostics.Stopwatch]::StartNew()
    $stage = "connect"
    $received = New-Object System.Collections.Generic.List[byte]
    $failure = $null

    try {
        Connect-WithTimeout -Client $client -Milliseconds $Milliseconds
        $stage = "send"
        $stream = $client.GetStream()
        $bytes = [System.Text.Encoding]::ASCII.GetBytes($Payload)
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush()
        $stage = "receive"

        $buffer = New-Object byte[] 4096
        $deadline = [DateTime]::UtcNow.AddMilliseconds($Milliseconds)
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
            if ([System.Text.Encoding]::ASCII.GetString($received.ToArray()).Contains($Payload)) {
                break
            }
            Start-Sleep -Milliseconds 10
        }
    } catch {
        $failure = $_.Exception.Message
    } finally {
        $timer.Stop()
        $client.Close()
    }

    $response = [System.Text.Encoding]::ASCII.GetString($received.ToArray())
    $hex = [BitConverter]::ToString($received.ToArray())
    $status = "PASS"
    if ($failure -ne $null -or -not $response.Contains($Payload)) {
        $status = "FAIL"
    }

    [PSCustomObject]@{
        Status = $status
        Stage = $stage
        DurationMs = $timer.ElapsedMilliseconds
        Response = $response
        Hex = $hex
        Error = $failure
    }
}

function Test-TcpListener {
    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        Connect-WithTimeout -Client $client -Milliseconds 2000
        return "reachable"
    } catch {
        return ("unreachable: " + $_.Exception.Message)
    } finally {
        $client.Close()
    }
}

$payload = "Hello world"
$results = New-Object System.Collections.Generic.List[string]
$passes = 0
$results.Add("## Test 1 - additional run with diagnostics")
$results.Add("")
$results.Add("- Data: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$results.Add("- Cel: ${Address}:${Port}; iteracje: $Iterations")
$results.Add("")

for ($iteration = 1; $iteration -le $Iterations; $iteration++) {
    $attempt = Invoke-EchoAttempt -Payload $payload -Milliseconds $TimeoutMs
    if ($attempt.Status -eq "PASS") {
        $passes++
        $line = "Iteracja ${iteration}: PASS (czas $($attempt.DurationMs) ms)"
    } else {
        $line = "Iteracja ${iteration}: FAIL na etapie $($attempt.Stage) po $($attempt.DurationMs) ms."
        if ($attempt.Error) {
            $line += " Error: $($attempt.Error)"
        }
        $line += " Odebrane: '$($attempt.Response)' (hex: $($attempt.Hex))."

        if ($attempt.Stage -eq "connect") {
            $line += " Port check after failure: $(Test-TcpListener)."
        } else {
            $diagnostic = Invoke-EchoAttempt -Payload $payload -Milliseconds $TimeoutMs
            $line += " Diagnostic echo retry: $($diagnostic.Status), stage $($diagnostic.Stage), time $($diagnostic.DurationMs) ms, received '$($diagnostic.Response)' (hex: $($diagnostic.Hex))."
            if ($diagnostic.Error) {
                $line += " Diagnostic error: $($diagnostic.Error)"
            }
        }
    }

    $results.Add("- $line")
    Write-Output $line
}

$results.Add("")
$results.Add("- Podsumowanie: **$passes/$Iterations PASS**")
$results.Add("")

$reportPath = Join-Path $PSScriptRoot "test_results.md"
$reportText = [System.IO.File]::ReadAllText($reportPath)
$appendText = [Environment]::NewLine + [Environment]::NewLine +
    ($results -join [Environment]::NewLine)
[System.IO.File]::AppendAllText(
    $reportPath,
    $appendText,
    [System.Text.UTF8Encoding]::new($false)
)
