
#include <iostream>       // Operacje wejścia/wyjścia (cout, cin)
#include <fstream>        // Obsługa plików (ifstream, ofstream)
#include <queue>          // Kolejka priorytetowa (priority_queue)
#include <unordered_map>  // Mapa haszująca (unordered_map)
#include <chrono>         // Pomiar czasu (high_resolution_clock)
#include <string>         // Obsługa łańcuchów znaków (string)

using namespace std;
using namespace std::chrono;

// Struktura reprezentująca węzeł drzewa Huffmana
struct Node {
    char ch;       // Przechowywany znak (dla węzłów wewnętrznych: '\0')
    int freq;      // Częstość występowania znaku/suma częstości dzieci
    Node* left;    // Lewe dziecko (odpowiada bitowi 0)
    Node* right;   // Prawe dziecko (odpowiada bitowi 1)

    // Konstruktor inicjalizujący wszystkie pola
    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// Funktor do porównywania węzłów w kolejce priorytetowej
struct Compare {
    bool operator()(Node* a, Node* b) {
        // Kolejka priorytetowa sortuje rosnąco - mniejsze częstości mają wyższy priorytet
        return a->freq > b->freq;
    }
};

// Funkcja budująca drzewo Huffmana na podstawie mapy częstości
Node* buildHuffmanTree(const unordered_map<char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> pq;

    for (const auto& pair : freqMap) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Special case: tylko jeden unikalny znak
    if (pq.size() == 1) {
        Node* single = pq.top();
        pq.pop();
        Node* dummy = new Node('\0', 0); // Dummy node z częstością 0
        Node* root = new Node('\0', single->freq);
        root->left = single;
        root->right = dummy;
        pq.push(root);
    }

    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();

        Node* newNode = new Node('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    return pq.empty() ? nullptr : pq.top();
}

// Funkcja rekurencyjna generująca kody Huffmana dla znaków
void generateCodes(Node* root, const string& code, unordered_map<char, string>& codes) {
    if (!root) return; // Warunek bazowy: pusty węzeł

    // Jeśli węzeł jest liściem (zawiera znak)
    if (root->ch != '\0') {
        codes[root->ch] = code; // Zapisz kod w mapie
        return; // Nie schodź głębiej
    }

    // Przejdź w lewo - dodaj '0' do kodu
    generateCodes(root->left, code + "0", codes);
    // Przejdź w prawo - dodaj '1' do kodu
    generateCodes(root->right, code + "1", codes);
}

// Funkcja kodująca tekst na podstawie mapy kodów
string encode(const string& text, const unordered_map<char, string>& codes) {
    string encodedText;
    for (char c : text) {
        encodedText += codes.at(c); // Konkatenacja kodów dla każdego znaku
    }
    return encodedText;
}

// Funkcja dekodująca zakodowany tekst przy użyciu drzewa Huffmana
string decode(const string& encodedText, Node* root) {
    string decodedText;
    Node* current = root; // Zaczynamy od korzenia

    for (char bit : encodedText) {
        // Przejdź w lewo lub prawo w zależności od bitu
        current = (bit == '0') ? current->left : current->right;

        // Jeśli dotarliśmy do liścia
        if (!current->left && !current->right) {
            decodedText += current->ch; // Dodaj znak do wyniku
            current = root; // Wróć do korzenia dla następnego znaku
        }
    }
    return decodedText;
}

// Funkcja obliczająca metryki jakości kompresji
void calculateMetrics(const string& input, const string& encoded,
    const unordered_map<char, string>& codes,
    ofstream& output) {

    // Oblicz wskaźnik kompresji (stosunek rozmiarów przed i po)
    double compressionRatio = (input.length() * 8.0) / encoded.length();

    // Oblicz stopień kompresji (procentowa redukcja rozmiaru)
    double compressionDegree = 100.0 * (1 - encoded.length() / (input.length() * 8.0));

    // Oblicz średnią długość kodu
    unordered_map<char, int> freqMap;
    for (char c : input) freqMap[c]++; // Zlicz częstości znaków

    double avgCodeLength = 0.0;
    for (const auto& pair : codes) {
        // Długość kodu * częstość występowania
        avgCodeLength += freqMap[pair.first] * pair.second.length();
    }
    avgCodeLength /= input.length(); // Średnia dla wszystkich znaków

    // Zapisz wyniki do pliku
    output << "Wskaznik kompresji: " << compressionRatio << endl;
    output << "Stopien kompresji: " << compressionDegree << "%" << endl;
    output << "Srednia dlugosc kodu: " << avgCodeLength << " bitow na znak" << endl;
}

//Funkcja zwalniająca pamięć zajmowaną przez drzewo Huffmana
void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

int main() {
    // Otwórz plik wejściowy
    ifstream inputFile("input.txt");
    if (!inputFile.is_open()) {
        cerr << "Blad: Nie mozna otworzyc pliku input.txt" << endl;
        return 1;
    }

    // Wczytaj całą zawartość pliku
    string text((istreambuf_iterator<char>(inputFile)),
        istreambuf_iterator<char>());
    inputFile.close();

    // Sprawdź poprawność danych wejściowych
    if (text.empty()) {
        cerr << "Blad: Plik wejsciowy jest pusty" << endl;
        return 1;
    }
    if (text.length() < 2) {
        cerr << "Blad: Tekst musi zawierac co najmniej 2 znaki" << endl;
        return 1;
    }

    // Krok 1: Zlicz częstości znaków
    unordered_map<char, int> freqMap;
    for (char c : text) freqMap[c]++;

    // Krok 2: Zbuduj drzewo Huffmana
    auto startBuild = high_resolution_clock::now();
    Node* root = buildHuffmanTree(freqMap);
    auto stopBuild = high_resolution_clock::now();

    // Krok 3: Wygeneruj kody Huffmana
    unordered_map<char, string> codes;
    generateCodes(root, "", codes);

    // Krok 4: Zakoduj tekst
    auto startEncode = high_resolution_clock::now();
    string encoded = encode(text, codes);
    auto stopEncode = high_resolution_clock::now();

    // Krok 5: Dekoduj tekst
    auto startDecode = high_resolution_clock::now();
    string decoded = decode(encoded, root);
    auto stopDecode = high_resolution_clock::now();

    // Zapisz wyniki do pliku
    ofstream outputFile("output.txt");
    outputFile << "Tekst wejsciowy:\n" << text << "\n\n";
    outputFile << "Zakodowany tekst:\n" << encoded << "\n\n";
    outputFile << "Tekst po dekodowaniu:\n" << decoded << "\n\n";

    // Oblicz i zapisz metryki
    calculateMetrics(text, encoded, codes, outputFile);

    // Oblicz i zapisz czasy wykonania
    auto encode_time = duration_cast<nanoseconds>(stopEncode - startEncode).count();
    auto decode_time = duration_cast<nanoseconds>(stopDecode - startDecode).count();
    outputFile << "Czas kodowania: " << encode_time << " ns\n";
    outputFile << "Czas dekodowania: " << decode_time << " ns\n";

    outputFile.close();

    // Zwolnij pamięć zajmowaną przez drzewo
    deleteTree(root);
    return 0;
}