# Wyniki testów

## Drugie uruchomienie

- Data: 2026-11-19 21:36:12
- Urządzenie: ESP32-C3-MINI-1
- Adres TCP: 192.168.1.114:2000

## Wyniki

- Test 1, pętla zwrotna `Hello world`: **PASS - 10/10**
- Test 2, wielokrotne połączenia: **PASS - 20/20**
- Test 3, dane TCP: **FAIL - 3/4 rozmiarów**
  - 1 B: **FAIL - brak odpowiedzi**
  - 32 B: **PASS**
  - 1024 B: **PASS**
  - 4096 B: **PASS**
- Test 6, zamknięcie i ponowne połączenie: **PASS - 5/5**
- Test 7, timeout 66 s: **PASS - sesja została zamknięta**

## Test 1 - dodatkowe uruchomienie

- Data: 2026-11-19
- Wynik: **PASS - 10/10 iteracji**


## Test 1 - additional run with diagnostics

- Data: 2026-11-19 21:43:11
- Cel: 192.168.1.114:2000; iteracje: 10

- Iteracja 1: PASS (czas 339 ms)
- Iteracja 2: PASS (czas 34 ms)
- Iteracja 3: PASS (czas 54 ms)
- Iteracja 4: PASS (czas 59 ms)
- Iteracja 5: PASS (czas 62 ms)
- Iteracja 6: PASS (czas 75 ms)
- Iteracja 7: PASS (czas 47 ms)
- Iteracja 8: PASS (czas 50 ms)
- Iteracja 9: PASS (czas 40 ms)
- Iteracja 10: PASS (czas 57 ms)

- Podsumowanie: **10/10 PASS**


## Test 1 - additional run with diagnostics

- Data: 2026-11-19 21:44:57
- Cel: 192.168.1.114:2000; iteracje: 100

- Iteracja 1: PASS (czas 296 ms)
- Iteracja 2: PASS (czas 36 ms)
- Iteracja 3: PASS (czas 47 ms)
- Iteracja 4: PASS (czas 66 ms)
- Iteracja 5: PASS (czas 51 ms)
- Iteracja 6: PASS (czas 60 ms)
- Iteracja 7: PASS (czas 48 ms)
- Iteracja 8: PASS (czas 54 ms)
- Iteracja 9: PASS (czas 60 ms)
- Iteracja 10: PASS (czas 1367 ms)
- Iteracja 11: PASS (czas 66 ms)
- Iteracja 12: PASS (czas 44 ms)
- Iteracja 13: PASS (czas 57 ms)
- Iteracja 14: PASS (czas 57 ms)
- Iteracja 15: PASS (czas 48 ms)
- Iteracja 16: PASS (czas 65 ms)
- Iteracja 17: PASS (czas 41 ms)
- Iteracja 18: PASS (czas 46 ms)
- Iteracja 19: PASS (czas 63 ms)
- Iteracja 20: PASS (czas 49 ms)
- Iteracja 21: PASS (czas 63 ms)
- Iteracja 22: PASS (czas 55 ms)
- Iteracja 23: PASS (czas 44 ms)
- Iteracja 24: PASS (czas 47 ms)
- Iteracja 25: PASS (czas 53 ms)
- Iteracja 26: PASS (czas 48 ms)
- Iteracja 27: PASS (czas 56 ms)
- Iteracja 28: PASS (czas 48 ms)
- Iteracja 29: PASS (czas 47 ms)
- Iteracja 30: PASS (czas 66 ms)
- Iteracja 31: PASS (czas 47 ms)
- Iteracja 32: PASS (czas 62 ms)
- Iteracja 33: PASS (czas 46 ms)
- Iteracja 34: PASS (czas 56 ms)
- Iteracja 35: PASS (czas 51 ms)
- Iteracja 36: PASS (czas 46 ms)
- Iteracja 37: PASS (czas 44 ms)
- Iteracja 38: PASS (czas 50 ms)
- Iteracja 39: PASS (czas 49 ms)
- Iteracja 40: PASS (czas 48 ms)
- Iteracja 41: PASS (czas 61 ms)
- Iteracja 42: PASS (czas 59 ms)
- Iteracja 43: PASS (czas 63 ms)
- Iteracja 44: PASS (czas 58 ms)
- Iteracja 45: PASS (czas 49 ms)
- Iteracja 46: PASS (czas 50 ms)
- Iteracja 47: PASS (czas 47 ms)
- Iteracja 48: PASS (czas 50 ms)
- Iteracja 49: PASS (czas 62 ms)
- Iteracja 50: PASS (czas 59 ms)
- Iteracja 51: PASS (czas 44 ms)
- Iteracja 52: PASS (czas 38 ms)
- Iteracja 53: PASS (czas 53 ms)
- Iteracja 54: PASS (czas 44 ms)
- Iteracja 55: PASS (czas 41 ms)
- Iteracja 56: PASS (czas 46 ms)
- Iteracja 57: PASS (czas 48 ms)
- Iteracja 58: PASS (czas 64 ms)
- Iteracja 59: PASS (czas 48 ms)
- Iteracja 60: PASS (czas 64 ms)
- Iteracja 61: PASS (czas 49 ms)
- Iteracja 62: PASS (czas 48 ms)
- Iteracja 63: PASS (czas 46 ms)
- Iteracja 64: PASS (czas 53 ms)
- Iteracja 65: PASS (czas 47 ms)
- Iteracja 66: PASS (czas 49 ms)
- Iteracja 67: PASS (czas 57 ms)
- Iteracja 68: PASS (czas 121 ms)
- Iteracja 69: PASS (czas 47 ms)
- Iteracja 70: PASS (czas 58 ms)
- Iteracja 71: PASS (czas 66 ms)
- Iteracja 72: PASS (czas 47 ms)
- Iteracja 73: PASS (czas 49 ms)
- Iteracja 74: PASS (czas 48 ms)
- Iteracja 75: PASS (czas 50 ms)
- Iteracja 76: PASS (czas 46 ms)
- Iteracja 77: PASS (czas 56 ms)
- Iteracja 78: PASS (czas 51 ms)
- Iteracja 79: PASS (czas 48 ms)
- Iteracja 80: PASS (czas 53 ms)
- Iteracja 81: PASS (czas 48 ms)
- Iteracja 82: PASS (czas 48 ms)
- Iteracja 83: PASS (czas 45 ms)
- Iteracja 84: PASS (czas 52 ms)
- Iteracja 85: PASS (czas 49 ms)
- Iteracja 86: PASS (czas 60 ms)
- Iteracja 87: PASS (czas 49 ms)
- Iteracja 88: PASS (czas 58 ms)
- Iteracja 89: PASS (czas 65 ms)
- Iteracja 90: PASS (czas 58 ms)
- Iteracja 91: PASS (czas 48 ms)
- Iteracja 92: PASS (czas 49 ms)
- Iteracja 93: PASS (czas 55 ms)
- Iteracja 94: PASS (czas 58 ms)
- Iteracja 95: PASS (czas 47 ms)
- Iteracja 96: PASS (czas 61 ms)
- Iteracja 97: PASS (czas 55 ms)
- Iteracja 98: PASS (czas 50 ms)
- Iteracja 99: PASS (czas 48 ms)
- Iteracja 100: PASS (czas 43 ms)

