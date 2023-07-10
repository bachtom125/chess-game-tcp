#pragma once

#include <SFML/Graphics.hpp>
#include "ChessBoardScreen.hpp"

enum MainMenuOption
{
    Option_Challenger,
    Option_RandomMatch,
    Option_OnlineUsers
};

class MainMenu
{
public:
    MainMenu(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event);
    void update();
    void draw();

private:
    sf::RenderWindow& window;
    ChessBoardScreen chessBoardScreen;
    MainMenuOption currentOption = Option_Challenger;

    sf::Font font;
    sf::Text titleText;
    sf::Text challengerText;
    sf::Text randomMatchText;
    sf::Text onlineUsersText;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Color defaultTextColor;
    sf::Color hoverTextColor;
    sf::Color currentOptionColor;

    bool isMouseOver(const sf::Text& text);
};