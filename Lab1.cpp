#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

// Funkcja wypisujaca wszystkie kombinacje do bufora (wektora stringow)
void saveCombination(const vector<int>& comb, vector<string>& buffer) {
    string line;
    for (int num : comb)
        line += to_string(num) + " ";
    buffer.push_back(line);
}

// Funkcja generujaca wszystkie kombinacje wg algorytmu klasycznego
void generateCombinations(int n, int k, vector<string>& buffer) {
    vector<int> comb(k);
    for (int i = 0; i < k; i++)
        comb[i] = i + 1;

    saveCombination(comb, buffer);

    while (true) {
        int i = k - 1;
        while (i >= 0 && comb[i] == n - k + i + 1)
            i--;
        if (i < 0) break;
        comb[i]++;
        for (int j = i + 1; j < k; j++)
            comb[j] = comb[i] + (j - i);
        saveCombination(comb, buffer);
    }
}

// Funkcja generująca kombinacje wg algorytmu Semby
void generateCombinationsSemby(int n, int k, vector<string>& buffer) {
    vector<int> comb(k);
    for (int i = 0; i < k; i++)
        comb[i] = i + 1;

    saveCombination(comb, buffer);

    while (true) {
        int i = k - 1;
        while (i >= 0 && (comb[i] - (i + 1)) == (n - k))
            i--;
        if (i < 0) break;
        comb[i]++;
        for (int j = i + 1; j < k; j++)
            comb[j] = comb[j - 1] + 1;
        saveCombination(comb, buffer);
    }
}

// Funkcja sprawdzająca poprawność danych wejściowych
bool validateInput(int n, int k) {
    return (n > 0 && k > 0 && k <= n);
}

int main() {
    ifstream ifs("input.txt");
    if (!ifs) {
        cerr << "Błąd: nie można otworzyć pliku input.txt\n";
        return 1;
    }

    int n, k;
    if (!(ifs >> n >> k)) {
        cerr << "Błąd: n i k muszą być liczbami całkowitymi\n";
        return 1;
    }
    ifs.close();

    if (!validateInput(n, k)) {
        cerr << "Błąd: niepoprawne dane wejściowe (n > 0, k > 0, k ≤ n)\n";
        return 1;
    }

    ofstream ofs("output.txt");
    if (!ofs) {
        cerr << "Błąd: nie można otworzyć pliku output.txt\n";
        return 1;
    }

    vector<string> classicalBuffer;
    vector<string> sembaBuffer;

    auto start = high_resolution_clock::now();
    generateCombinations(n, k, classicalBuffer);
    auto end = high_resolution_clock::now();
    cout << "Czas wykonania klasycznego algorytmu: "
        << duration_cast<microseconds>(end - start).count()
        << " us\n";
    ofs << "Czas wykonania klasycznego algorytmu: "
        << duration_cast<microseconds>(end - start).count()
        << " us\n";

    start = high_resolution_clock::now();
    generateCombinationsSemby(n, k, sembaBuffer);
    end = high_resolution_clock::now();
    cout << "Czas wykonania algorytmu Semby: "
        << duration_cast<microseconds>(end - start).count()
        << " us\n";
    ofs << "Czas wykonania algorytmu Semby: "
        << duration_cast<microseconds>(end - start).count()
        << " us\n";

    ofs << "\nKombinacje klasyczne\n";
    for (const auto& line : classicalBuffer)
        ofs << line << "\n";

    ofs << "Kombinacje Semby\n";
    for (const auto& line : sembaBuffer)
        ofs << line << "\n";

    ofs.close();
    return 0;
}
