#include <stdio.h>      // Standard Input/Output: umożliwia korzystanie z funkcji takich jak printf, fgets, fopen, fclose, fread, fwrite, fseek
#include <stdlib.h>     // Standard Library: obsługuje dynamiczne zarządzanie pamięcią (malloc, realloc, free), exit oraz inne funkcje pomocnicze
#include <string.h>     // String Library: umożliwia operacje na łańcuchach znaków, np. strcmp, strcpy, strcspn, memcpy (przydatne przy obsłudze tekstu)
#include <stdbool.h>    // Standard Boolean Library: definiuje typ bool i stałe true/false, co poprawia czytelność kodu przy operacjach logicznych
#include <ctype.h>      // Character Type Library: dostarcza funkcji do obsługi znaków, np. isdigit, isalpha czy toupper – pomocne w walidacji wejścia

// Definicje stałych ułatwiające modyfikację kodu
#define MAX_STR 100             // Maksymalna długość łańcucha znaków (np. imię, nazwisko, marka)
#define KLIENT_FILE "klienci.dat"   // Nazwa pliku binarnego przechowującego dane klientów
#define SAMOCHOD_FILE "samochody.dat" // Nazwa pliku binarnego przechowującego dane samochodów
#define MIN_YEAR 1900           // Minimalny rok produkcji, jako walidacja danych
#define CURRENT_YEAR 2024       // Aktualny rok – ograniczenie dla wprowadzanych danych dotyczących rocznika

// Struktura przechowująca dane klienta
typedef struct {
    int id;                     // Unikalny identyfikator klienta
    char imie[MAX_STR];         // Imię klienta
    char nazwisko[MAX_STR];     // Nazwisko klienta
    char adres[MAX_STR];        // Adres klienta
    bool aktywny;               // Flaga: true, jeśli rekord jest aktywny; false, jeśli został usunięty
} Klient;

// Struktura przechowująca dane samochodu
typedef struct {
    int id;                     // Unikalny identyfikator samochodu
    char marka[MAX_STR];        // Marka samochodu (np. Toyota)
    char model[MAX_STR];        // Model samochodu (np. Corolla)
    int rok;                    // Rok produkcji – ograniczamy wpisy do przedziału MIN_YEAR-CURRENT_YEAR
    float moc;                  // Moc samochodu wyrażona w koniach mechanicznych (KM)
    float cena;                 // Cena samochodu (jednostka: zł lub inna, zależnie od kontekstu)
    int id_wlasciciela;         // Identyfikator właściciela – -1 oznacza, że samochód jest dostępny (nie sprzedany)
    bool aktywny;               // Flaga: true, jeśli rekord jest aktywny; false, jeśli został usunięty
} Samochod;

/* ====================== FUNKCJE POMOCNICZE ====================== */

// Funkcja czyszcząca bufor wejściowy. Przydaje się, gdy chcemy usunąć znaki pozostałe po poprzednich odczytach.
void wyczysc_bufor() {
    int c;
    // Odczytuje znaki dopóki nie napotka znaku końca linii lub końca pliku
    while ((c = getchar()) != '\n' && c != EOF);
}

// Funkcja wczytująca liczbę całkowitą z wejścia.
// Używamy funkcji fgets() aby odczytać cały wiersz, a następnie sscanf() aby sparsować wartość.
int wczytaj_liczbe(const char* prompt) {
    char buf[32];
    int value;
    while (1) {
        printf("%s", prompt); // Wyświetlenie komunikatu dla użytkownika
        fgets(buf, sizeof(buf), stdin);
        if (sscanf(buf, "%d", &value) == 1)
            return value;
        printf("Niepoprawny format, wprowadź ponownie.\n");
    }
}

// Funkcja wczytująca liczbę zmiennoprzecinkową z wejścia.
// Mechanizm jest analogiczny do funkcji wczytaj_liczbe, ale obsługuje typ float.
float wczytaj_liczbe_float(const char* prompt) {
    char buf[32];
    float value;
    while (1) {
        printf("%s", prompt);
        fgets(buf, sizeof(buf), stdin);
        if (sscanf(buf, "%f", &value) == 1)
            return value;
        printf("Niepoprawny format, wprowadź ponownie.\n");
    }
}

