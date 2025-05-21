// Biblioteki do obsługi wejścia/wyjścia, plików, struktur danych, pomiaru czasu i operacji na zbiorach
#include <iostream>       // Obsługa strumieni wejścia/wyjścia (np. cout, cerr)
#include <fstream>        // Obsługa operacji na plikach (ifstream, ofstream)
#include <vector>         // Kontener vector do przechowywania danych
#include <chrono>         // Pomiar czasu wykonania programu
#include <algorithm>      // Funkcje min i max do ograniczania zakresów
#include <unordered_set>  // Kontener do sprawdzania unikalności grup

using namespace std;      // Użyj przestrzeni nazw std dla uproszczenia zapisu

int n, k; // Globalne zmienne: n - liczba elementów, k - liczba grup

// Funkcja rekurencyjna generująca wszystkie podziały zbioru na dokładnie k grup
// Parametry:
// - index: aktualny indeks elementu do przypisania do grupy
// - maxGroup: najwyższy numer grupy użyty dotychczas
// - n: liczba elementów w zbiorze
// - k: docelowa liczba grup
// - partition: aktualny podział (przypisanie elementów do grup)
// - results: lista wszystkich poprawnych podziałów
void generatePartitions(int index, int maxGroup, int n, int k,
    vector<int>& partition, vector<vector<int>>& results) {

    // Warunek zakończenia rekurencji: wszystkie elementy są przypisane
    if (index == n) {
        // Bez dodatkowego sprawdzania – dodajemy każdy wygenerowany podział,
        // który nie przekracza k grup (rekurencja zapewnia to)
        results.push_back(partition);
        return;
    }

    // Próbuj przypisać aktualny element do każdej możliwej grupy
    // Nowa grupa może być utworzona tylko wtedy, gdy liczba grup nie przekroczy k
    for (int i = 1; i <= std::min(maxGroup + 1, k); ++i) {
        partition[index] = i; // Przypisz element do grupy i
        // Rekurencyjnie generuj podziały dla kolejnego elementu
        generatePartitions(index + 1, std::max(maxGroup, i), n, k, partition, results);
    }
}

int main() {
    // Otwórz plik wejściowy i wczytaj dane
    ifstream inputFile("input.txt");
    if (!inputFile) {
        cerr << "Blad: Nie mozna otworzyć pliku wejsciowego!" << endl;
        return 1;
    }
    inputFile >> n >> k; // Wczytaj liczbę elementów (n) i grup (k)
    inputFile.close();

    // Sprawdź poprawność danych wejściowych
    if (n <= 0 || k <= 0 || k > n) {
        cerr << "Blad: Niepoprawne wartosci n lub k!" << endl;
        return 1;
    }

    vector<int> partition(n);     // Aktualny podział (grupy dla każdego elementu)
    vector<vector<int>> results;  // Lista wszystkich poprawnych podziałów

    cout << "Wczytano poprawnie: n = " << n << ", k = " << k << endl;

    // Rozpocznij pomiar czasu generowania podziałów
    auto start = chrono::high_resolution_clock::now();

    // Wygeneruj wszystkie podziały (rekurencja)
    generatePartitions(0, 0, n, k, partition, results);

    // Zakończ pomiar czasu
    auto end = chrono::high_resolution_clock::now();

    // Zapisz wyniki do pliku output.txt
    ofstream outputFile("output.txt");
    if (!outputFile) {
        cerr << "Blad: Nie mozna otworzyc pliku wyjsciowego!" << endl;
        return 1;
    }

    // Zapisz każdy podział w formacie: "numer_grupy_1 numer_grupy_2 ..."
    for (const auto& p : results) {
        for (int val : p) {
            outputFile << val << " ";
        }
        outputFile << endl; // Nowa linia po każdym podziale
    }

    // Oblicz i wypisz czas wykonania w mikrosekundach
    chrono::duration<double, milli> duration = end - start;
    cout << "Czas wykonania: " << duration.count() << " ms" << endl;
    outputFile << "Czas wykonania: " << duration.count() << " ms" << endl;

    // Wypisz liczbę znalezionych podziałów (liczba Stirlinga drugiego rodzaju)
    cout << "Laczna liczba wygenerowanych podzialow: " << results.size() << endl;
    outputFile << "Laczna liczba wygenerowanych podzialow: " << results.size() << endl;

    outputFile.close(); // Zamknij plik wyjściowy

    return 0;
}