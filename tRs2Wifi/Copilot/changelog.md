# Proponowany changelog

> **Etapy 0–2 wdrożone.** Etapy 3–5 pozostają propozycją. Target projektu: ESP32-C3.

## Etap 0 — przygotowanie i punkt odniesienia — WYKONANY

**Ustalenia**

- Target ESP32-C3 potwierdzono w kontekście projektu i konfiguracji. Interfejs: TCP port 2000, UART1, 57600 baud, TX GPIO4, RX GPIO5.
- Build wykonano przez ESP-IDF 6.1 (`export.bat` i `idf.py build`) z istniejącym venv `C:\Espressif\tools\python\v6.1\venv`; zakończył się powodzeniem (kod wyjścia 0).
- Przed buildem katalog `build/` nie istniał, więc `idf.py` utworzył go i przeprowadził build od świeżej konfiguracji. Nie wykonywano `fullclean`, resetu Git ani zmian w kodzie aplikacji.
- W bieżącym procesie pominięto sprawdzanie wersji constraints (`IDF_PYTHON_CHECK_CONSTRAINTS=no`), ponieważ plik constraints został wcześniej usunięty na prośbę użytkownika. ESP-IDF zgłosił, że wymagane pakiety Pythona są dostępne.

**Weryfikacja**

- `idf.py build` — **PASS** dla ESP32-C3.
- Konfiguracja targetu i toolchainu jest zgodna z ESP32-C3/RISC-V.
- Test na fizycznym urządzeniu nie był wykonywany.

## Etap 1 — konfiguracja, zależności i zasoby — WYKONANY

**Wykonane zmiany**

- SSID i hasło są ustawiane w ESP-IDF Configuration Editor (`idf.py menuconfig`), w menu „Example Configuration” zdefiniowanym w `main/Kconfig.projbuild`.
- Wartości są przechowywane lokalnie w `sdkconfig.local`, ignorowanym przez Git, i kompilowane do firmware jako `CONFIG_ESP_WIFI_SSID` oraz `CONFIG_ESP_WIFI_PASSWORD`. Przy pierwszej konfiguracji CMake tworzy ten plik na podstawie `sdkconfig`; oba pola startują puste. Hasło nie jest logowane.
- Inicjalizacja LED, UART, NVS i Wi-Fi zwraca błędy do `app_main`; przy błędzie połączenia Wi-Fi nie zostaje uruchomiony serwer TCP.
- Sprawdzane są wyniki tworzenia zadań aplikacji i zadań przekazywania TCP–UART; obsłużono też błąd alokacji buforów tych zadań.
- Usunięto nieużywaną alokację bufora z `configureUart()` i dodano bezpośrednie nagłówki API oraz zależności komponentów, w tym sterowniki GPIO/UART ESP-IDF 6.
- Przy pierwszym połączeniu Wi-Fi kończy próbę po 10 nieudanych ponowieniach, raportuje błąd i nie uruchamia serwera TCP; późniejsze rozłączenia zachowują dotychczasowe ponawianie połączenia.

**Weryfikacja**

- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS**; skompilowano zmienione pliki i wygenerowano obraz firmware.
- Sprawdzono, że `sdkconfig.local` jest ignorowany przez Git i zawiera puste pola SSID/hasła; w kodzie nie ma logowania hasła.
- Testu na fizycznym urządzeniu ani połączenia z poprawnymi danymi Wi-Fi nie wykonano. Przed uruchomieniem należy wpisać SSID i hasło w lokalnej konfiguracji.
- Podczas konfiguracji CMake pojawił się nieblokujący komunikat o braku `ESP_ROM_ELF_DIR`; build mimo to zakończył się powodzeniem.

## Etap 2 — niezawodna sesja TCP–UART — WDROŻONY

**Wykonane zmiany**

