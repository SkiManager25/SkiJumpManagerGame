#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

struct PrzyciskZawodnika {
    std::string imie;
    sf::RectangleShape prostokat;
    sf::Text tekst;
    bool wybrany = false;

    PrzyciskZawodnika(const std::string& im, const sf::Font& font)
        : imie(im), tekst(font, im, 24) {}
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Ski Jumping Manager - Wybor skladu");

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        return -1;
    }

    std::vector<std::string> imionaKadry = {
        "Kamil Stoch", "Dawid Kubacki", "Piotr Zyla",
        "Aleksander Zniszczol", "Pawel Wasek", "Jakub Wolny"
    };

    const int MAX_WYBOR = 4;
    const float SZEROKOSC_PRZYCISKU = 400.f;
    const float WYSOKOSC_PRZYCISKU = 50.f;
    const float ODSTEP = 10.f;
    const float START_Y = 50.f;

    std::vector<PrzyciskZawodnika> przyciski;

    for (size_t i = 0; i < imionaKadry.size(); i++) {
        przyciski.emplace_back(imionaKadry[i], font);

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
    naglowek.setPosition({230.f, 10.f});

    std::vector<std::string> finalnySklad;
    bool skladZatwierdzony = false;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f pozycjaMyszy(static_cast<float>(mouseEvent->position.x),
                                               static_cast<float>(mouseEvent->position.y));

                    int iluWybranych = 0;
                    for (auto& p : przyciski) {
                        if (p.wybrany) iluWybranych++;
                    }

                    for (auto& p : przyciski) {
                        if (p.prostokat.getGlobalBounds().contains(pozycjaMyszy)) {
                            if (p.wybrany) {
                                p.wybrany = false;
                            } else if (iluWybranych < MAX_WYBOR) {
                                p.wybrany = true;
                            }
                        }
                    }

                    if (przyciskZatwierdz.getGlobalBounds().contains(pozycjaMyszy)) {
                        int policz = 0;
                        for (auto& p : przyciski) if (p.wybrany) policz++;

                        if (policz == MAX_WYBOR) {
                            finalnySklad.clear();
                            for (auto& p : przyciski) {
                                if (p.wybrany) finalnySklad.push_back(p.imie);
                            }
                            skladZatwierdzony = true;
                            window.close();
                        }
                    }
                }
            }
        }

        for (auto& p : przyciski) {
            if (p.wybrany) {
                p.prostokat.setFillColor(sf::Color(30, 150, 30));
            } else {
                p.prostokat.setFillColor(sf::Color(50, 60, 80));
            }
        }

        window.clear(sf::Color(20, 30, 50));
        window.draw(naglowek);
        for (auto& p : przyciski) {
            window.draw(p.prostokat);
            window.draw(p.tekst);
        }
        window.draw(przyciskZatwierdz);
        window.draw(tekstZatwierdz);
        window.display();
    }

    if (skladZatwierdzony) {
        std::cout << "Wybrany sklad:\n";
        for (auto& imie : finalnySklad) {
            std::cout << "- " << imie << "\n";
        }
    }

    return 0;
}