// Funkcja wczytująca rok, z dodatkową walidacją, żeby rok mieścił się w ustalonym przedziale.
// Zapewnia to, że użytkownik nie wprowadzi nierealistycznych wartości.
int wczytaj_rok(const char* prompt) {
    int year;
    do {
        year = wczytaj_liczbe(prompt);
        if (year < MIN_YEAR || year > CURRENT_YEAR)
            printf("Rok musi być z przedziału [%d, %d].\n", MIN_YEAR, CURRENT_YEAR);
    } while (year < MIN_YEAR || year > CURRENT_YEAR);
    return year;
}

// Funkcja szukająca "wolnego miejsca" (rekordu o nieaktywnym statusie) w pliku.
// Parametry:
//    - f: wskaźnik do otwartego pliku
//    - rozmiar: rozmiar rekordu (np. sizeof(Klient) lub sizeof(Samochod))
// Zwraca pozycję (offset) w pliku, gdzie znajdują się nieaktywne dane; -1 jeśli nie znaleziono.
long znajdz_wolne_miejsce(FILE* f, size_t rozmiar) {
    // Ustawienie wskaźnika na początek pliku
    fseek(f, 0, SEEK_SET);
    long pozycja = -1;
    // Rezerwujemy bufor do odczytania pojedynczego rekordu
    char* buf = malloc(rozmiar);

    // Odczytujemy rekordy jeden po drugim
    while (fread(buf, rozmiar, 1, f)) {
        bool aktywny;
        // Zakładając, że pole aktywny znajduje się na końcu struktury,
        // kopiujemy ostatnie bajty odpowiadające typowi bool.
        memcpy(&aktywny, buf + rozmiar - sizeof(bool), sizeof(bool));
        if (!aktywny) {
            // Jeśli rekord nie jest aktywny, zwracamy pozycję początku tego rekordu
            pozycja = ftell(f) - rozmiar;
            break;
        }
    }

    free(buf);
    return pozycja;
}

/* ====================== OPERACJE NA KLIENTACH ====================== */

// Funkcja dodająca nowego klienta do pliku.
// Plik jest otwierany w trybie "r+b" (do odczytu i zapisu, bez kasowania) lub, 
// jeśli nie istnieje, w trybie "w+b" (tworzy nowy plik binarny).
// Ustawienie wskaźnika na koniec pliku (fseek) zapewnia, że dane są dopisywane.
void dodaj_klienta() {
    FILE* f = fopen(KLIENT_FILE, "r+b");
    if (!f) f = fopen(KLIENT_FILE, "w+b");  // Jeśli plik nie istnieje, utwórz go bez kasowania danych
    // Przesuwamy wskaźnik na koniec pliku, aby dopisać nowy rekord
    fseek(f, 0, SEEK_END);

    Klient k;
    // Obliczamy ID na podstawie bieżącej pozycji – liczba rekordów plus jeden
    k.id = ftell(f) / sizeof(Klient) + 1;
    k.aktywny = true; // Rekord jest aktywny w momencie dodania

    // Wczytanie danych od użytkownika
    printf("\nDodawanie nowego klienta:\n");
    printf("Imię: ");
    fgets(k.imie, MAX_STR, stdin);
    printf("Nazwisko: ");
    fgets(k.nazwisko, MAX_STR, stdin);
    printf("Adres: ");
    fgets(k.adres, MAX_STR, stdin);

    // Usuwamy znak nowej linii, który fgets może umieszczać na końcu łańcucha
    k.imie[strcspn(k.imie, "\n")] = 0;
    k.nazwisko[strcspn(k.nazwisko, "\n")] = 0;
    k.adres[strcspn(k.adres, "\n")] = 0;

    // Zapisujemy rekord do pliku
    fwrite(&k, sizeof(Klient), 1, f);
    fclose(f);
    printf("Dodano klienta.\n");
}

// Funkcja usuwająca klienta – oznaczając rekord jako nieaktywny.
// Nie usuwamy rekordu fizycznie, aby zachować spójność pliku (łatwiejsze wyszukiwanie).
void usun_klienta() {
    int id = wczytaj_liczbe("Podaj ID klienta do usunięcia: ");
    FILE* f = fopen(KLIENT_FILE, "rb+");
    if (!f) {
        printf("Plik z klientami nie istnieje.\n");
        return;
    }
    Klient k;
    bool found = false;

    // Przechodzimy przez wszystkie rekordy w pliku
    while (fread(&k, sizeof(Klient), 1, f)) {
        if (k.id == id && k.aktywny) {
            // Oznaczamy rekord jako nieaktywny – tzw. "soft delete"
            k.aktywny = false;
            // Cofamy wskaźnik o jeden rekord
            fseek(f, -sizeof(Klient), SEEK_CUR);
            // Nadpisujemy rekord zmienioną strukturą
            fwrite(&k, sizeof(Klient), 1, f);
            found = true;
            break;
        }
    }

    fclose(f);
    printf(found ? "Klient usunięty.\n" : "Nie znaleziono klienta.\n");
}