- Podsumowanie: **100/100 PASS**

## Full run up to --stop--

- Started: 2026-11-19 21:47:57
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.114:2000

Test 1 iteration 1: PASS
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 10/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: FAIL - echo length 0
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: PASS - echo length 1024
Test 3 4096 B: PASS - echo length 4096
Test 3: 3/4 PASS
Test 6: 5/5 PASS
Test 7: PASS - session closed

## Etap 4 — przeciążenie, LED i diagnostyka

- Started: 2026-11-23 19:15
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.113:2000
- Flash przez COM7: **PASS**
- Test 1: **10/10 PASS**
- Test 2: **20/20 PASS**
- Test 3: **4/4 PASS** dla 1, 32, 1024 i 4096 B
- Test 6: **5/5 PASS**
- Test 7: **PASS**
- Powtórny stres transferów: **40/40 PASS**
- Monitor UART/TCP: brak panic, resetu i komunikatów przeciążenia buforów.

## Etap 5 — test akceptacyjny z 2026-11-23

- Finalny build: **PASS**
- Finalny flash przez COM7: **PASS**
- Zimny start i TCP: **PASS** — IP `192.168.1.113`, port `2000`
- Test 1: **10/10 PASS**
- Test 2: **20/20 PASS**
- Test 3: **4/4 PASS**
- Test 6: **5/5 PASS**
- Test 7: **PASS**
- Stabilność po restartach: **10/10 PASS**
- Monitor: brak PANIC, Guru Meditation, resetu lub przeciążenia buforów w przebiegu.




## Test 3 byte-level diagnosis

- Date: 2026-11-19 21:50:18
- TCP target: 192.168.1.114:2000; timeout: 2000 ms

- single ASCII A: FAIL; sent 41, received (empty), 2167 ms
- single NUL: PASS; sent 00, received 00, 140 ms
- single 0xFF: PASS; sent FF, received FF, 47 ms
- two bytes: PASS; sent 41-42, received 41-42, 74 ms
- four bytes: PASS; sent 41-42-43-44, received 41-42-43-44, 52 ms
- 32 bytes: PASS; sent 41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41, received 41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41, 56 ms

