#pragma once

#include <SFML/Graphics.hpp>

enum class Screen;

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
    Screen activeScreen;


private:
    sf::RenderWindow& window;
    MainMenuOption currentOption;

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