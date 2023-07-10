#include "ScreenManager.hpp"
#include <iostream>

ScreenManager::ScreenManager(sf::RenderWindow& window)
    : window(window),
    loginScreen(window),
    mainMenu(window),
    currentScreen(Screen::Login)
{
}

void ScreenManager::run()
{
    std::cout << "Run" << std::endl;

    while (window.isOpen())
    {
        handleEvents();
        update();
        draw();
    }
}

void ScreenManager::handleEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();

        if (currentScreen == Screen::Login)
        {
            loginScreen.handleEvent(event);
            if (loginScreen.isLoginSuccessful)
            {
                currentScreen = Screen::MainMenu;
                std::cout << "Login success" << std::endl;
            }
        }
        else if (currentScreen == Screen::MainMenu)
        {
            mainMenu.handleEvent(event);
        }
    }
}


void ScreenManager::update()
{
    if (currentScreen == Screen::Login)
    {
        loginScreen.update();
        if (loginScreen.isLoginSuccessful)
        {
            currentScreen = Screen::MainMenu;
        }
    }
    else if (currentScreen == Screen::MainMenu)
    {
        mainMenu.update();
    }
}

void ScreenManager::draw()
{
    window.clear();
    if (currentScreen == Screen::Login)
    {
        loginScreen.draw();
    }
    else if (currentScreen == Screen::MainMenu)
    {
        mainMenu.draw();
    }
    window.display();
}