- Ten sequential one-byte writes in one TCP session: 9/10 echoed

## Diagnoza niepowodzenia testu 3 (1 B)

- Test klientem PowerShell/.NET: pojedynczy bajt `0x41` nie dotarł do `recv()` na ESP; połączenie i oba zadania sesji startowały, ale urządzenie nie logowało odbioru TCP ani ruchu UART. Ustawienie `TCP_NODELAY` po stronie .NET nie zmieniło wyniku.
- Kontrolna ramka 32 B z tego samego klienta dotarła w całości; logi urządzenia wykazały `TCP RX 32 -> UART TX 32 -> UART RX 32 -> TCP TX 32`.
- Klient Python `socket.sendall()` do tego samego urządzenia: pojedyncze `A` **10/10 PASS** z `TCP_NODELAY` włączonym i wyłączonym. Dokładne echo dla rozmiarów 1, 32, 1024 i 4096 B: **4/4 PASS**.
- Wniosek: wcześniejszy FAIL był fałszywym wynikiem klienta testowego PowerShell/.NET w tym środowisku; nie wskazywał na utratę bajtu w firmware. Dokładna przyczyna zachowania stosu klienta .NET nie została ustalona.
- Tymczasowe logi diagnostyczne usunięto; firmware bez zmian diagnostycznych zbudowano i wgrano. Końcowy test 1 B po przywróceniu firmware: **PASS**.



## Test 3 byte-level diagnosis

- Date: 2026-11-19 21:51:05
- TCP target: 192.168.1.114:2000; timeout: 2000 ms

- single ASCII A: FAIL; sent 41, received (empty), 9234 ms
- single NUL: PASS; sent 00, received 00, 120 ms
- single 0xFF: PASS; sent FF, received FF, 45 ms
- two bytes: FAIL; sent 41-42, received (empty), 2255 ms
- four bytes: FAIL; sent 41-42-43-44, received 41-42-41-42-43-44, 1151 ms
- 32 bytes: PASS; sent 41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41, received 41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41-41, 66 ms
- Isolated 1-byte A trial 1: FAIL; stage receive, sent 41, received (empty), 2045 ms
- Isolated 1-byte A trial 2: FAIL; stage receive, sent 41, received (empty), 2280 ms
- Isolated 1-byte A trial 3: FAIL; stage receive, sent 41, received (empty), 3283 ms
- Isolated 1-byte A trial 4: FAIL; stage receive, sent 41, received (empty), 2114 ms
- Isolated 1-byte A trial 5: FAIL; stage receive, sent 41, received (empty), 2207 ms
- Isolated 1-byte A trial 6: FAIL; stage receive, sent 41, received (empty), 2172 ms
- Isolated 1-byte A trial 7: FAIL; stage receive, sent 41, received (empty), 2068 ms
- Isolated 1-byte A trial 8: FAIL; stage receive, sent 41, received (empty), 2162 ms
- Isolated 1-byte A trial 9: FAIL; stage receive, sent 41, received (empty), 2138 ms
- Isolated 1-byte A trial 10: FAIL; stage receive, sent 41, received (empty), 2183 ms
- Isolated 1-byte A trial 11: FAIL; stage receive, sent 41, received (empty), 2181 ms
- Isolated 1-byte A trial 12: FAIL; stage receive, sent 41, received (empty), 5151 ms
- Isolated 1-byte A trial 13: FAIL; stage receive, sent 41, received (empty), 2161 ms
- Isolated 1-byte A trial 14: FAIL; stage receive, sent 41, received (empty), 2152 ms
- Isolated 1-byte A trial 15: FAIL; stage receive, sent 41, received (empty), 2166 ms
- Isolated 1-byte A trial 16: FAIL; stage receive, sent 41, received (empty), 2114 ms
- Isolated 1-byte A trial 17: FAIL; stage receive, sent 41, received (empty), 2176 ms
- Isolated 1-byte A trial 18: FAIL; stage receive, sent 41, received (empty), 3376 ms
- Isolated 1-byte A trial 19: FAIL; stage receive, sent 41, received (empty), 3349 ms
- Isolated 1-byte A trial 20: FAIL; stage receive, sent 41, received (empty), 2133 ms
- Isolated 1-byte A trial 21: FAIL; stage receive, sent 41, received (empty), 2162 ms
- Isolated 1-byte A trial 22: FAIL; stage receive, sent 41, received (empty), 2143 ms
- Isolated 1-byte A trial 23: FAIL; stage receive, sent 41, received (empty), 2141 ms
- Isolated 1-byte A trial 24: FAIL; stage receive, sent 41, received (empty), 2160 ms
- Isolated 1-byte A trial 25: FAIL; stage receive, sent 41, received (empty), 3390 ms
- Isolated 1-byte A trial 26: FAIL; stage receive, sent 41, received (empty), 2124 ms
- Isolated 1-byte A trial 27: FAIL; stage receive, sent 41, received (empty), 2145 ms
- Isolated 1-byte A trial 28: FAIL; stage receive, sent 41, received (empty), 2166 ms
- Isolated 1-byte A trial 29: FAIL; stage receive, sent 41, received (empty), 2152 ms
- Isolated 1-byte A trial 30: FAIL; stage receive, sent 41, received (empty), 2165 ms
- Isolated 1-byte A summary: 0/30 echoed

