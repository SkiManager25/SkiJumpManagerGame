#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <fstream>

// ---------- LOGIKA GRY ----------

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
    if (miejsce >= 1 && miejsce <= (int)tabela.size()) return tabela[miejsce - 1];
    return 0.0;
}

const std::string PLIK_ZAPISU = "kadra.txt";

void zapiszKadre(const std::vector<Zawodnik>& kadra) {
    std::ofstream plik(PLIK_ZAPISU);
    if (!plik) return;
    for (const auto& z : kadra) {
        plik << z.imie << ";" << z.technika << ";" << z.lot << ";" << z.ladowanie << ";" << z.punktyGeneralne << "\n";
    }
    plik.close();
}

std::vector<Zawodnik> wczytajKadre() {
    std::vector<Zawodnik> kadra;
    std::ifstream plik(PLIK_ZAPISU);
    if (!plik) return kadra;

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

std::string rozegrajKonkurs(std::vector<Zawodnik*>& sklad, const Skocznia& skocznia) {
    std::stringstream wynikTekst;
    wynikTekst << "=== " << skocznia.nazwa << " (K-" << skocznia.punktK << ") ===\n";

    std::vector<WynikSkoku> wyniki;
    for (auto* zawodnik : sklad) {
        double wiatr = (std::rand() % 21 - 10) / 10.0;
        wyniki.push_back(wykonajSkok(*zawodnik, skocznia, wiatr));
    }

    std::sort(wyniki.begin(), wyniki.end(), [](const WynikSkoku& a, const WynikSkoku& b) {
        return a.suma > b.suma;
    });

    for (size_t i = 0; i < wyniki.size(); i++) {
        int miejsce = i + 1;
        double pktGeneralne = punktyZaMiejsce(miejsce);
        wynikTekst << miejsce << ". " << wyniki[i].imieZawodnika << " - "
                   << (int)wyniki[i].suma << " pkt (+" << (int)pktGeneralne << ")\n";

        for (auto* zawodnik : sklad) {
            if (zawodnik->imie == wyniki[i].imieZawodnika) {
                zawodnik->punktyGeneralne += pktGeneralne;
                break;
            }
        }
    }
    wynikTekst << "\n";
    return wynikTekst.str();
}

std::string klasyfikacjaGeneralnaTekst(std::vector<Zawodnik*>& sklad) {
    std::vector<Zawodnik*> posortowani = sklad;
    std::sort(posortowani.begin(), posortowani.end(), [](Zawodnik* a, Zawodnik* b) {
        return a->punktyGeneralne > b->punktyGeneralne;
    });

    std::stringstream tekst;
    tekst << "=== KLASYFIKACJA GENERALNA ===\n";
    for (size_t i = 0; i < posortowani.size(); i++) {
        tekst << (i + 1) << ". " << posortowani[i]->imie << " - "
              << (int)posortowani[i]->punktyGeneralne << " pkt\n";
    }
    return tekst.str();
}

// ---------- UI ----------

struct PrzyciskZawodnika {
    std::string imie;
    sf::RectangleShape prostokat;
    sf::Text tekst;
    bool wybrany = false;

    PrzyciskZawodnika(const std::string& im, const sf::Font& font)
        : imie(im), tekst(font, im, 24) {}
};

enum class Ekran { WYBOR_SKLADU, WYNIKI };

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({800, 700}), "Ski Jumping Manager");

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        return -1;
    }

    std::vector<Zawodnik> kadra = wczytajKadre();
    if (kadra.empty()) {
        kadra.push_back(Zawodnik("Kamil Stoch", 85, 90, 80));
        kadra.push_back(Zawodnik("Dawid Kubacki", 88, 82, 85));
        kadra.push_back(Zawodnik("Piotr Zyla", 80, 85, 78));
        kadra.push_back(Zawodnik("Aleksander Zniszczol", 75, 78, 72));
        kadra.push_back(Zawodnik("Pawel Wasek", 78, 80, 76));
        kadra.push_back(Zawodnik("Jakub Wolny", 74, 76, 70));
    }
    for (auto& z : kadra) z.punktyGeneralne = 0;

    const int MAX_WYBOR = 4;
    const float SZEROKOSC_PRZYCISKU = 400.f;
    const float WYSOKOSC_PRZYCISKU = 50.f;
    const float ODSTEP = 10.f;
    const float START_Y = 80.f;

    std::vector<PrzyciskZawodnika> przyciski;
    for (size_t i = 0; i < kadra.size(); i++) {
        przyciski.emplace_back(kadra[i].imie, font);
        przyciski[i].prostokat.setSize({SZEROKOSC_PRZYCISKU, WYSOKOSC_PRZYCISKU});
        przyciski[i].prostokat.setPosition({200.f, START_Y + i * (WYSOKOSC_PRZYCISKU + ODSTEP)});
        przyciski[i].prostokat.setFillColor(sf::Color(50, 60, 80));
        przyciski[i].prostokat.setOutlineColor(sf::Color::White);
        przyciski[i].prostokat.setOutlineThickness(2.f);
        przyciski[i].tekst.setFillColor(sf::Color::White);
        przyciski[i].tekst.setPosition({215.f, START_Y + i * (WYSOKOSC_PRZYCISKU + ODSTEP) + 10.f});
    }

    sf::RectangleShape przyciskZatwierdz({200.f, 50.f});
    przyciskZatwierdz.setPosition({300.f, 500.f});
    przyciskZatwierdz.setFillColor(sf::Color(30, 120, 30));

    sf::Text tekstZatwierdz(font, "Zatwierdz sklad", 22);
    tekstZatwierdz.setFillColor(sf::Color::White);
    tekstZatwierdz.setPosition({315.f, 515.f});

    sf::Text naglowek(font, "Wybierz 4 zawodnikow", 30);
    naglowek.setFillColor(sf::Color::White);
    naglowek.setPosition({230.f, 20.f});

    sf::Text komunikatBledu(font, "", 20);
    komunikatBledu.setFillColor(sf::Color::Red);
    komunikatBledu.setPosition({230.f, 570.f});

    Ekran aktualnyEkran = Ekran::WYBOR_SKLADU;

    // Elementy ekranu wynikow
    sf::Text tekstWynikow(font, "", 18);
    tekstWynikow.setFillColor(sf::Color::White);
    tekstWynikow.setPosition({30.f, 20.f});

    sf::RectangleShape przyciskZakoncz({200.f, 50.f});
    przyciskZakoncz.setPosition({300.f, 630.f});
    przyciskZakoncz.setFillColor(sf::Color(120, 30, 30));

    sf::Text tekstZakoncz(font, "Zakoncz", 22);
    tekstZakoncz.setFillColor(sf::Color::White);
    tekstZakoncz.setPosition({345.f, 645.f});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f pozycjaMyszy(static_cast<float>(mouseEvent->position.x),
                                               static_cast<float>(mouseEvent->position.y));

                    if (aktualnyEkran == Ekran::WYBOR_SKLADU) {
                        int iluWybranych = 0;
                        for (auto& p : przyciski) if (p.wybrany) iluWybranych++;

                        for (auto& p : przyciski) {
                            if (p.prostokat.getGlobalBounds().contains(pozycjaMyszy)) {
                                if (p.wybrany) p.wybrany = false;
                                else if (iluWybranych < MAX_WYBOR) p.wybrany = true;
                            }
                        }

                        if (przyciskZatwierdz.getGlobalBounds().contains(pozycjaMyszy)) {
                            int policz = 0;
                            for (auto& p : przyciski) if (p.wybrany) policz++;

                            if (policz == MAX_WYBOR) {
                                std::vector<Zawodnik*> sklad;
                                for (size_t i = 0; i < przyciski.size(); i++) {
                                    if (przyciski[i].wybrany) sklad.push_back(&kadra[i]);
                                }

                                std::vector<Skocznia> kalendarz = {
                                    Skocznia("Wielka Krokiew", 125, 1.8),
                                    Skocznia("Innsbruck (Bergisel)", 128, 1.8),
                                    Skocznia("Willingen", 145, 1.8),
                                    Skocznia("Wisla (Malinka)", 116, 1.8)
                                };

                                std::string calyTekst;
                                for (const auto& skocznia : kalendarz) {
                                    calyTekst += rozegrajKonkurs(sklad, skocznia);
                                }
                                calyTekst += klasyfikacjaGeneralnaTekst(sklad);

                                tekstWynikow.setString(calyTekst);
                                zapiszKadre(kadra);

                                aktualnyEkran = Ekran::WYNIKI;
                            } else {
                                komunikatBledu.setString("Musisz wybrac dokladnie 4 zawodnikow!");
                            }
                        }
                    } else if (aktualnyEkran == Ekran::WYNIKI) {
                        if (przyciskZakoncz.getGlobalBounds().contains(pozycjaMyszy)) {
                            window.close();
                        }
                    }
                }
            }
        }

        for (auto& p : przyciski) {
            p.prostokat.setFillColor(p.wybrany ? sf::Color(30, 150, 30) : sf::Color(50, 60, 80));
        }

        window.clear(sf::Color(20, 30, 50));

        if (aktualnyEkran == Ekran::WYBOR_SKLADU) {
            window.draw(naglowek);
            for (auto& p : przyciski) {
                window.draw(p.prostokat);
                window.draw(p.tekst);
            }
            window.draw(przyciskZatwierdz);
            window.draw(tekstZatwierdz);
            window.draw(komunikatBledu);
        } else if (aktualnyEkran == Ekran::WYNIKI) {
            window.draw(tekstWynikow);
            window.draw(przyciskZakoncz);
            window.draw(tekstZakoncz);
        }

        window.display();
    }

    return 0;
}