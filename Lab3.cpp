//Legenda - input.txt
// K - liczba poziomow podzialow
// E - liczba krawedzi do wygenerowania
// a,b,c,d - prawdopodobienstwo dla ćwiartek: A, B, C, D

#include <iostream>      // standardowe wejście-wyjście
#include <fstream>       // pliki (ifstream, ofstream)
#include <vector>        // struktura vector
#include <random>        // losowanie liczb
#include <chrono>        // pomiar czasu
#include <cmath>         // operacje matematyczne
#include <string>        // do pracy z tekstem
#include <map>           // do histogramu

using namespace std;

// Globalne zmienne pobierane z pliku wejściowego
int K;                  // liczba poziomów podziałów
int E;                  // liczba krawędzi do wygenerowania
double a, b, c, d;      // prawdopodobieństwa dla ćwiartek: A, B, C, D

int N;                  // liczba wierzchołków = 2^K
vector<vector<int>> A;  // macierz sąsiedztwa

// Funkcja wczytuje dane wejściowe z pliku
void read_input(const string& filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Blad otwarcia pliku wejsciowego: " << filename << endl;
        exit(1);
    }
    infile >> K;             // np. 3
    infile >> E;             // np. 15
    infile >> a >> b >> c >> d;  // np. 0.45 0.15 0.15 0.25
    infile.close();

    // Wyznaczenie liczby wierzchołków: N = 2^K
    N = 1 << K; // czyli N = 2^K
    // Inicjalizacja macierzy sąsiedztwa N x N wypełnionej zerami
    A = vector<vector<int>>(N, vector<int>(N, 0));
}

// Funkcja generująca graf metodą R-MAT
void generate_RMAT() {
    random_device rd;                           // źródło losowości
    mt19937 gen(rd());                          // generator 
    uniform_real_distribution<double> dis(0.0, 1.0); // losowa liczba z przedziału [0, 1]

    // Generowanie E krawędzi
    for (int i = 0; i < E; ++i) {
        int x = 0, y = 0;
        // Dokonujemy K podziałów, ustalając dokładne współrzędne krawędzi
        for (int j = 0; j < K; ++j) {
            double p = dis(gen);
            if (p < a) {
                // Ćwiartka A (lewa góra) – brak przesunięcia
            }
            else if (p < a + b) {
                // Ćwiartka B (prawa góra) – przesunięcie w prawo
                y += (1 << (K - j - 1));
            }
            else if (p < a + b + c) {
                // Ćwiartka C (lewy dół) – przesunięcie w dół
                x += (1 << (K - j - 1));
            }
            else {
                // Ćwiartka D (prawy dół) – przesunięcie w dół i w prawo
                x += (1 << (K - j - 1));
                y += (1 << (K - j - 1));
            }
        }
        // Jeśli w wybranej komórce nie ma krawędzi, ustawiamy 1
        if (A[x][y] == 0) {
            A[x][y] = 1;
        }
        else {
            i--;  // Jeśli krawędź już istnieje, powtarzamy próbę
        }
    }
}

// Funkcja zapisująca wyniki do pliku wyjściowego
void save_to_file(const string& filename, long long duration_us) {
    ofstream out(filename);
    if (!out) {
        cerr << "Blad otwarcia pliku wyjsciowego: " << filename << endl;
        exit(1);
    }

    out << "\nLiczba wierzcholkow N: " << N << "\n\n";

    // Zapis macierzy sąsiedztwa
    for (const auto& row : A) {
        for (int val : row) {
            out << val << " ";
        }
        out << "\n";
    }
    out << "\n";

    // Obliczenie gęstości grafu:
    // Przyjmujemy, że maksymalna liczba możliwych krawędzi (bez pętli) wynosi N*(N-1)
    double density = static_cast<double>(E) / (N * (N - 1));
    out << "Gestosc grafu: " << density << "\n";

    // Obliczenie histogramu rozkładu liczby połączeń (liczba jedynek w każdym wierszu)
    map<int, int> degree_histogram;
    int no_edges = 0;
    for (int i = 0; i < N; ++i) {
        int degree = 0;
        for (int j = 0; j < N; ++j) {
            if (A[i][j] == 1) {
                degree++;
            }
        }
        if (degree == 0) {
            no_edges++;
        }
        degree_histogram[degree]++;
    }

    out << "\nHistogram rozkladu liczby polaczen:\n";
    for (const auto& entry : degree_histogram) {
        out << "Liczba wierzcholkow z " << entry.first
            << " polaczeniami: " << entry.second << "\n";
    }

    // Zapis czasu generowania grafu
    out << "\nCzas generowania: " << duration_us << " us\n";

    out.close();
}

int main() {
    // Wczytanie danych wejściowych z pliku input.txt
    read_input("input.txt");

    // Pomiar czasu generowania grafu
    auto start = chrono::high_resolution_clock::now();
    generate_RMAT();
    auto end = chrono::high_resolution_clock::now();
    long long duration_us = chrono::duration_cast<chrono::microseconds>(end - start).count();

    // Zapis wyników do pliku output.txt
    save_to_file("output.txt", duration_us);

    cout << "Graf R-MAT zostal wygenerowany i zapisany do pliku output.txt." << endl;
    cout << "Czas generowania: " << duration_us << " us." << endl;

    return 0;
}
