#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <fstream>

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

struct WierszWyniku {
    std::string tresc;
    sf::Color kolor;
};

struct KonkursDoRysowania {
    std::string tytul;
    std::vector<WierszWyniku> wiersze;
};

sf::Color kolorMiejsca(int miejsce) {
    if (miejsce == 1) return sf::Color(255, 215, 0);
    if (miejsce == 2) return sf::Color(200, 200, 210);
    if (miejsce == 3) return sf::Color(205, 127, 50);
    return sf::Color(220, 220, 220);
}

KonkursDoRysowania rozegrajKonkurs(std::vector<Zawodnik*>& sklad, const Skocznia& skocznia) {
    KonkursDoRysowania wynik;
    std::stringstream tytul;
    tytul << skocznia.nazwa << "  (K-" << skocznia.punktK << ")";
    wynik.tytul = tytul.str();

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

        std::stringstream linia;
        linia << miejsce << ".  " << wyniki[i].imieZawodnika << "   -   "
              << (int)wyniki[i].suma << " pkt   (+" << (int)pktGeneralne << ")";

        wynik.wiersze.push_back({linia.str(), kolorMiejsca(miejsce)});

        for (auto* zawodnik : sklad) {
            if (zawodnik->imie == wyniki[i].imieZawodnika) {
                zawodnik->punktyGeneralne += pktGeneralne;
                break;
            }
        }
    }
    return wynik;
}

KonkursDoRysowania klasyfikacjaGeneralnaDoRysowania(std::vector<Zawodnik*>& sklad) {
    KonkursDoRysowania wynik;
    wynik.tytul = "KLASYFIKACJA GENERALNA";

    std::vector<Zawodnik*> posortowani = sklad;
    std::sort(posortowani.begin(), posortowani.end(), [](Zawodnik* a, Zawodnik* b) {
        return a->punktyGeneralne > b->punktyGeneralne;
    });

    for (size_t i = 0; i < posortowani.size(); i++) {
        int miejsce = i + 1;
        std::stringstream linia;
        linia << miejsce << ".  " << posortowani[i]->imie << "   -   "
              << (int)posortowani[i]->punktyGeneralne << " pkt";
        wynik.wiersze.push_back({linia.str(), kolorMiejsca(miejsce)});
    }
    return wynik;
}

struct PrzyciskZawodnika {
    std::string imie;
    sf::RectangleShape prostokat;
    sf::Text tekst;
    sf::Text statystyki;
    bool wybrany = false;

    PrzyciskZawodnika(const Zawodnik& z, const sf::Font& font)
        : imie(z.imie), tekst(font, z.imie, 22), statystyki(font, "", 16)
    {
        std::stringstream staty;
        staty << "TEC " << z.technika << "   LOT " << z.lot << "   LAD " << z.ladowanie;
        statystyki.setString(staty.str());
    }
};

enum class StatTyp { TECHNIKA, LOT, LADOWANIE };

struct PrzyciskTreningu {
    Zawodnik* zawodnik;
    StatTyp typ;
    sf::RectangleShape prostokat;

    int wartosc() const {
        if (typ == StatTyp::TECHNIKA) return zawodnik->technika;
        if (typ == StatTyp::LOT) return zawodnik->lot;
        return zawodnik->ladowanie;
    }

    void dodaj() {
        if (typ == StatTyp::TECHNIKA) { if (zawodnik->technika < 100) zawodnik->technika++; }
        else if (typ == StatTyp::LOT) { if (zawodnik->lot < 100) zawodnik->lot++; }
        else { if (zawodnik->ladowanie < 100) zawodnik->ladowanie++; }
    }

    std::string nazwaStatu() const {
        if (typ == StatTyp::TECHNIKA) return "TEC";
        if (typ == StatTyp::LOT) return "LOT";
        return "LAD";
    }
};

