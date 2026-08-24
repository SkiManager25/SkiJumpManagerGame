#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>

class Zawodnik {
public:
    std::string imie;
    int technika;
    int lot;
    int ladowanie;
    int forma;
    double punktyGeneralne;

    Zawodnik(std::string i, int t, int l, int lad)
        : imie(i), technika(t), lot(l), ladowanie(lad), forma(50), punktyGeneralne(0) {}

    void wylosujForme() {
        forma = 40 + (std::rand() % 21);
    }
};

class Skocznia {
public:
    std::string nazwa;
    int punktK;
    double wartoscMetra;

    Skocznia(std::string n, int k, double wm) : nazwa(n), punktK(k), wartoscMetra(wm) {}

    int punktyBazowe() const {
        return (wartoscMetra >= 1.8) ? 120 : 60;
    }
};

struct WynikSkoku {
    std::string imieZawodnika;
    double odleglosc;
    double punktyOdleglosc;
    double punktyStyl;
    double rekompensataWiatru;
    double suma;
};

double symulujOdleglosc(const Zawodnik& z, const Skocznia& s) {
    double jakoscSkoku = (z.technika * 0.4 + z.lot * 0.4 + z.forma * 0.2) / 100.0;
    double odchylenie = (std::rand() % 21) - 10;
    double odleglosc = s.punktK * (0.85 + jakoscSkoku * 0.3) + odchylenie;
    return std::max(0.0, odleglosc);
}

std::vector<double> wylosujNotySedziowskie(int ladowanie) {
    std::vector<double> noty;
    for (int i = 0; i < 5; i++) {
        double baza = 12.0 + (ladowanie / 100.0) * 6.0;
        double losowosc = (std::rand() % 21 - 10) / 10.0;
        double nota = std::min(20.0, std::max(0.0, baza + losowosc));
        noty.push_back(nota);
    }
    return noty;
}

double policzPunktyStylu(std::vector<double> noty) {
    std::sort(noty.begin(), noty.end());
    return noty[1] + noty[2] + noty[3];
}

WynikSkoku wykonajSkok(Zawodnik& z, const Skocznia& s, double wiatr) {
    z.wylosujForme();

    double odleglosc = symulujOdleglosc(z, s);
    double punktyOdleglosc = s.punktyBazowe() + (odleglosc - s.punktK) * s.wartoscMetra;

    std::vector<double> noty = wylosujNotySedziowskie(z.ladowanie);
    double punktyStyl = policzPunktyStylu(noty);

    double rekompensataWiatru = wiatr * -6.0;
    double suma = punktyOdleglosc + punktyStyl + rekompensataWiatru;

    return {z.imie, odleglosc, punktyOdleglosc, punktyStyl, rekompensataWiatru, suma};
}

