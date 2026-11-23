# Analiza kodu źródłowego

## Zakres

Przejrzano główne pliki aplikacji: `main/main.c`, `main/Wifi.c`, `main/tcpServer.c`, `main/uart.c`, `main/Led.c` oraz `main/CMakeLists.txt`.

## Jak działa aplikacja

1. `app_main()` inicjalizuje diodę LED i UART, uruchamia zadanie LED oraz wywołuje `initWifi()`.
2. `initWifi()` inicjalizuje NVS i uruchamia Wi-Fi w trybie stacji (STA). Oczekiwanie na adres IP jest synchroniczne.
3. Dopiero po uzyskaniu adresu IP `app_main()` uruchamia zadanie serwera TCP.
4. Serwer przyjmuje jednego klienta naraz. Dwa zadania przekazują dane w obu kierunkach: TCP → UART1 i UART1 → TCP.

## Konfiguracja komunikacji

- TCP: serwer IPv4 nasłuchuje na porcie **2000**, na wszystkich interfejsach (`INADDR_ANY`).
- UART: **UART1**, **57600 baud**, 8 bitów danych, bez parzystości, 1 bit stopu, bez sprzętowego flow control.
- Piny UART: TX GPIO4, RX GPIO5.
- UART instaluje bufor RX o rozmiarze `BUF_SIZE * 2`; zadania przekazujące dane używają buforów po 1024 bajty.

## Najważniejsze obserwacje

### Wi-Fi i uruchomienie serwera

- Dane sieci Wi-Fi są wpisane na stałe w `Wifi.c`; wartości z `Kconfig.projbuild` nie są używane przez konfigurację stacji.
- Kod loguje SSID i hasło przy wyniku połączenia. Nie należy wypisywać hasła w logach; dane dostępu warto pobierać z konfiguracji lub bezpieczniejszego źródła.
- `EXAMPLE_ESP_MAXIMUM_RETRY` jest zdefiniowane, ale limit prób i ustawienie `WIFI_FAIL_BIT` są zakomentowane. Przy braku połączenia `xEventGroupWaitBits(..., portMAX_DELAY)` może czekać bez końca, więc serwer TCP nie zostanie uruchomiony.
- Po rozłączeniu kod ponawia `esp_wifi_connect()` bez ograniczenia liczby prób.

### Przekazywanie TCP ↔ UART

- TCP jest strumieniem bajtów, bez granic wiadomości. Kod przekazuje kolejne porcje danych; klient nie powinien zakładać, że jedno `send()` odpowiada jednemu `recv()`.
- Przy wysyłaniu z UART do TCP kod sprawdza tylko, czy `send()` zwróciło wartość ujemną. Dodatni wynik mniejszy od długości porcji oznacza, że część danych nie została wysłana; obecnie brak pętli dosyłającej resztę.
- Jednocześnie obsługiwany jest jeden klient. Kolejne połączenia czekają, aż zakończy się obsługa bieżącego.
- Współdzielone flagi stanu zadań są używane bez synchronizacji, a ich nazwy i przypisania w dwóch zadaniach są mało czytelne. Warto rozważyć prostszy mechanizm zamykania połączenia oraz bezpieczną synchronizację.
- Wyniki `xTaskCreate()` nie są sprawdzane. Przy nieudanym utworzeniu zadania stan połączenia może być błędny.
- Przy błędzie `malloc()` zadanie kończy działanie, ale nie zeruje własnego uchwytu zadania. Serwer oczekuje, aż oba uchwyty będą `NULL`, więc może czekać bez końca.
- Gniazdo ma timeout odbioru 60 s. Jeśli jedno zadanie zakończy się, drugie może pozostać zablokowane w `recv()` do upływu timeoutu.

### UART, LED i zasoby

- `configureUart()` alokuje bufor `data`, ale go nie używa ani nie zwalnia — powoduje to wyciek pamięci.
- Funkcja `task_Led_500ms()` odświeża LED co 300 ms. Nazwa zadania nie odpowiada faktycznemu interwałowi.
- Stan kolorów LED jest współdzielony między zadaniem LED a obsługą zdarzeń Wi-Fi bez synchronizacji.

### Konfiguracja komponentu

- W `main/CMakeLists.txt` listy `REQUIRES` i `PRIV_REQUIRES` są puste mimo korzystania z komponentów ESP-IDF i `led_strip`. Jeśli konfiguracja lub kompilacja zgłosi brak zależności, należy jawnie zadeklarować wymagane komponenty.
- Niektóre pliki używają m.in. `malloc`, `free` i API FreeRTOS bez bezpośredniego dołączenia wszystkich odpowiadających im nagłówków. Warto dołączać wymagane nagłówki jawnie, zamiast polegać na nagłówkach pośrednich.

