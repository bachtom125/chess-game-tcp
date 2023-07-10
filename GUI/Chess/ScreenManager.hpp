#pragma once

#include <SFML/Graphics.hpp>
#include "LoginScreen.hpp"
#include "MainMenu.hpp"

enum class Screen
{
    Login,
    MainMenu
};

class ScreenManager
{
public:
    ScreenManager(sf::RenderWindow& window);

    void run();

private:
    void handleEvents();
    void update();
    void draw();

    sf::RenderWindow& window;
    LoginScreen loginScreen;
    MainMenu mainMenu;
    Screen currentScreen;
};