- Usunięto globalne flagi statusu, globalne uchwyty zadań i nieużywaną flagę `unBreak`. Każde połączenie ma własny kontekst z socketem i grupą zdarzeń; oba zadania otrzymują ten sam kontekst.
- Zdarzenia sesji sygnalizują zatrzymanie i zakończenie każdego kierunku. Błąd lub rozłączenie zatrzymuje drugie zadanie, a serwer czeka na zakończenie uruchomionych zadań przed zamknięciem socketu, zwolnieniem grupy zdarzeń i przyjęciem kolejnego klienta.
- Każde zadanie zwalnia swój bufor także przy błędzie alokacji, a ścieżki błędu tworzenia zadań i konfiguracji socketu sprzątają zasoby, które zostały już utworzone.
- Odczyt UART→TCP pobiera tylko aktualnie zbuforowane dane zamiast czekać na zapełnienie porcji 1024 B; przy pustym buforze zadanie usypia na 10 ms, dzięki czemu szybko reaguje na koniec sesji. Błędy odczytu UART, operacji socketu i zapisu do UART są raportowane.
- Wysyłanie przez TCP obsługuje częściowe wyniki `send()` i ponawia próbę po `EINTR`; ustawiono też 5-sekundowy limit blokującego wysyłania.
- Ostatnia aktywność sesji jest synchronizowana w jej kontekście i aktualizowana przy danych TCP odebranych od klienta oraz wysłanych klientowi. `recv()` sprawdza bezczynność co sekundę; po 60 sekundach bez ruchu w żadnym kierunku sesja jest zamykana. Zamknięcie połączenia przez klienta nadal kończy sesję od razu.

**Weryfikacja**

- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS**; wygenerowano `build/app-template.bin` (882 064 B, 16% wolnego miejsca w partycji aplikacji) i wgrano firmware na urządzenie przez COM7.
- Po wysłaniu `Test` klient pozostawił socket otwarty przez 5 s; sesja nie została zamknięta przez serwer. Po normalnym zamknięciu klienta kolejne połączenie pozostało aktywne przez 3 s.
- W próbie 65-sekundowej klient nie wysyłał danych, ale odebrał 3 771 B z UART→TCP; sesja pozostała otwarta, ponieważ ruch wychodzący resetował timer bezczynności. Z tego powodu nie zweryfikowano na urządzeniu zamknięcia po pełnych 60 s bez ruchu w obu kierunkach.
- Wcześniejsze 10 testów używało `shutdown(SHUT_WR)`, czyli zamykało kierunek nadawania klienta; nie sprawdzały utrzymywania otwartego socketu klienta. Nie weryfikowano wzorca bajtów UART z analizatorem/urządzeniem pętli zwrotnej ani błędów alokacji.
- Nie weryfikowano wzorca bajtów UART z analizatorem/urządzeniem pętli zwrotnej ani błędów alokacji; odbierane dane UART miały zmienną długość.

## Sygnalizacja aktywnego połączenia TCP

- Przy aktywnej sesji TCP dioda miga dwa razy szybciej: interwał przełączania zmienia się z 300 ms na 150 ms. Po zakończeniu sesji wraca do normalnego tempa.
- Stan połączenia jest ustawiany przez serwer TCP i bezpiecznie odczytywany przez zadanie LED.
- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS** (`build/app-template.bin`, 16% wolnego miejsca w partycji aplikacji); firmware wgrano przez COM7.
- Test TCP po wgraniu — **PASS**: po wysłaniu `Test` klient utrzymał pierwszą sesję przez 5 s; po jej zamknięciu serwer przyjął kolejne połączenie, które pozostało aktywne przez 3 s.
- Nie zmierzono optycznie częstotliwości migania LED; w środowisku testowym nie było dostępnej kamery ani przyrządu pomiarowego podłączonego do GPIO.

## Etap 3 — Wi-Fi i odzyskiwanie po utracie sieci — WDROŻONY

**Wykonane zmiany**

- Początkowe oczekiwanie na adres IP jest ograniczone do 30 sekund. Przy braku AP aplikacja nie czeka w `portMAX_DELAY`, LED i zadania systemowe nadal działają.
- Dodano limit 10 prób początkowego połączenia oraz 5-sekundowy jednorazowy timer ponawiania po wyczerpaniu puli prób.
- Po odzyskaniu każdego adresu IP serwer TCP jest uruchamiany tylko raz i ponownie przyjmuje nowych klientów.
- Dodano obsługę zdarzeń utraty IP oraz rozłączenia Wi-Fi: aktywna sesja TCP jest natychmiast zatrzymywana przez `shutdown()`, a LED przechodzi w stan czerwony.
- Po ponownym uzyskaniu IP stan LED przechodzi na zielony i zamykany jest timer ponawiania.