enum class Ekran { MENU_GLOWNE, WYBOR_SKLADU, TRENING, WYNIKI };

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({1920, 1080}), "Ski Jumping Manager", sf::State::Fullscreen);

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
        przyciski.emplace_back(kadra[i], font);

        float y = START_Y + i * (WYSOKOSC_PRZYCISKU + ODSTEP);

        przyciski[i].prostokat.setSize({SZEROKOSC_PRZYCISKU, WYSOKOSC_PRZYCISKU});
        przyciski[i].prostokat.setPosition({200.f, y});
        przyciski[i].prostokat.setFillColor(sf::Color(45, 55, 75));
        przyciski[i].prostokat.setOutlineColor(sf::Color(90, 105, 130));
        przyciski[i].prostokat.setOutlineThickness(2.f);

        przyciski[i].tekst.setFillColor(sf::Color::White);
        przyciski[i].tekst.setPosition({220.f, y + 6.f});

        przyciski[i].statystyki.setFillColor(sf::Color(160, 180, 210));
        przyciski[i].statystyki.setPosition({220.f, y + 34.f});
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

    std::vector<KonkursDoRysowania> wynikiDoRysowania;

    sf::RectangleShape przyciskZakoncz({200.f, 50.f});
    przyciskZakoncz.setPosition({480.f, 630.f});
    przyciskZakoncz.setFillColor(sf::Color(120, 30, 30));

    sf::Text tekstZakoncz(font, "Zakoncz gre", 20);
    tekstZakoncz.setFillColor(sf::Color::White);
    tekstZakoncz.setPosition({495.f, 645.f});

    sf::RectangleShape przyciskNastepnySezon({200.f, 50.f});
    przyciskNastepnySezon.setPosition({120.f, 630.f});
    przyciskNastepnySezon.setFillColor(sf::Color(30, 120, 60));

    sf::Text tekstNastepnySezon(font, "Nastepny sezon", 18);
    tekstNastepnySezon.setFillColor(sf::Color::White);
    tekstNastepnySezon.setPosition({135.f, 645.f});

    sf::VertexArray tloGradient(sf::PrimitiveType::TriangleStrip, 4);
    tloGradient[0].position = {0.f, 0.f};
    tloGradient[0].color = sf::Color(12, 18, 32);
    tloGradient[1].position = {1920.f, 0.f};
    tloGradient[1].color = sf::Color(12, 18, 32);
    tloGradient[2].position = {0.f, 1080.f};
    tloGradient[2].color = sf::Color(35, 55, 90);
    tloGradient[3].position = {1920.f, 1080.f};
    tloGradient[3].color = sf::Color(35, 55, 90);

    sf::ConvexShape gora;
    gora.setPointCount(5);
    gora.setPoint(0, {0.f, 1080.f});
    gora.setPoint(1, {0.f, 950.f});
    gora.setPoint(2, {700.f, 700.f});
    gora.setPoint(3, {1920.f, 950.f});
    gora.setPoint(4, {1920.f, 1080.f});
    gora.setFillColor(sf::Color(18, 26, 42));

    sf::Text tytulGry(font, "SKI JUMPING MANAGER", 72);
    tytulGry.setFillColor(sf::Color(255, 200, 60));
    tytulGry.setPosition({510.f, 280.f});

    sf::Text podtytulGry(font, "Zarzadzaj kadra i wygrywaj!", 26);
    podtytulGry.setFillColor(sf::Color(160, 180, 210));
    podtytulGry.setPosition({650.f, 380.f});

    sf::RectangleShape liniaOzdobna({700.f, 3.f});
    liniaOzdobna.setPosition({610.f, 430.f});
    liniaOzdobna.setFillColor(sf::Color(255, 200, 60));

    sf::RectangleShape cienStart({400.f, 90.f});
    cienStart.setPosition({766.f, 508.f});
    cienStart.setFillColor(sf::Color(0, 0, 0, 100));

    sf::RectangleShape przyciskStart({400.f, 90.f});
    przyciskStart.setPosition({760.f, 500.f});
    przyciskStart.setFillColor(sf::Color(35, 140, 60));
    przyciskStart.setOutlineColor(sf::Color(80, 200, 110));
    przyciskStart.setOutlineThickness(3.f);

    sf::Text tekstStart(font, "START", 36);
    tekstStart.setFillColor(sf::Color::White);
    tekstStart.setPosition({895.f, 528.f});

    sf::RectangleShape cienWyjdz({400.f, 80.f});
    cienWyjdz.setPosition({766.f, 628.f});
    cienWyjdz.setFillColor(sf::Color(0, 0, 0, 100));

    sf::RectangleShape przyciskWyjdz({400.f, 80.f});
    przyciskWyjdz.setPosition({760.f, 620.f});
    przyciskWyjdz.setFillColor(sf::Color(140, 35, 35));
    przyciskWyjdz.setOutlineColor(sf::Color(200, 80, 80));
    przyciskWyjdz.setOutlineThickness(3.f);

    sf::Text tekstWyjdz(font, "WYJDZ", 32);
    tekstWyjdz.setFillColor(sf::Color::White);
    tekstWyjdz.setPosition({890.f, 645.f});

    std::vector<Zawodnik*> sklad;
    std::vector<PrzyciskTreningu> przyciskiTreningu;
    int pulaTreningowa = 0;

    sf::Text naglowekTreningu(font, "TRENING - rozdaj punkty", 28);
    naglowekTreningu.setFillColor(sf::Color(255, 200, 60));
    naglowekTreningu.setPosition({180.f, 15.f});

    sf::RectangleShape przyciskRozegrajSezon({220.f, 55.f});
    przyciskRozegrajSezon.setPosition({290.f, 630.f});
    przyciskRozegrajSezon.setFillColor(sf::Color(35, 140, 60));
    przyciskRozegrajSezon.setOutlineColor(sf::Color(80, 200, 110));
    przyciskRozegrajSezon.setOutlineThickness(2.f);

    sf::Text tekstRozegrajSezon(font, "ROZEGRAJ SEZON", 20);
    tekstRozegrajSezon.setFillColor(sf::Color::White);
    tekstRozegrajSezon.setPosition({305.f, 648.f});

    float scrollOffset = 0.f;

    Ekran aktualnyEkran = Ekran::MENU_GLOWNE;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>()){
                if (aktualnyEkran == Ekran::WYNIKI) {
                    scrollOffset -= scrollEvent->delta * 30.f;
                    if (scrollOffset < 0.f) scrollOffset = 0.f;
                }
            }

            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f pozycjaMyszy(static_cast<float>(mouseEvent->position.x),
                                               static_cast<float>(mouseEvent->position.y));

                    if (aktualnyEkran == Ekran::MENU_GLOWNE) {
                        if (przyciskStart.getGlobalBounds().contains(pozycjaMyszy)) {
                            aktualnyEkran = Ekran::WYBOR_SKLADU;
                        }
                        if (przyciskWyjdz.getGlobalBounds().contains(pozycjaMyszy)) {
                            window.close();
                        }
                    } else if (aktualnyEkran == Ekran::WYBOR_SKLADU) {
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
                                sklad.clear();
                                for (size_t i = 0; i < przyciski.size(); i++) {
                                    if (przyciski[i].wybrany) sklad.push_back(&kadra[i]);
                                }

                                pulaTreningowa = 10;
                                przyciskiTreningu.clear();

                                float yTr = 80.f;
                                for (auto* zawodnik : sklad) {
                                    for (int t = 0; t < 3; t++) {
                                        PrzyciskTreningu pt;
                                        pt.zawodnik = zawodnik;
                                        pt.typ = static_cast<StatTyp>(t);
                                        pt.prostokat.setSize({50.f, 50.f});
                                        pt.prostokat.setPosition({500.f + t * 70.f, yTr});
                                        pt.prostokat.setFillColor(sf::Color(35, 140, 60));
                                        pt.prostokat.setOutlineColor(sf::Color::White);
                                        pt.prostokat.setOutlineThickness(2.f);
                                        przyciskiTreningu.push_back(pt);
                                    }
                                    yTr += 70.f;
                                }

                                aktualnyEkran = Ekran::TRENING;
                            } else {
                                komunikatBledu.setString("Wybierz dokladnie 4 zawodnikow!");
                            }
                        }
                    } else if (aktualnyEkran == Ekran::TRENING) {
                        if (pulaTreningowa > 0) {
                            for (auto& pt : przyciskiTreningu) {
                                if (pt.prostokat.getGlobalBounds().contains(pozycjaMyszy)) {
                                    pt.dodaj();
                                    pulaTreningowa--;
                                    break;
                                }
                            }
                        }

                        if (przyciskRozegrajSezon.getGlobalBounds().contains(pozycjaMyszy)) {
                            std::vector<Skocznia> kalendarz = {
                                Skocznia("Wisla (Malinka)", 134, 1.8),
                                Skocznia("Kuusamo (Ruka)", 142, 1.8),
                                Skocznia("Klingenthal", 140, 1.8),
                                Skocznia("Lillehammer", 140, 1.8),
                                Skocznia("Engelberg", 140, 1.8),
                                Skocznia("Oberstdorf", 137, 1.8),
                                Skocznia("Garmisch-Partenkirchen", 142, 1.8),
                                Skocznia("Innsbruck (Bergisel)", 128, 1.8),
                                Skocznia("Bischofshofen", 142, 1.8),
                                Skocznia("Wielka Krokiew", 140, 1.8),
                                Skocznia("Willingen (Muhlenkopfschanze)", 147, 1.8),
                                Skocznia("Lahti", 130, 1.8),
                                Skocznia("Oslo (Holmenkollen)", 134, 1.8),
                                Skocznia("Planica (Letalnica)", 240, 1.2)
                            };

                            wynikiDoRysowania.clear();
                            for (const auto& skocznia : kalendarz) {
                                wynikiDoRysowania.push_back(rozegrajKonkurs(sklad, skocznia));
                            }
                            wynikiDoRysowania.push_back(klasyfikacjaGeneralnaDoRysowania(sklad));

                            zapiszKadre(kadra);
                            scrollOffset = 0.f;
                            aktualnyEkran = Ekran::WYNIKI;
                        }
                    } else if (aktualnyEkran == Ekran::WYNIKI) {
                        if (przyciskZakoncz.getGlobalBounds().contains(pozycjaMyszy)) {
                            window.close();
                        }
                        if (przyciskNastepnySezon.getGlobalBounds().contains(pozycjaMyszy)) {
                            for (auto& z : kadra) z.punktyGeneralne = 0;
                            for (auto&p : przyciski) p.wybrany = false;
                            scrollOffset = 0.f;
                            aktualnyEkran = Ekran::WYBOR_SKLADU;
                        }
                    }
                }
            }
        }

        for (auto& p : przyciski) {
            p.prostokat.setFillColor(p.wybrany ? sf::Color(30, 150, 30) : sf::Color(50, 60, 80));
        }

        window.clear(sf::Color(20, 30, 50));

        if (aktualnyEkran == Ekran::MENU_GLOWNE) {
            window.draw(tloGradient);
            window.draw(gora);
            window.draw(tytulGry);
            window.draw(podtytulGry);
            window.draw(liniaOzdobna);
            window.draw(cienStart);
            window.draw(przyciskStart);
            window.draw(tekstStart);
            window.draw(cienWyjdz);
            window.draw(przyciskWyjdz);
            window.draw(tekstWyjdz);
        } else if (aktualnyEkran == Ekran::WYBOR_SKLADU) {
            window.draw(naglowek);
            for (auto& p : przyciski) {
                window.draw(p.prostokat);
                window.draw(p.tekst);
                window.draw(p.statystyki);
            }
            window.draw(przyciskZatwierdz);
            window.draw(tekstZatwierdz);
            window.draw(komunikatBledu);
        } else if (aktualnyEkran == Ekran::TRENING) {
            window.draw(naglowekTreningu);

            std::stringstream pulaStr;
            pulaStr << "Pozostale punkty: " << pulaTreningowa;
            sf::Text tekstPula(font, pulaStr.str(), 22);
            tekstPula.setFillColor(sf::Color::White);
            tekstPula.setPosition({30.f, 55.f});
            window.draw(tekstPula);

            Zawodnik* poprzedni = nullptr;
            for (auto& pt : przyciskiTreningu) {
                if (pt.zawodnik != poprzedni) {
                    sf::Text nazwaZawodnika(font, pt.zawodnik->imie, 20);
                    nazwaZawodnika.setFillColor(sf::Color(200, 200, 220));
                    nazwaZawodnika.setPosition({30.f, pt.prostokat.getPosition().y + 15.f});
                    window.draw(nazwaZawodnika);
                    poprzedni = pt.zawodnik;
                }

                window.draw(pt.prostokat);

                std::stringstream etykieta;
                etykieta << pt.nazwaStatu() << "\n" << pt.wartosc();
                sf::Text tekstPrzycisku(font, etykieta.str(), 13);
                tekstPrzycisku.setFillColor(sf::Color::White);
                tekstPrzycisku.setPosition({pt.prostokat.getPosition().x + 5.f, pt.prostokat.getPosition().y + 3.f});
                window.draw(tekstPrzycisku);
            }

            window.draw(przyciskRozegrajSezon);
            window.draw(tekstRozegrajSezon);
        } else if (aktualnyEkran == Ekran::WYNIKI) {
            float y = 20.f - scrollOffset;
            for (auto& konkurs : wynikiDoRysowania) {
                sf::Text tytul(font, konkurs.tytul, 22);
                bool jestKlasyfikacja = (konkurs.tytul == "KLASYFIKACJA GENERALNA");
                tytul.setFillColor(jestKlasyfikacja ? sf::Color(255, 200, 60) : sf::Color(140, 180, 230));
                tytul.setPosition({30.f, y});
                window.draw(tytul);
                y += 32.f;

                for (auto& wiersz : konkurs.wiersze) {
                    sf::Text t(font, wiersz.tresc, 17);
                    t.setFillColor(wiersz.kolor);
                    t.setPosition({50.f, y});
                    window.draw(t);
                    y += 24.f;
                }
                y += 16.f;
            }

            window.draw(przyciskZakoncz);
            window.draw(tekstZakoncz);
            window.draw(przyciskNastepnySezon);
            window.draw(tekstNastepnySezon);
        }

        window.display();
    }

    return 0;
}