// Funkcja modyfikująca dane klienta.
// Po znalezieniu rekordu (używamy unikalnego ID), umożliwia aktualizację pól,
// przy czym użytkownik może pominąć aktualizację (wciśnięcie Enter).
void modyfikuj_klienta() {
    int id = wczytaj_liczbe("Podaj ID klienta do modyfikacji: ");
    FILE* f = fopen(KLIENT_FILE, "rb+");
    if (!f) {
        printf("Plik z klientami nie istnieje.\n");
        return;
    }
    Klient k;
    bool found = false;

    while (fread(&k, sizeof(Klient), 1, f)) {
        if (k.id == id && k.aktywny) {
            // Wyświetlamy aktualne dane do wglądu
            printf("Aktualne dane:\n");
            printf("Imię: %s\nNazwisko: %s\nAdres: %s\n", k.imie, k.nazwisko, k.adres);

            char buf[MAX_STR];

            // Aktualizacja imienia, jeżeli użytkownik poda nową wartość
            printf("Nowe imię (Enter aby pominąć): ");
            fgets(buf, MAX_STR, stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0) strcpy(k.imie, buf);

            // Aktualizacja nazwiska
            printf("Nowe nazwisko (Enter aby pominąć): ");
            fgets(buf, MAX_STR, stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0) strcpy(k.nazwisko, buf);

            // Aktualizacja adresu
            printf("Nowy adres (Enter aby pominąć): ");
            fgets(buf, MAX_STR, stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0) strcpy(k.adres, buf);

            // Nadpisujemy zmodyfikowany rekord w pliku
            fseek(f, -sizeof(Klient), SEEK_CUR);
            fwrite(&k, sizeof(Klient), 1, f);
            found = true;
            break;
        }
    }
    fclose(f);
    printf(found ? "Zaktualizowano klienta.\n" : "Nie znaleziono klienta.\n");
}

// Funkcja porównująca dwa rekordy klientów pod kątem sortowania.
// Parametr tryb określa kryterium sortowania: 1 – nazwisko, 2 – imię, 3 – adres, domyślnie po ID.
int porownaj_klientow(const void* a, const void* b, int tryb) {
    Klient* k1 = (Klient*)a;
    Klient* k2 = (Klient*)b;

    switch (tryb) {
    case 1: return strcmp(k1->nazwisko, k2->nazwisko);
    case 2: return strcmp(k1->imie, k2->imie);
    case 3: return strcmp(k1->adres, k2->adres);
    default: return k1->id - k2->id;
    }
}

// Funkcja wyświetlająca listę aktywnych klientów.
// Dane są najpierw wczytywane do dynamicznej tablicy, a następnie sortowane metodą bąbelkową.
// Sortowanie bąbelkowe jest łatwe do implementacji (choć nieefektywne dla dużych danych).
void wyswietl_klientow(int tryb_sort) {
    FILE* f = fopen(KLIENT_FILE, "rb");
    if (!f) {
        printf("Brak danych klientów.\n");
        return;
    }

    Klient k;
    Klient* klienci = NULL;  // Dynamiczna tablica przechowująca rekordy
    int count = 0;

    // Wczytujemy wszystkie rekordy, które mają flagę aktywną
    while (fread(&k, sizeof(Klient), 1, f)) {
        if (k.aktywny) {
            // Zwiększamy rozmiar tablicy o 1 rekord
            klienci = realloc(klienci, (count + 1) * sizeof(Klient));
            klienci[count++] = k;
        }
    }

    // Sortowanie bąbelkowe według wybranego kryterium
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (porownaj_klientow(&klienci[j], &klienci[j + 1], tryb_sort) > 0) {
                Klient temp = klienci[j];
                klienci[j] = klienci[j + 1];
                klienci[j + 1] = temp;
            }
        }
    }

    // Wyświetlamy sformatowaną tabelę z danymi klientów
    printf("\n%5s %-20s %-20s %-30s\n", "ID", "Imię", "Nazwisko", "Adres");
    for (int i = 0; i < count; i++) {
        printf("%5d %-20s %-20s %-30s\n",
            klienci[i].id,
            klienci[i].imie,
            klienci[i].nazwisko,
            klienci[i].adres);
    }

    free(klienci);
    fclose(f);
}

