#pragma once

#include <SFML/Graphics.hpp>
#include "LoginScreen.hpp"
enum class Screen;

enum MainMenuOption
{
    Option_MainMenu,
    Option_Matchmaking,
    Option_Challenge,
    Option_Exit
};

class MainMenu
{
public:
    MainMenu(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    Screen activeScreen;
    User user; // Store the user pointer to access user information


private:
    sf::RenderWindow& window;
    MainMenuOption currentOption = MainMenuOption::Option_MainMenu;
    // Add a new sf::Text object to display user information
    sf::Text userInfoText;

    sf::Font font;
    sf::Text titleText;
    sf::Text matchmakingText;
    sf::Text challengeText;
    sf::Text exitText;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    sf::Color defaultTextColor;
    sf::Color hoverTextColor;
    sf::Color currentOptionColor;

    bool isMouseOver(const sf::Text& text);
};