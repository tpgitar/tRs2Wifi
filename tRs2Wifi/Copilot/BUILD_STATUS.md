# Stan builda i testów

**Status: PASS — build, flash i testy funkcjonalne wykonane 2026-11-19.**

## Środowisko

- ESP-IDF: `C:\esp\v6.1\esp-idf` (v6.1), target `esp32c3`.
- Python: istniejący venv `C:\Espressif\tools\python\v6.1\venv`.
- Przy buildzie użyto `IDF_PYTHON_ENV_PATH=C:\Espressif\tools\python\v6.1\venv`, `IDF_PYTHON_CHECK_CONSTRAINTS=no` i wyłączonego ccache.
- Plik constraints środowiska ESP-IDF nie jest dostępny; kontrola wersji constraints została pominięta.

## Ostatni build i flash

- Diagnostyczny build ESP32-C3 zakończył się powodzeniem.
- Tymczasowe logi TCP/UART usunięto z `main/tcpServer.c`.
- Ponownie wykonano `idf.py -p COM7 flash`; esptool potwierdził hash zapisu i zresetował urządzenie.
- Obraz po usunięciu diagnostyki: `0xd7610` B; w partycji aplikacji pozostało `0x289f0` B (16%).
- Końcowe echo pojedynczego bajtu po przywróceniu firmware: PASS.

## Ostatni pełny przebieg testów do `--stop--`

Wyniki zapisano w `Copilot/Test/test_results.md` (przebieg z 2026-11-19 22:24):

- Test 1, pętla `Hello world`: **10/10 PASS**.
- Test 2, połączenia TCP: **20/20 PASS**.
- Test 3, dokładne echo: **4/4 PASS** dla 1, 32, 1024 i 4096 B.
- Test 6, zamykanie i ponowne połączenie: **5/5 PASS**.
- Test 7, bezczynność przez 66 s: **PASS**, sesja została zamknięta.

Testy echo używają `Copilot/Test/tcp_echo_client.py` z Python `socket.sendall()`. Wcześniejszy klient PowerShell/.NET nie odbierał pojedynczego bajtu w tym środowisku; Python z tym samym firmware przeszedł test. Dokładna przyczyna zachowania .NET nie została ustalona.

## Przed kolejnym buildem

- Sprawdzić `git status`, zwłaszcza śledzone pliki w `build/` i zmiany w `main/tcpServer.c`; nie resetować ani nie czyścić bez weryfikacji.
- Zachować target `esp32c3`, chyba że zmieni się potwierdzony moduł.
- Nie publikować ani nie śledzić `sdkconfig.local`, które zawiera lokalne dane Wi-Fi.

## Etap 3 — build

- Dodano obsługę limitu początkowych prób Wi-Fi, opóźnionego ponawiania co 5 s i zachowania po utracie IP.
- Dodano centralne uruchomienie serwera TCP po uzyskaniu IP oraz natychmiastowe zatrzymanie aktywnej sesji przy utracie sieci.
- `idf.py build` dla ESP32-C3 — **PASS** po zmianach Etapu 3.
- Próba `idf.py -p COM7 flash` — **NIEUDANA z powodu niedostępności portu** (`COM7` nie był wykryty w systemie Windows); firmware nie zostało w tej sesji ponownie wgrane.

## Etap 3 — testy urządzenia z 2026-11-21

- Flash przez COM7: **PASS**.
- Zimny start i uzyskanie IP: **PASS** — początkowo `192.168.1.100`, później po reconnect `192.168.1.113`.
- Serwer TCP port 2000: **PASS**.
- Powtarzane dokładne transfery 1/32/1024/4096 B: **4/4 PASS**.
- Powtarzane echo `Hello world`: **10/10 PASS**.
- Timeout bezczynności: **PASS**.
- Utrata i przywrócenie Wi-Fi: **PASS częściowy** — rozłączenie, reconnect co 5 s i nowe IP potwierdzono logami.
- Zamknięcie aktywnej sesji podczas utraty sieci pozostaje do ponownego sprzętowego potwierdzenia.

## Etap 4 — przeciążenie, LED i diagnostyka

- Build ESP-IDF 6.1 dla ESP32-C3: **PASS**.
- Flash przez COM7: **PASS**.
- Synchronizacja kolorów LED i aktywnego TCP używa wspólnej sekcji krytycznej.
- Zadanie LED działa z `vTaskDelayUntil()` co 50 ms i nazwą `task_led_blink`.
- Pełny runner: Test 1 10/10, Test 2 20/20, Test 3 4/4, Test 6 5/5, Test 7 PASS.
- Powtórzone transfery długości 4096/1024/32/1 B: **40/40 PASS**.
- Nie wykryto panic/resetu ani komunikatów przeciążenia.

## Etap 5 — test akceptacyjny i wydanie

- Finalny build ESP-IDF 6.1 dla ESP32-C3: **PASS**.
- Finalny flash przez COM7: **PASS**.
- Zimny start i serwer portu 2000: **PASS**.
- Test 1 10/10, Test 2 20/20, Test 3 4/4, Test 6 5/5, Test 7 PASS.
- Cykl stabilności po restartach: **10/10 PASS**.
- Dokumentacja użytkownika i opis parametrów TCP/UART w [README.md](/E:/Tomek/EspIff_Workspace/tRs2Wifi/tRs2Wifi/README.md): **PASS**.
- Fizyczny Test 10 utraty/odzyskania Wi-Fi pozostaje częściowo potwierdzony z Etapu 3.