## Wniosek

Kod realizuje podstawowy most TCP–UART, ale przed uznaniem go za niezawodny warto poprawić obsługę błędów i zakończenia zadań, zachowanie przy braku Wi-Fi, częściowe wysyłanie TCP oraz wyciek bufora UART. Konfiguracja sieci i sprzętu jest obecnie w dużej mierze wpisana na stałe. Ten dokument opisuje analizę statyczną; nie potwierdza poprawności kompilacji ani działania na urządzeniu.

## Proponowane usprawnienia

### Priorytet 1 — niezawodność połączeń i przekazywania danych

1. **Ujednolicić kończenie sesji TCP.** Zastąpić globalne flagi o niejasnych nazwach kontekstem jednej sesji i jednoznacznym mechanizmem stop (np. powiadomieniem FreeRTOS). Po błędzie lub rozłączeniu przerwać blokujące operacje drugiego zadania, zamknąć sesję i poczekać na zakończenie obu zadań. Każda ścieżka wyjścia, także błąd alokacji, powinna zwalniać zasoby i aktualizować stan sesji.
2. **Obsłużyć pełne wysłanie TCP.** Wysyłać pozostałą część porcji, dopóki wszystkie bajty nie zostaną zaakceptowane albo wystąpi błąd. Sprawdzać wyniki `setsockopt()`, `shutdown()` i tworzenia zadań oraz logować przyczynę błędu.
3. **Zdefiniować zachowanie przy rozłączeniu Wi-Fi.** Zamiast bezterminowego oczekiwania w `initWifi()` wprowadzić jawny stan połączenia, ograniczone ponowienia z opóźnieniem i obsługę utraty IP. Przy utracie sieci zamykać aktywne sesje TCP; po odzyskaniu IP pozwalać na nowe połączenia.
4. **Obsłużyć przeciążenie i utratę danych.** Określić, co ma się stać, gdy UART lub sieć nie nadąża: użyć kontrolowanych buforów/kolejek, raportować przepełnienia i sprawdzić, czy zapis UART przetworzył całą porcję. Rozmiary buforów dobrać testami przy długotrwałym transferze.

### Priorytet 2 — konfiguracja i bezpieczeństwo wdrożenia

5. **Przenieść konfigurację Wi-Fi do konfiguracji projektu.** Użyć wartości z Kconfig zamiast stałych w `Wifi.c`; nie logować hasła. Nie umieszczać właściwych danych dostępowych w publicznie śledzonym pliku konfiguracyjnym.
6. **Ocenić dostępność serwera TCP.** Serwer nasłuchuje na wszystkich interfejsach i nie uwierzytelnia klientów. Dla wdrożenia ograniczyć dostęp na poziomie sieci lub dodać uzgodniony mechanizm autoryzacji, jeśli wymagają tego warunki użycia.
7. **Jawnie zadeklarować zależności komponentu.** Uzupełnić `REQUIRES`/`PRIV_REQUIRES` w `main/CMakeLists.txt` o faktycznie używane komponenty i dołączyć bezpośrednio nagłówki standardowe oraz FreeRTOS, zamiast polegać na zależnościach pośrednich.

### Priorytet 3 — utrzymanie i testowanie

8. **Usunąć nieużywaną alokację UART.** Bufor w `configureUart()` nie jest potrzebny; jeśli alokacja będzie potrzebna, przypisać jej właściciela, sprawdzać wynik i zwalniać pamięć.
9. **Uprościć obsługę LED.** Zmienić pętlę na `vTaskDelayUntil()` dla stabilnego okresu i dopasować nazwę zadania do interwału. Do przekazywania zmian koloru użyć kolejki/powiadomień lub innego prostego mechanizmu synchronizacji.
10. **Dodać testy zachowania mostu.** Sprawdzić brak i powrót Wi-Fi, rozłączenie klienta podczas ruchu, długie transfery w obu kierunkach, przeciążenie UART oraz błędy tworzenia zadań/alokacji. Zweryfikować także parametry UART na docelowej płytce.

Najpierw warto wdrożyć usprawnienia z priorytetu 1, bo dotyczą zawieszania sesji i utraty danych. Zmiany powinny być wprowadzane etapami, z testem kompilacji i testem sprzętowym po każdej większej zmianie.
