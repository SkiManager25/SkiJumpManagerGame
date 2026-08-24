#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iomanip>

class Zawodnik {
public:
    std::string imie;
    int technika;
    int lot;
    int ladowanie;
    int forma;

    Zawodnik(std::string i, int t, int l, int lad)
        : imie(i), technika(t), lot(l), ladowanie(lad), forma(50) {}

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
    double jakoscSkoku = (z.technika * 0.4 + z.lot * 0.4 +  z.forma * 0.2) / 100.0;
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

WynikSkoku wykonajSkok(Zawodnik&z, const Skocznia& s, double wiatr) {
    z.wylosujForme();

    double odleglosc = symulujOdleglosc(z, s);
    double punktyOdleglosc = s.punktyBazowe() + (odleglosc - s.punktK) * s.wartoscMetra;

    std::vector<double> noty = wylosujNotySedziowskie(z.ladowanie);
    double punktyStyl = policzPunktyStylu(noty);

    double rekompensataWiatru = wiatr * -6.0;

    double suma = punktyOdleglosc + punktyStyl + rekompensataWiatru;

    return {z.imie, odleglosc, punktyOdleglosc, punktyStyl, rekompensataWiatru, suma};
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Skocznia wielkaKrokiew("Wielka Krokiew", 125, 1.8);

    std::vector<Zawodnik> kadra = {
        Zawodnik("Kamil Stoch", 85, 90, 80),
        Zawodnik("Dawid Kubacki", 88, 82, 85),
        Zawodnik("Piotr Zyla", 80, 85, 78)
    };

    std::cout << "=== Konkurs na " << wielkaKrokiew.nazwa << " (K-" << wielkaKrokiew.punktK << ") ===\n\n";

    std::vector<WynikSkoku> wyniki;

    for(auto& zawodnik : kadra) {
        double wiatr = (std::rand() % 21 - 10) / 10.0;
        WynikSkoku w = wykonajSkok(zawodnik, wielkaKrokiew, wiatr);
        wyniki.push_back(w);

        std::cout << std::fixed << std::setprecision(1);
        std::cout << w.imieZawodnika << ":\n";
        std::cout << "  Odleglosc: " << w.odleglosc << " m\n";
        std::cout << "  Punkty za odleglosc: " << w.punktyOdleglosc << "\n";
        std::cout << "  Noty stylu: " << w.punktyStyl << "\n";
        std::cout << "  Rekompensata wiatru: " << w.rekompensataWiatru << "\n";
        std::cout << "  SUMA: " << w.suma << " pkt\n\n";
    }

    std::sort(wyniki.begin(), wyniki.end(), [](const WynikSkoku& a, const WynikSkoku& b) {
        return a.suma > b.suma;
    });

    std::cout << "=== WYNIKI KONCOWE ===\n";
    for (size_t i = 0; i < wyniki.size(); i++) {
        std::cout << (i + 1) << ". " << wyniki[i].imieZawodnika << " - " << wyniki[i].suma << " pkt\n";
    }

    return 0;
}