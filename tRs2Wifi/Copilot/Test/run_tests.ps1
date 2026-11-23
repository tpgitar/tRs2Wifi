param(
    [string]$Address = "192.168.1.114",
    [int]$Port = 2000
)

function Invoke-Echo {
    param(
        [string]$Payload,
        [int]$TimeoutMs = 3000
    )

    $sent = [System.Text.Encoding]::ASCII.GetBytes($Payload)
    $encodedPayload = [Convert]::ToBase64String($sent)
    $clientPath = Join-Path $PSScriptRoot "tcp_echo_client.py"
    $output = & python $clientPath $Address $Port $TimeoutMs $encodedPayload 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw ("TCP echo client failed: " + ($output -join " "))
    }

    $encodedResponse = ($output | Out-String).Trim()
    $received = [Convert]::FromBase64String($encodedResponse)
    return [System.Text.Encoding]::ASCII.GetString($received)
}

$results = @()
$passCount = 0
for ($i = 1; $i -le 10; $i++) {
    try {
        $response = Invoke-Echo -Payload "Hello world"
        if ($response.Contains("Hello world")) {
            $passCount++
            $results += ("Test 1 iteration {0}: PASS" -f $i)
        } else {
            $results += ("Test 1 iteration {0}: FAIL - no matching echo; received '{1}'" -f $i, $response)
        }
    } catch {
        $results += ("Test 1 iteration {0}: FAIL - {1}" -f $i, $_.Exception.Message)
    }
}
$results += "Test 1: $passCount/10 PASS"

$passCount = 0
for ($i = 1; $i -le 20; $i++) {
    $client = [System.Net.Sockets.TcpClient]::new()
    try {
        $client.Connect($Address, $Port)
        $passCount++
    } catch {
        $results += ("Test 2 iteration {0}: FAIL - {1}" -f $i, $_.Exception.Message)
    } finally {
        $client.Close()
    }
}
$results += "Test 2: $passCount/20 PASS"

$passCount = 0
foreach ($size in @(1, 32, 1024, 4096)) {
    try {
        $builder = New-Object System.Text.StringBuilder
        for ($index = 0; $index -lt $size; $index++) {
            [void]$builder.Append([char](33 + ($index % 90)))
        }
        $payload = $builder.ToString()
        $response = Invoke-Echo -Payload $payload
        if ([string]::Equals($response, $payload, [System.StringComparison]::Ordinal)) {
            $passCount++
            $results += ("Test 3 {0} B: PASS - echo length {1}" -f $size, $response.Length)
        } else {
            $results += ("Test 3 {0} B: FAIL - echo length {1}" -f $size, $response.Length)
        }
    } catch {
        $results += ("Test 3 {0} B: FAIL - {1}" -f $size, $_.Exception.Message)
    }
}
$results += "Test 3: $passCount/4 PASS"

$passCount = 0
for ($i = 1; $i -le 5; $i++) {
    $first = [System.Net.Sockets.TcpClient]::new()
    $second = $null
    try {
        $first.Connect($Address, $Port)
        $firstStream = $first.GetStream()
        $payload = [System.Text.Encoding]::ASCII.GetBytes("close-check")
        $firstStream.Write($payload, 0, $payload.Length)
        $first.Close()
        Start-Sleep -Milliseconds 100

        $second = [System.Net.Sockets.TcpClient]::new()
        $second.ReceiveTimeout = 3000
        $second.Connect($Address, $Port)
        $secondStream = $second.GetStream()
        $payload = [System.Text.Encoding]::ASCII.GetBytes("next")
        $secondStream.Write($payload, 0, $payload.Length)
        $responseFromSecond = ""
        $deadline = [DateTime]::UtcNow.AddMilliseconds(1500)
        $buffer = New-Object byte[] 256
        while ([DateTime]::UtcNow -lt $deadline) {
            if ($secondStream.DataAvailable) {
                $count = $secondStream.Read($buffer, 0, $buffer.Length)
                if ($count -gt 0) {
                    $responseFromSecond += [System.Text.Encoding]::ASCII.GetString($buffer, 0, $count)
                }
            }
            if ($responseFromSecond.Contains("next")) {
                break
            }
            Start-Sleep -Milliseconds 10
        }
        if ($responseFromSecond.Contains("next")) {
            $passCount++
        } else {
            $results += ("Test 6 iteration {0}: FAIL - no matching echo; received '{1}'" -f $i, $responseFromSecond)
        }
    } catch {
        $results += ("Test 6 iteration {0}: FAIL - {1}" -f $i, $_.Exception.Message)
    } finally {
        if ($second) { $second.Close() }
        $first.Close()
    }
}
$results += "Test 6: $passCount/5 PASS"

$idleClient = [System.Net.Sockets.TcpClient]::new()
$idleClosed = $false
try {
    $idleClient.Connect($Address, $Port)
    Start-Sleep -Milliseconds 66000
    $idleStream = $idleClient.GetStream()
    $buffer = New-Object byte[] 1
    try {
        $count = $idleStream.Read($buffer, 0, 1)
        if ($count -eq 0) {
            $idleClosed = $true
        }
    } catch [System.IO.IOException] {
        $idleClosed = $true
    } catch [System.Net.Sockets.SocketException] {
        $idleClosed = $true
    }
} catch {
    $idleClosed = $true
} finally {
    $idleClient.Close()
}
$results += ("Test 7: {0}" -f $(if ($idleClosed) { "PASS - session closed" } else { "FAIL - session still open" }))

$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$header = @(
    "## Full run up to --stop--",
    "",
    "- Started: $timestamp",
    "- Device: ESP32-C3-MINI-1",
    "- TCP target: ${Address}:${Port}",
    ""
)
$lines = $header + $results
[System.IO.File]::AppendAllText(
    (Join-Path $PSScriptRoot "test_results.md"),
    [Environment]::NewLine + ($lines -join [Environment]::NewLine) + [Environment]::NewLine,
    [System.Text.UTF8Encoding]::new($false)
)
$results | Write-Output