**Weryfikacja**

- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS**.
- Kompilacja potwierdziła poprawne API `esp_timer`, zdarzeń Wi-Fi/IP oraz centralną obsługę uruchamiania serwera TCP.
- Flash firmware przez COM7 — **PASS**.
- Zimny start i połączenie: **PASS** — urządzenie uzyskało IP i uruchomiło serwer portu 2000.
- Dokładne transfery 1/32/1024/4096 B: **4/4 PASS** przy powtórnym teście.
- Powtarzane echo `Hello world`: **10/10 PASS**.
- Timeout bezczynności: **PASS**.
- Utrata i powrót Wi-Fi: **PASS częściowy** — rozłączenie, retry co 5 s i nowe IP potwierdzono; zamknięcie aktywnej sesji TCP podczas utraty sieci pozostaje do potwierdzenia przy kolejnym fizycznym wyłączeniu AP.

## Etap 4 — przeciążenie, LED i diagnostyka

**Wykonane zmiany**

- Kolory LED i stan aktywnego połączenia TCP są synchronizowane wspólną sekcją krytyczną. Buforowanie zmiany koloru zostało ograniczone do jednego spójnego stanu.
- Zadanie LED używa `vTaskDelayUntil()` z bazowym tickiem 50 ms i stabilnym przełączaniem interwału 300/150 ms. Nazwa zadania brzmi `task_led_blink`.
- Zapis TCP→UART obsługuje częściowe i zerowe wyniki `uart_write_bytes()`, dosyła niewysłane bajty i kończy operację po 5 s bez postępu bufora.
- Odczyt UART→TCP loguje presję, gdy bufor RX przekracza 1536 bajtów.
- Logi przeciążenia i błędów nie ujawniają danych dostępowych ani payloadów.

**Weryfikacja**

- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS**.
- Flash przez COM7 — **PASS**.
- Pełny runner testów: Test 1 10/10, Test 2 20/20, Test 3 4/4, Test 6 5/5, Test 7 PASS.
- Powtórzone transfery o rozmiarach 4096/1024/32/1 B: **40/40 PASS**.
- Monitor COM7 nie wykazał resetu, panic ani komunikatów przeciążenia buforów.

## Etap 5 — test akceptacyjny i wydanie

**Wykonane zmiany**

- Zintegrowano obsługę Wi-Fi, sesji TCP, UART i LED w jednym finalnym obrazie firmware.
- README zastąpiono opisem adaptera, parametrów połączenia, konfiguracji Wi-Fi, sygnalizacji LED i sposobu testów.
- Dodano czyszczenie wskaźnika aktywnej sesji TCP po zamknięciu i wyłączenie szybkiego trybu LED.
- TCP opisano jako strumień bajtów bez granic wiadomości.

**Weryfikacja**

- Build ESP-IDF 6.1 dla ESP32-C3 — **PASS**.
- Flash przez COM7 — **PASS**.
- Zimny start i uzyskanie IP `192.168.1.113` — **PASS**.
- Test 1 10/10, Test 2 20/20, Test 3 4/4, Test 6 5/5, Test 7 PASS.
- Test stabilności po restartach: **10/10 PASS**.
- Monitor nie wykazał PANIC, Guru Meditation, resetu ani przeciążeń buforów w przebiegu.
- Test fizycznego braku AP i powrotu sieci pozostaje częściowo potwierdzony z Etapu 3 i nie został ponownie powtórzony w Etapie 5.

## Zasady realizacji

- Wdrażać jeden etap naraz; po każdym etapie uruchomić build i testy przypisane powyżej.
- Nie wykonywać destrukcyjnego czyszczenia ani resetu repozytorium bez sprawdzenia zmian użytkownika.
- Po zakończeniu etapu zaktualizować ten plik z „propozycji” na rzeczywisty wpis changeloga, podając wykonane zmiany i wyniki testów.
