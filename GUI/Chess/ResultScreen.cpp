#include "ResultScreen.hpp"
#include <iostream>
ResultScreen::ResultScreen(sf::RenderWindow& window)
    : window(window)
{
    if (!font.loadFromFile("fonts/inter.ttf")) {
        // Handle font loading error
        // Print an error message, throw an exception, or take appropriate action
        std::cout << "Error loading font" << std::endl;
        return;
    }


    winnerText.setFont(font);
    winnerText.setCharacterSize(30);
    winnerText.setPosition(100, 100);
    winnerText.setFillColor(sf::Color::White);

    loserText.setFont(font);
    loserText.setCharacterSize(30);
    loserText.setPosition(100, 150);
    loserText.setFillColor(sf::Color::White);


    gameLogText.setFont(font);
    gameLogText.setCharacterSize(20);
    gameLogText.setPosition(100, 200);
    gameLogText.setFillColor(sf::Color::White);


    backButton.setSize(sf::Vector2f(200, 50));
    backButton.setPosition(100, 500);
    backButton.setFillColor(sf::Color::White);


    backButtonText.setFont(font);
    backButtonText.setString("Back to Main Menu");
    backButtonText.setCharacterSize(20);
    backButtonText.setFillColor(sf::Color::Black);

    backButtonText.setPosition(120, 510);
}

void ResultScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
            {
                // Go back to the main menu
                // Implement your logic here to switch screens or return to the main menu
                backToMainMenu = true;
            }
        }
    }
}

void ResultScreen::update() {
    winnerText.setString("Winner: " + winner + " (ELO: " + std::to_string(winnerElo) + ")");
    loserText.setString("Loser: " + loser + " (ELO: " + std::to_string(loserElo) + ")");
    gameLogText.setString("Game Log:\n" + gameLog);


}

void ResultScreen::draw()
{
    window.clear(sf::Color::Black);
    window.draw(winnerText);
    window.draw(loserText);
    window.draw(gameLogText);
    window.draw(backButton);
    window.draw(backButtonText);
}