#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class ResultScreen
{
public:
    ResultScreen(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    std::string winner;
    std::string loser;
    int winnerElo;
    int loserElo;
    std::string gameLog;
    bool backToMainMenu = false;

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text winnerText;
    sf::Text loserText;
    sf::Text gameLogText;
    sf::RectangleShape backButton;
    sf::Text backButtonText;
};