/* ====================== OPERACJE NA SAMOCHODACH ====================== */

// Funkcja dodająca nowy rekord samochodu do pliku.
// Podobnie jak dla klientów, plik otwieramy w trybie "r+b", aby nie kasować dotychczasowych danych,
// lub, jeśli plik nie istnieje, w trybie "w+b". Wskaźnik ustawiamy na koniec pliku.
void dodaj_samochod() {
    FILE* f = fopen(SAMOCHOD_FILE, "r+b");
    if (!f) f = fopen(SAMOCHOD_FILE, "w+b");  // Utworzenie pliku, jeśli nie istnieje
    fseek(f, 0, SEEK_END); // Ustawienie wskaźnika na koniec pliku, by dopisywać dane

    Samochod s;
    // Obliczenie nowego ID na podstawie ilości już zapisanych rekordów
    s.id = ftell(f) / sizeof(Samochod) + 1;
    s.aktywny = true;       // Rekord jest aktywny w momencie dodania
    s.id_wlasciciela = -1;   // -1 wskazuje, że samochód jest dostępny

    // Wczytanie informacji o samochodzie od użytkownika
    printf("\nDodawanie nowego samochodu:\n");
    printf("Marka: ");
    fgets(s.marka, MAX_STR, stdin);
    printf("Model: ");
    fgets(s.model, MAX_STR, stdin);
    // Rok produkcji: ograniczenie, by rok mieścił się w ustalonym przedziale (MIN_YEAR do CURRENT_YEAR)
    s.rok = wczytaj_rok("Rok produkcji: ");
    // Moc samochodu – w jednostkach KM (konie mechaniczne)
    s.moc = wczytaj_liczbe_float("Moc (KM): ");
    // Cena samochodu – jednostka zależna od zastosowania, np. zł
    s.cena = wczytaj_liczbe_float("Cena: ");

    // Usuwamy znak nowej linii z łańcuchów znaków
    s.marka[strcspn(s.marka, "\n")] = 0;
    s.model[strcspn(s.model, "\n")] = 0;

    // Zapisujemy rekord do pliku
    fwrite(&s, sizeof(Samochod), 1, f);
    fclose(f);
    printf("Dodano samochód.\n");
}

// Funkcja usuwająca samochód przez ustawienie flagi aktywnego na false.
// Rekord fizycznie pozostaje w pliku, aby utrzymać spójność pozycji danych.
void usun_samochod() {
    int id = wczytaj_liczbe("Podaj ID samochodu do usunięcia: ");
    FILE* f = fopen(SAMOCHOD_FILE, "rb+");
    if (!f) {
        printf("Plik z samochodami nie istnieje.\n");
        return;
    }
    Samochod s;
    bool found = false;

    // Przeglądamy wszystkie rekordy, szukając pasującego ID i aktywnego wpisu
    while (fread(&s, sizeof(Samochod), 1, f)) {
        if (s.id == id && s.aktywny) {
            s.aktywny = false; // Oznaczamy rekord jako nieaktywny ("soft delete")
            fseek(f, -sizeof(Samochod), SEEK_CUR);
            fwrite(&s, sizeof(Samochod), 1, f);
            found = true;
            break;
        }
    }

    fclose(f);
    printf(found ? "Samochód usunięty.\n" : "Nie znaleziono samochodu.\n");
}