double punktyZaMiejsce(int miejsce) {
    static std::vector<double> tabela = {100, 80, 60, 50, 45, 40, 36, 32, 29, 26, 24, 22, 20, 18, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    if (miejsce >= 1 && miejsce <= (int)tabela.size()) {
        return tabela[miejsce - 1];
    }
    return 0.0;
}

const std::string PLIK_ZAPISU = "kadra.txt";

void zapiszKadre(const std::vector<Zawodnik>& kadra) {
    std::ofstream plik(PLIK_ZAPISU);
    if (!plik) {
        std::cout << "Blad: nie mozna otworzyc pliku do zapisu.\n";
        return;
    }
    for (const auto& z : kadra) {
        plik << z.imie << ";" << z.technika << ";" << z.lot << ";" << z.ladowanie << ";" << z.punktyGeneralne << "\n";
    }
    plik.close();
}

std::vector<Zawodnik> wczytajKadre() {
    std::vector<Zawodnik> kadra;
    std::ifstream plik(PLIK_ZAPISU);

    if (!plik) {
        return kadra;
    }

    std::string linia;
    while (std::getline(plik, linia)) {
        std::stringstream ss(linia);
        std::string imie, technikaStr, lotStr, ladowanieStr, punktyStr;

        std::getline(ss, imie, ';');
        std::getline(ss, technikaStr, ';');
        std::getline(ss, lotStr, ';');
        std::getline(ss, ladowanieStr, ';');
        std::getline(ss, punktyStr, ';');

        if (imie.empty()) continue;

        Zawodnik z(imie, std::stoi(technikaStr), std::stoi(lotStr), std::stoi(ladowanieStr));
        if (!punktyStr.empty()) z.punktyGeneralne = std::stod(punktyStr);

        kadra.push_back(z);
    }
    plik.close();
    return kadra;
}

void rozegrajKonkurs(std::vector<Zawodnik*>& sklad, const Skocznia& skocznia) {
    std::cout << "\n=== Konkurs na " << skocznia.nazwa << " (K-" << skocznia.punktK << ") ===\n\n";

    std::vector<WynikSkoku> wyniki;

    for (auto* zawodnik : sklad) {
        double wiatr = (std::rand() % 21 - 10) / 10.0;
        WynikSkoku w = wykonajSkok(*zawodnik, skocznia, wiatr);
        wyniki.push_back(w);
    }

    std::sort(wyniki.begin(), wyniki.end(), [](const WynikSkoku& a, const WynikSkoku& b) {
        return a.suma > b.suma;
    });

    std::cout << std::fixed << std::setprecision(1);
    for (size_t i = 0; i < wyniki.size(); i++) {
        int miejsce = i + 1;
        double pktGeneralne = punktyZaMiejsce(miejsce);

        std::cout << miejsce << ". " << wyniki[i].imieZawodnika
                   << " - " << wyniki[i].suma << " pkt konkursu"
                   << " (+" << pktGeneralne << " do generalki)\n";

        for (auto* zawodnik : sklad) {
            if (zawodnik->imie == wyniki[i].imieZawodnika) {
                zawodnik->punktyGeneralne += pktGeneralne;
                break;
            }
        }
    }
}

void pokazKlasyfikacjeGeneralna(std::vector<Zawodnik> kadra) {
    std::sort(kadra.begin(), kadra.end(), [](const Zawodnik& a, const Zawodnik& b) {
        return a.punktyGeneralne > b.punktyGeneralne;
    });

    std::cout << "\n=== KLASYFIKACJA GENERALNA ===\n";
    for (size_t i = 0; i < kadra.size(); i++) {
        std::cout << (i + 1) << ". " << kadra[i].imie << " - " << kadra[i].punktyGeneralne << " pkt\n";
    }
}

std::vector<Zawodnik*> wybierzSklad(std::vector<Zawodnik>& kadra, int iluWybrac) {
    std::cout << "\n=== Twoja kadra ===\n";
    for (size_t i = 0; i < kadra.size(); i++) {
        std::cout << (i + 1) << ". " << kadra[i].imie
                   << " (Technika: " << kadra[i].technika
                   << ", Lot: " << kadra[i].lot
                   << ", Ladowanie: " << kadra[i].ladowanie << ")\n";
    }

    std::vector<Zawodnik*> sklad;
    std::cout << "\nWybierz " << iluWybrac << " zawodnikow na sezon (wpisz numery oddzielone spacja): ";

    for (int i = 0; i < iluWybrac; i++) {
        int wybor;
        std::cin >> wybor;
        if (wybor >= 1 && wybor <= (int)kadra.size()) {
            sklad.push_back(&kadra[wybor - 1]);
        }
    }

    return sklad;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::vector<Zawodnik> kadra = wczytajKadre();

    if (kadra.empty()) {
        std::cout << "Brak zapisu - tworze nowa kadre startowa.\n";
        kadra.push_back(Zawodnik("Kamil Stoch", 85, 90, 80));
        kadra.push_back(Zawodnik("Dawid Kubacki", 88, 82, 85));
        kadra.push_back(Zawodnik("Piotr Zyla", 80, 85, 78));
        kadra.push_back(Zawodnik("Aleksander Zniszczol", 75, 78, 72));
        kadra.push_back(Zawodnik("Pawel Wasek", 78, 80, 76));
        kadra.push_back(Zawodnik("Jakub Wolny", 74, 76, 70));
    } else {
        std::cout << "Wczytano zapisana kadre (" << kadra.size() << " zawodnikow).\n";
    }

    std::vector<Skocznia> kalendarz = {
        Skocznia("Wielka Krokiew", 125, 1.8),
        Skocznia("Innsbruck (Bergisel)", 120, 1.8),
        Skocznia("Willingen (Muehlenkopfschanze)", 130, 1.8),
        Skocznia("Wisla (Malinka)", 120, 1.8)
    };

    std::vector<Zawodnik*> sklad = wybierzSklad(kadra, 4);

    for (const auto& skocznia : kalendarz) {
        rozegrajKonkurs(sklad, skocznia);
    }

    pokazKlasyfikacjeGeneralna(kadra);

    zapiszKadre(kadra);
    std::cout << "\nZapisano stan kadry do pliku " << PLIK_ZAPISU << "\n";

    return 0;
}