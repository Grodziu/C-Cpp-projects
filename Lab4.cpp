#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <limits>

using namespace std;

// XOR modulo 2 na tapach
int xor_mod2(const vector<int>& state, const vector<int>& feedback_positions) {
    int result = 0;
    for (int pos : feedback_positions) {
        result ^= state.at(pos);
    }
    return result;
}

// Obliczenie entropii binarnej sekwencji
double calculate_entropy(const vector<int>& sequence) {
    int count0 = 0, count1 = 0;
    for (int bit : sequence) {
        if (bit == 0) ++count0;
        else ++count1;
    }
    double p0 = count0 / static_cast<double>(sequence.size());
    double p1 = count1 / static_cast<double>(sequence.size());
    double entropy = 0.0;
    if (p0 > 0.0) entropy -= p0 * log2(p0);
    if (p1 > 0.0) entropy -= p1 * log2(p1);
    return entropy;
}

int main() {
    try {
        ifstream infile("input.txt");
        if (!infile.is_open())
            throw runtime_error("Nie mozna otworzyc pliku input.txt");

        // Sprawdzenie poprawności n
        int n;
        infile >> n;
        if (infile.fail() || n <= 0)
            throw invalid_argument("Nieprawidlowa wartosc parametru n (musi byc dodatnia)");

        if (n > 64)
            throw invalid_argument("Maksymalna dlugosc rejestru to 64 bity");

        // Czyszczenie bufora po wczytaniu n
        infile.ignore(numeric_limits<streamsize>::max(), '\n');

        // Wczytanie stanu początkowego
        string state_line;
        getline(infile, state_line);
        istringstream state_stream(state_line);

        vector<int> initial_state;
        int bit;
        while (state_stream >> bit) {
            if (bit != 0 && bit != 1)
                throw invalid_argument("Nieprawidlowa wartosc bitu: " + to_string(bit));
            initial_state.push_back(bit);
        }

        if (initial_state.size() != static_cast<size_t>(n))
            throw invalid_argument("Nieprawidlowa liczba bitow w stanie poczatkowym");

        if (all_of(initial_state.begin(), initial_state.end(), [](int b) { return b == 0; }))
            throw invalid_argument("Stan poczatkowy nie moze byc zerowy");

        // Wczytanie tapów
        string taps_line;
        getline(infile, taps_line);
        if (taps_line.empty())
            throw invalid_argument("Brak linii z pozycjami sprzezenia zwrotnego");

        istringstream taps_stream(taps_line);
        vector<int> feedback_positions;
        int tap;
        while (taps_stream >> tap) {
            if (tap < 0 || tap >= n)
                throw out_of_range("Nieprawidlowa pozycja tapu: " + to_string(tap));
            feedback_positions.push_back(tap);
        }

        // Usunięcie duplikatów i sortowanie
        sort(feedback_positions.begin(), feedback_positions.end());
        auto last = unique(feedback_positions.begin(), feedback_positions.end());
        feedback_positions.erase(last, feedback_positions.end());

        if (feedback_positions.empty())
            throw invalid_argument("Brak poprawnych pozycji sprzezenia zwrotnego");

        // Reszta kodu bez zmian
        uint64_t max_length = (1ULL << n) - 1;
        vector<int> state = initial_state;
        vector<int> sequence;
        sequence.reserve(max_length);

        auto t_start = chrono::high_resolution_clock::now();

        for (uint64_t i = 0; i < max_length; ++i) {
            int output_bit = state.back();
            sequence.push_back(output_bit);

            int feedback = xor_mod2(state, feedback_positions);
            for (int j = n - 1; j > 0; --j) {
                state[j] = state[j - 1];
            }
            state[0] = feedback;
        }

        auto t_end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::nanoseconds>(t_end - t_start);

        double entropy = calculate_entropy(sequence);

        ofstream outfile("output.csv");
        if (!outfile.is_open())
            throw runtime_error("Nie mozna otworzyc pliku output.csv");

        outfile << "Dlugosc: " << sequence.size() << "\n"
            << "Entropia: " << entropy << "\n"
            << "Czas: " << duration.count() << " ns\n";

        for (int b : sequence) {
            outfile << b << "\n";
        }

        cout << "Generowanie zakonczone pomyslnie!\n";
    }
    catch (const exception& ex) {
        cerr << "BLAD: " << ex.what() << endl;
        return 1;
    }
    return 0;
}