// Funkcja modyfikująca dane samochodu.
// Po znalezieniu rekordu o danym ID pozwala na modyfikację pól – użytkownik może pominąć zmianę, wpisując 0 lub pozostawiając puste wejście.
void modyfikuj_samochod() {
    int id = wczytaj_liczbe("Podaj ID samochodu do modyfikacji: ");
    FILE* f = fopen(SAMOCHOD_FILE, "rb+");
    if (!f) {
        printf("Plik z samochodami nie istnieje.\n");
        return;
    }
    Samochod s;
    bool found = false;
    char buf[MAX_STR];

    // Przeglądamy plik w poszukiwaniu rekordu do modyfikacji
    while (fread(&s, sizeof(Samochod), 1, f)) {
        if (s.id == id && s.aktywny) {
            // Wyświetlamy aktualne dane, aby użytkownik wiedział, co modyfikuje
            printf("Aktualne dane samochodu:\n");
            printf("Marka: %s\nModel: %s\nRok: %d\nMoc: %.0f\nCena: %.2f\n",
                s.marka, s.model, s.rok, s.moc, s.cena);

            // Aktualizacja marki
            printf("Nowa marka (Enter aby pominąć): ");
            fgets(buf, MAX_STR, stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0)
                strcpy(s.marka, buf);

            // Aktualizacja modelu
            printf("Nowy model (Enter aby pominąć): ");
            fgets(buf, MAX_STR, stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0)
                strcpy(s.model, buf);

            // Aktualizacja roku – przyjmujemy wartość 0 jako oznaczenie "nie modyfikuj"
            printf("Nowy rok (0 aby pominąć): ");
            int nowy_rok = wczytaj_liczbe("");
            if (nowy_rok != 0)
                s.rok = nowy_rok;

            // Aktualizacja mocy – 0 oznacza pominięcie zmiany
            printf("Nowa moc (0 aby pominąć): ");
            float nowa_moc = wczytaj_liczbe_float("");
            if (nowa_moc != 0.0)
                s.moc = nowa_moc;

            // Aktualizacja ceny – 0 oznacza, że cena się nie zmienia
            printf("Nowa cena (0 aby pominąć): ");
            float nowa_cena = wczytaj_liczbe_float("");
            if (nowa_cena != 0.0)
                s.cena = nowa_cena;

            // Nadpisujemy zmodyfikowany rekord w pliku
            fseek(f, -sizeof(Samochod), SEEK_CUR);
            fwrite(&s, sizeof(Samochod), 1, f);
            found = true;
            break;
        }
    }
    fclose(f);
    printf(found ? "Zaktualizowano samochód.\n" : "Nie znaleziono samochodu.\n");
}

// Funkcja realizująca sprzedaż samochodu.
// Po potwierdzeniu, że klient o podanym ID istnieje (i jest aktywny) oraz samochód jest dostępny (id_wlasciciela == -1),
// przypisuje samochodowi id klienta.
void sprzedaz_samochodu() {
    int id_samochodu = wczytaj_liczbe("Podaj ID samochodu: ");
    int id_klienta = wczytaj_liczbe("Podaj ID klienta: ");

    FILE* f_sam = fopen(SAMOCHOD_FILE, "rb+");
    FILE* f_kl = fopen(KLIENT_FILE, "rb");

    if (!f_sam || !f_kl) {
        printf("Brak plików z danymi.\n");
        if (f_sam) fclose(f_sam);
        if (f_kl) fclose(f_kl);
        return;
    }

    Samochod s;
    Klient k;
    bool sam_found = false, kl_found = false;

    // Weryfikacja, czy klient o podanym ID istnieje i jest aktywny
    while (fread(&k, sizeof(Klient), 1, f_kl)) {
        if (k.id == id_klienta && k.aktywny) {
            kl_found = true;
            break;
        }
    }

    // Weryfikacja, czy samochód jest dostępny oraz znalezienie rekordu samochodu
    while (fread(&s, sizeof(Samochod), 1, f_sam)) {
        if (s.id == id_samochodu && s.aktywny) {
            if (s.id_wlasciciela != -1) {
                // Jeżeli samochód ma już przypisanego właściciela, nie można go sprzedać ponownie
                printf("Samochód już został sprzedany!\n");
                fclose(f_sam);
                fclose(f_kl);
                return;
            }
            sam_found = true;
            // Przypisujemy ID klienta do samochodu, co oznacza sprzedaż
            s.id_wlasciciela = id_klienta;
            fseek(f_sam, -sizeof(Samochod), SEEK_CUR);
            fwrite(&s, sizeof(Samochod), 1, f_sam);
            break;
        }
    }

    fclose(f_sam);
    fclose(f_kl);

    if (!sam_found)
        printf("Nie znaleziono samochodu!\n");
    else if (!kl_found)
        printf("Nie znaleziono klienta!\n");
    else
        printf("Sprzedaż zarejestrowana!\n");
}

// Funkcja porównująca dwa rekordy samochodów według wybranego kryterium
// Parametr tryb określa, co jest podstawą porównania:
// 1 – marka, 2 – model, 3 – rok, 4 – moc, 5 – cena, domyślnie po ID.
int porownaj_samochody(const Samochod* a, const Samochod* b, int tryb) {
    switch (tryb) {
    case 1: return strcmp(a->marka, b->marka);
    case 2: return strcmp(a->model, b->model);
    case 3: return a->rok - b->rok;
    case 4: return (a->moc > b->moc) - (a->moc < b->moc);
    case 5: return (a->cena > b->cena) - (a->cena < b->cena);
    default: return a->id - b->id;
    }
}