- Ten sequential one-byte writes in one TCP session: 9/10 echoed


## Full run up to --stop--

- Started: 2026-11-19 22:17:49
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.114:2000

Test 1 iteration 1: PASS
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 10/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: PASS - echo length 1
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: PASS - echo length 1024
Test 3 4096 B: PASS - echo length 4096
Test 3: 4/4 PASS
Test 6: 5/5 PASS
Test 7: PASS - session closed

## Full run up to --stop--

- Started: 2026-11-19 22:24:34
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.114:2000

Test 1 iteration 1: PASS
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 10/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: PASS - echo length 1
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: PASS - echo length 1024
Test 3 4096 B: PASS - echo length 4096
Test 3: 4/4 PASS
Test 6: 5/5 PASS
Test 7: PASS - session closed

## Full run up to --stop--

- Started: 2026-11-21 16:52:43
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.100:2000

Test 1 iteration 1: FAIL - TCP echo client failed: timed out
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 9/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: PASS - echo length 1
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: FAIL - TCP echo client failed: timed out
Test 3 4096 B: PASS - echo length 4096
Test 3: 3/4 PASS
Test 6 iteration 2: FAIL - no matching echo; received 'close-check'
Test 6: 4/5 PASS
Test 7: PASS - session closed
## Etap 3 — testy sprzętowe z 2026-11-21

- Urządzenie flashowane przez COM7: **PASS**.
- Zimny start: **PASS** — urządzenie połączyło się z Wi-Fi i uzyskało początkowo IP `192.168.1.100`, następnie serwer uruchomił port 2000.
- TCP exact transfer: powtórzone **4/4 PASS** dla 1, 32, 1024 i 4096 B.
- Stabilność sesji: **10/10 PASS** dla powtarzanych transferów `Hello world`.
- Timeout bezczynności: **PASS** — sesja zamknęła się po czasie idle.
- Port TCP po testach: **PASS** — `Test-NetConnection` potwierdził dostępność 192.168.1.100:2000.
- Utrata i przywrócenie Wi-Fi: **PASS częściowy** — logi potwierdziły:
  - rozłączenie AP i przejście w stan reconnect,
  - wielokrotne ponawianie co 5 s,
  - ponowną rejestrację AP i uzyskanie nowego IP `192.168.1.113`.
- Zamknięcie aktywnej sesji podczas utraty Wi-Fi: **NIE POTWIERDZONE** — sesja aktywna została powtórzona po reconnect, ale fizyczne ponowne wyłączenie AP nie zostało wykonane w tym oknie.
- Reset/panic firmware podczas testów: **NIE WYKRYTO**.

## Full run up to --stop--

- Started: 2026-11-23 19:15:24
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.113:2000

Test 1 iteration 1: PASS
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 10/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: PASS - echo length 1
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: PASS - echo length 1024
Test 3 4096 B: PASS - echo length 4096
Test 3: 4/4 PASS
Test 6: 5/5 PASS
Test 7: PASS - session closed

## Full run up to --stop--

- Started: 2026-11-23 19:24:38
- Device: ESP32-C3-MINI-1
- TCP target: 192.168.1.113:2000

Test 1 iteration 1: PASS
Test 1 iteration 2: PASS
Test 1 iteration 3: PASS
Test 1 iteration 4: PASS
Test 1 iteration 5: PASS
Test 1 iteration 6: PASS
Test 1 iteration 7: PASS
Test 1 iteration 8: PASS
Test 1 iteration 9: PASS
Test 1 iteration 10: PASS
Test 1: 10/10 PASS
Test 2: 20/20 PASS
Test 3 1 B: PASS - echo length 1
Test 3 32 B: PASS - echo length 32
Test 3 1024 B: PASS - echo length 1024
Test 3 4096 B: PASS - echo length 4096
Test 3: 4/4 PASS
Test 6: 5/5 PASS
Test 7: PASS - session closed
