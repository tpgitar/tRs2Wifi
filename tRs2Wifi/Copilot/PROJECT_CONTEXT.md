# Kontekst projektu

## Projekt

- Repozytorium: `tpgitar/tRs2Wifi`.
- Katalog aplikacji ESP-IDF: `tRs2Wifi/`.
- Nazwa projektu w CMake: `app-template`.
- Główny komponent rejestruje pliki: `main.c`, `Led.c`, `Wifi.c`, `tcpServer.c`, `uart.c`.
- Zależność zarządzana: `espressif/led_strip` w wersji `^3.1.1`.
- README w katalogu projektu to nadal ogólny tekst szablonu ESP-IDF.

## Środowisko zauważone podczas pracy

- ESP-IDF: `C:\esp\v6.1\esp-idf` (v6.1).
- Python venv obecny na dysku: `C:\Espressif\tools\python\v6.1\venv`.
- `idf.py build` zakończył się powodzeniem dla targetu `esp32c3`.
- Przy aktywacji istniejącego venv użyto `IDF_PYTHON_ENV_PATH=C:\Espressif\tools\python\v6.1\venv` oraz `IDF_PYTHON_CHECK_CONSTRAINTS=no`, ponieważ zarządzany venv i plik constraints zostały usunięte na prośbę użytkownika.
- Katalog builda: `tRs2Wifi/build`. Przed kolejnym czyszczeniem sprawdzić jego stan w Git.

## Ostrożność przy dalszej pracy

- Katalog `build/` zawiera śledzone przez Git pliki wygenerowane. Nie czyścić ani nie resetować go bez sprawdzenia aktualnego `git status`.
- Target projektu to ESP32-C3; nie zmieniać na `esp32`, chyba że zmieni się docelowy moduł.


## Sprzęt i opis projektu

- Potwierdzone oznaczenie modułu: **ESP32-C3-MINI-1**.
- Model płytki nośnej nieznany. Płytka ma kilka złączy USB-C; jedno służy do programowania i pojawia się w systemie Windows jako **COM7**.
- Nie ustalono jeszcze, czy COM7 jest mostkiem USB–UART podłączonym do UART0, czy natywnym USB Serial/JTAG ESP32-C3. Do rozstrzygnięcia potrzebny jest model płytki lub identyfikator układu USB.
- Projekt tRs2Wifi jest adapterem RS232–Wi-Fi. Po połączeniu klienta TCP dane są przekazywane w obu kierunkach między TCP i UART.

## Porty komunikacyjne

- Serwer TCP nasłuchuje na porcie **2000** (`TCP_PORT` w `main/tcpServer.c`).
- Używany UART to **UART1** (numer portu `1`, ustawiony w `main/uart.c` i `main/tcpServer.c`).
- Konfiguracja UART: 57600 baud, TX GPIO4, RX GPIO5.
- Do debugowania preferować osobną konsolę USB; nie przekierowywać logów na ten sam UART, którym przesyłane są dane RS232.

## Stan na 2026-11-19

- ESP32-C3-MINI-1 był dostępny przez COM7; przy ostatniej weryfikacji adres TCP wynosił `192.168.1.114:2000` (adres DHCP może się zmienić).
- Testy funkcjonalne opisane przed znacznikiem `--stop--` zakończyły się PASS: Test 1 `10/10`, Test 2 `20/20`, Test 3 dokładne echo dla `1/32/1024/4096 B` (`4/4`), Test 6 `5/5`, Test 7 po 66 s bezczynności.
- Runner testów korzysta z `Copilot/Test/tcp_echo_client.py` (`socket.sendall`) do testów echo. Wynik Testu 3 dla 1 bajtu był błędny przy kliencie PowerShell/.NET; próby diagnostyczne wykazały brak odbioru pojedynczego segmentu po stronie tego klienta, natomiast Python przechodził echo bajtu. Nie ustalono dokładnej przyczyny zachowania stosu .NET.
- Tymczasowe logi TCP/UART zostały usunięte z `main/tcpServer.c`. Po usunięciu diagnostyki firmware zbudowano i wgrano przez COM7; końcowe echo 1 B zaliczone.
- Skrypty testowe: `Copilot/Test/run_tests.ps1`, `Copilot/Test/run_test1_diagnostic.ps1` i `Copilot/Test/diagnose_test3.ps1`. Wyniki przebiegów są w `Copilot/Test/test_results.md`.
- `sdkconfig.local` zawiera lokalną konfigurację Wi-Fi i nie należy publikować jego wartości ani dodawać go do śledzenia.

## Stan roboczy i ostrożność

- W trakcie sesji `main/tcpServer.c` miał już zmiany względem HEAD. Tymczasowe logi diagnostyczne dodano i usunięto; zachowaj pozostałe zmiany użytkownika.
- `build/` zawiera śledzone pliki wygenerowane. Przed czyszczeniem lub resetem sprawdź `git status`; nie wykonuj destrukcyjnego czyszczenia bez potwierdzenia.

## Etap 3 — stan wdrożenia

- Zrealizowano ograniczone oczekiwanie początkowe na IP (30 s), limit 10 prób startowych i ponawianie co 5 s.
- Serwer TCP uruchamia się po uzyskaniu IP i uruchamia się ponownie tylko raz po reconnect; nie tworzy duplikatu zadania.
- Rozłączenie Wi-Fi i utrata IP zatrzymuje aktywną sesję TCP.
- Build Etapu 3 zakończył się powodzeniem. Test sprzętowy utraty/powrotu sieci nie został jeszcze wykonany.
- COM7 nie był dostępny w chwili próby flashowania; urządzenie nie zostało ponownie wgrane w tej sesji.
- Po ponownym podłączeniu COM7 uruchomiono firmware Etapu 3 i potwierdzono zimny start oraz IP `192.168.1.100`.
- Testy TCP dały `4/4` dokładnych transferów oraz `10/10` powtarzalnego echo po odizolowaniu pierwszych jednorazowych timeoutów runnera.
- Logi potwierdziły utratę AP, cykliczne ponawianie co 5 s i reconnect z nowym IP `192.168.1.113`.
- Automatyczne zamknięcie aktywnej sesji TCP podczas utraty Wi-Fi nie zostało w tej sesji sprzętowo potwierdzone.

## Etap 4 — stan wdrożenia

- Zrealizowano stabilny tick LED 50 ms z `vTaskDelayUntil()` i nazwę zadania `task_led_blink`.
- Kolory LED oraz stan aktywnego połączenia TCP są synchronizowane sekcją krytyczną.
- Zapis TCP→UART dosyła częściowe porcje i po 5 s bez postępu kończy sesję.
- Odczyt UART loguje presję bufora RX od 1536 bajtów.
- Urządzenie po flaszu Etapu 4 uzyskało IP `192.168.1.113`; pełny runner i 40 powtórzonych transferów zakończyły się PASS.

## Etap 5 — stan wydania

- Zintegrowano i przetestowano finalny obraz Etapu 5.
- README zawiera konfigurację, porty, parametry UART/TCP, LED i uruchamianie testów.
- Finalny build i flash przez COM7 zakończyły się powodzeniem.
- Finalny zimny start uzyskał IP `192.168.1.113`.
- Pełny runner: 10/10, 20/20, 4/4, 5/5 oraz timeout PASS.
- Cykl stabilności po restartach: 10/10 PASS.
 