// Funkcja wyświetlająca listę samochodów.
// Podobnie jak przy klientach, wczytuje wszystkie aktywne rekordy do dynamicznej tablicy,
// sortuje je metodą bąbelkową według kryterium podanego przez użytkownika, a następnie wyświetla.
void wyswietl_samochody(int tryb_sort) {
    FILE* f = fopen(SAMOCHOD_FILE, "rb");
    if (!f) {
        printf("Brak danych samochodów.\n");
        return;
    }

    Samochod s;
    Samochod* samochody = NULL;  // Dynamiczna tablica przechowująca rekordy samochodów
    int count = 0;

    // Odczytujemy wszystkie rekordy, które są aktywne
    while (fread(&s, sizeof(Samochod), 1, f)) {
        if (s.aktywny) {
            samochody = realloc(samochody, (count + 1) * sizeof(Samochod));
            samochody[count++] = s;
        }
    }

    // Sortowanie bąbelkowe według wybranego kryterium
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (porownaj_samochody(&samochody[j], &samochody[j + 1], tryb_sort) > 0) {
                Samochod temp = samochody[j];
                samochody[j] = samochody[j + 1];
                samochody[j + 1] = temp;
            }
        }
    }

    // Wyświetlamy sformatowaną tabelę z danymi samochodów
    printf("\n%5s %-15s %-15s %-6s %-6s %-10s %-10s\n",
        "ID", "Marka", "Model", "Rok", "Moc", "Cena", "Właściciel");
    for (int i = 0; i < count; i++) {
        printf("%5d %-15s %-15s %-6d %-6.0f %-10.2f %-10d\n",
            samochody[i].id,
            samochody[i].marka,
            samochody[i].model,
            samochody[i].rok,
            samochody[i].moc,
            samochody[i].cena,
            samochody[i].id_wlasciciela);
    }

    free(samochody);
    fclose(f);
}

/* ====================== FUNKCJA GŁÓWNA Z MENU ====================== */
// W funkcji main prezentowane jest główne menu aplikacji.
// Użytkownik wybiera opcje, które odpowiadają różnym operacjom na danych: dodawanie, usuwanie, modyfikacja,
// sprzedaż oraz wyświetlanie list klientów i samochodów. Pętla while(1) zapewnia ciągłą pracę programu, dopóki użytkownik nie wybierze opcji wyjścia.
int main() {
    while (1) {
        printf("\n=== System salonu samochodowego ===\n");
        printf("1. Dodaj klienta\n");
        printf("2. Dodaj samochód\n");
        printf("3. Usuń klienta\n");
        printf("4. Usuń samochód\n");
        printf("5. Sprzedaż samochodu\n");
        printf("6. Modyfikuj dane\n");
        printf("7. Lista klientów\n");
        printf("8. Lista samochodów\n");
        printf("0. Wyjście\n");

        int opcja = wczytaj_liczbe("Wybierz opcję: ");

        switch (opcja) {
        case 1:
            dodaj_klienta();
            break;
        case 2:
            dodaj_samochod();
            break;
        case 3:
            usun_klienta();
            break;
        case 4:
            usun_samochod();
            break;
        case 5:
            sprzedaz_samochodu();
            break;
        case 6: {
            // Menu modyfikacji – wybór, czy modyfikujemy dane klienta czy samochodu
            printf("Modyfikuj:\n1. Klienta\n2. Samochód\n");
            int choice = wczytaj_liczbe("Wybierz: ");
            if (choice == 1)
                modyfikuj_klienta();
            else if (choice == 2)
                modyfikuj_samochod();
            else
                printf("Niepoprawny wybór!\n");
            break;
        }
        case 7: {
            // Wyświetlenie listy klientów z możliwością wyboru kryterium sortowania
            printf("\nSortuj klientów według:\n");
            printf("1. Nazwisko\n2. Imię\n3. Adres\n");
            int tryb = wczytaj_liczbe("Wybierz tryb sortowania: ");
            wyswietl_klientow(tryb);
            break;
        }
        case 8: {
            // Wyświetlenie listy samochodów z możliwością wyboru kryterium sortowania
            printf("\nSortuj samochody według:\n");
            printf("1. Marka\n2. Model\n3. Rok\n4. Moc\n5. Cena\n");
            int tryb = wczytaj_liczbe("Wybierz tryb sortowania: ");
            wyswietl_samochody(tryb);
            break;
        }
        case 0:
            exit(0);
        default:
            printf("Niepoprawna opcja!\n");
        }
    }
    return 0;
}
