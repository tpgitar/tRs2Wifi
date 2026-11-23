# Testy funkcjonalne

## Uruchamianie automatyczne

Z katalogu projektu uruchom:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\Copilot\Test\run_tests.ps1"
```

Klient TCP używany przez runner jest w `tcp_echo_client.py`; wysyła dane przez `socket.sendall()` i sprawdza dokładną odpowiedź.

## 1. Pętla zwrotna UART/TCP

**Warunki:** TxD UART1 połączony z RxD, port TCP 2000, urządzenie ma połączenie z Wi-Fi.

1. Powtórz 10 razy:
   - nawiąż połączenie TCP,
   - wyślij tekst `Hello world`,
   - sprawdź, czy odpowiedź zawiera ten sam tekst,
   - zamknij połączenie TCP.

## 2. Wielokrotne połączenia

1. Nawiąż i zamknij połączenie TCP.
2. Powtórz tę operację co najmniej 20 razy.
3. Oczekiwane: każde nowe połączenie działa, serwer nie zawiesza się i urządzenie nie resetuje się.

## 3. Transfer danych z klienta do UART

1. Nawiąż połączenie TCP.
2. Wyślij porcje o różnych długościach: 1 B, 32 B, 1024 B i 4096 B.
3. Sprawdź poprawność danych otrzymanych przez klienta TCP.
4. Oczekiwane: wszystkie dane są przekazane bez zmian i bez utraty bajtów.

## 6. Zamknięcie połączenia przez klienta

1. Nawiąż połączenie TCP.
2. Wyślij dane i natychmiast zamknij gniazdo klienta.
3. Nawiąż nowe połączenie.
4. Oczekiwane: poprzednia sesja kończy się i można natychmiast obsłużyć kolejnego klienta.

## 7. Timeout bezczynności

1. Nawiąż połączenie TCP.
2. Nie wysyłaj ani nie odbieraj danych przez co najmniej 65 sekund.
3. Oczekiwane: serwer zamyka sesję po skonfigurowanym timeoutie bezczynności.

--stop--

## 8. Brak konfiguracji lub niedostępnego punktu dostępu Wi-Fi

1. Usuń lub wpisz niepoprawne dane Wi-Fi.
2. Uruchom ponownie urządzenie.
3. Oczekiwane: urządzenie raportuje błąd Wi-Fi i nie uruchamia serwera w nieznanym stanie.

## 9. Zimny start z poprawnym Wi-Fi

1. Uruchom urządzenie z poprawnie skonfigurowanym Wi-Fi.
2. Poczekaj na uzyskanie adresu IP i uruchomienie serwera.
3. Nawiąż połączenie TCP na porcie 2000.
4. Oczekiwane: połączenie działa i następuje poprawne przekazywanie danych.

## 10. Utrata i odzyskanie Wi-Fi

1. Nawiąż aktywne połączenie TCP.
2. Wyłącz punkt dostępu na co najmniej 30 sekund.
3. Ponownie uruchom punkt dostępu.
4. Oczekiwane: utracona sesja zostaje zakończona, a po odzyskaniu IP serwer przyjmuje nowe połączenie.

## 11. Stabilność po restarcie

1. Wykonaj powtórnie 10 cykli: uruchomienie urządzenia, połączenie TCP, przekazanie `Hello world`, zamknięcie połączenia.
2. Oczekiwane: brak resetu, zawieszenia ani błędnej konfiguracji kolejnego startu.

## 12. Logi i diagnozowanie

1. Wykonaj testy z poprawną i błędną konfiguracją.
2. Sprawdź logi przez osobną konsolę.
3. Oczekiwane: logi wskazują etap błędu i wynik połączenia, bez ujawniania hasła Wi-Fi ani danych aplikacyjnych.