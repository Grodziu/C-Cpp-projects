#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

// Struktura dla węzła listy dwukierunkowej
struct Wezel {
    int wartosc;
    Wezel* poprzedni;
    Wezel* nastepny;
    Wezel(int val) : wartosc(val), poprzedni(nullptr), nastepny(nullptr) {}
};

// Klasa listy dwukierunkowej
class ListaDwukierunkowa {
public:
    Wezel* glowa;
    Wezel* ogon;

    ListaDwukierunkowa() : glowa(nullptr), ogon(nullptr) {}

    void wstaw(int wartosc) {
        Wezel* nowyWezel = new Wezel(wartosc);
        if (!glowa) {
            glowa = ogon = nowyWezel;
        }
        else {
            ogon->nastepny = nowyWezel;
            nowyWezel->poprzedni = ogon;
            ogon = nowyWezel;
        }
    }

    void usun(int wartosc) {
        Wezel* temp = glowa;
        while (temp) {
            if (temp->wartosc == wartosc) {
                if (temp->poprzedni) temp->poprzedni->nastepny = temp->nastepny;
                if (temp->nastepny) temp->nastepny->poprzedni = temp->poprzedni;
                if (temp == glowa) glowa = temp->nastepny;
                if (temp == ogon) ogon = temp->poprzedni;
                delete temp;
                return;
            }
            temp = temp->nastepny;
        }
    }

    bool zawiera(int wartosc) const {
        Wezel* temp = glowa;
        while (temp) {
            if (temp->wartosc == wartosc) return true;
            temp = temp->nastepny;
        }
        return false;
    }

    void wyswietl() const {
        Wezel* temp = glowa;
        while (temp) {
            cout << temp->wartosc << " ";
            temp = temp->nastepny;
        }
        cout << endl;
    }
};

// Klasa reprezentująca zbiór
class Zbior {
private:
    ListaDwukierunkowa elementy;

public:
    void dodaj(int element) {
        if (!elementy.zawiera(element)) {
            elementy.wstaw(element);
        }
    }

    void usun(int element) {
        elementy.usun(element);
    }

    bool zawiera(int element) const {
        return elementy.zawiera(element);
    }

    void wyswietl() const {
        elementy.wyswietl();
    }

    Zbior suma(const Zbior& inny) const {
        Zbior wynik;
        Wezel* temp = elementy.glowa;
        while (temp) {
            wynik.dodaj(temp->wartosc);
            temp = temp->nastepny;
        }
        temp = inny.elementy.glowa;
        while (temp) {
            wynik.dodaj(temp->wartosc);
            temp = temp->nastepny;
        }
        return wynik;
    }

    Zbior iloczyn(const Zbior& inny) const {
        Zbior wynik;
        Wezel* temp = elementy.glowa;
        while (temp) {
            if (inny.zawiera(temp->wartosc)) {
                wynik.dodaj(temp->wartosc);
            }
            temp = temp->nastepny;
        }
        return wynik;
    }

    Zbior roznica(const Zbior& inny) const {
        Zbior wynik;
        Wezel* temp = elementy.glowa;
        while (temp) {
            if (!inny.zawiera(temp->wartosc)) {
                wynik.dodaj(temp->wartosc);
            }
            temp = temp->nastepny;
        }
        return wynik;
    }

    bool czyPodzbior(const Zbior& inny) const {
        Wezel* temp = elementy.glowa;
        while (temp) {
            if (!inny.zawiera(temp->wartosc)) {
                return false; // Jeśli jakiś element z A nie należy do B, A nie jest podzbiorem B
            }
            temp = temp->nastepny;
        }
        return true; // Wszystkie elementy z A należą do B
    }

    bool czyNadzbior(const Zbior& inny) const {
        return inny.czyPodzbior(*this); // A jest nadzbiorem B, jeśli B jest podzbiorem A
    }
};

// Funkcja do parsowania wyrażeń na zbiorach
Zbior parsujWyrazenie(const string& wyrazenie, const unordered_map<char, Zbior>& zbiory) {
    Zbior wynik;
    char operacja = '+';
    for (char znak : wyrazenie) {
        if (zbiory.find(znak) != zbiory.end()) {
            Zbior biezacyZbior = zbiory.at(znak);
            if (operacja == '+') {
                wynik = wynik.suma(biezacyZbior);
            }
            else if (operacja == '*') {
                wynik = wynik.iloczyn(biezacyZbior);
            }
            else if (operacja == '-') {
                wynik = wynik.roznica(biezacyZbior);
            }
        }
        else if (znak == '+' || znak == '*' || znak == '-') {
            operacja = znak;
        }
    }
    return wynik;
}

int main() {

    cout << "Legenda:\n + suma\n * iloczyn\n - roznica\n < zawiera sie w\n > wynika z\n\n";

    unordered_map<char, Zbior> zbiory;
    string wejscie;

    while (true) {
        cout << "Podaj operacje (np. A={1,2,3}, A+B, A*B, A-B, A<B, A>B, koniec): ";
        getline(cin, wejscie);

        if (wejscie == "koniec") {
            break;
        }

        if (wejscie.find("=") != string::npos) {
            char nazwaZbioru = wejscie[0];
            Zbior nowyZbior;
            size_t start = wejscie.find('{') + 1;
            size_t koniec = wejscie.find('}');
            string elementyStr = wejscie.substr(start, koniec - start);
            size_t pos = 0;
            while ((pos = elementyStr.find(',')) != string::npos) {
                int element = stoi(elementyStr.substr(0, pos));
                nowyZbior.dodaj(element);
                elementyStr.erase(0, pos + 1);
            }
            if (!elementyStr.empty()) {
                int element = stoi(elementyStr);
                nowyZbior.dodaj(element);
            }
            zbiory[nazwaZbioru] = nowyZbior;
            cout << "Zbior " << nazwaZbioru << " zdefiniowany." << endl;
        }
        else if (wejscie.find('+') != string::npos || wejscie.find('*') != string::npos || wejscie.find('-') != string::npos) {
            Zbior wynik = parsujWyrazenie(wejscie, zbiory);
            cout << "Wynik: ";
            wynik.wyswietl();
        }
        else if (wejscie.find("<") != string::npos) {
            char ZbiorA = wejscie[0];
            char ZbiorB = wejscie[wejscie.find("<") + 1];
            if (zbiory[ZbiorA].czyPodzbior(zbiory[ZbiorB])) {
                cout << ZbiorA << " jest podzbiorem " << ZbiorB << endl;
            }
            else {
                cout << ZbiorA << " nie jest podzbiorem " << ZbiorB << endl;
            }
        }
        else if (wejscie.find(">") != string::npos) {
            char ZbiorA = wejscie[0];
            char ZbiorB = wejscie[wejscie.find(">") + 1];
            if (zbiory[ZbiorA].czyNadzbior(zbiory[ZbiorB])) {
                cout << ZbiorA << " jest nadzbiorem " << ZbiorB << endl;
            }
            else {
                cout << ZbiorA << " nie jest nadzbiorem " << ZbiorB << endl;
            }
        }
        else {
            cout << "Nieznana operacja." << endl;
        }
    }

    return 0;
}