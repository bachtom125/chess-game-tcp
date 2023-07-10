#pragma once

#include <SFML/Graphics.hpp>
#include "LoginScreen.hpp"
#include "MainMenu.hpp"
#include "TcpClient.hpp"


enum class Screen
{
    Login,
    MainMenu
};

class ScreenManager
{
public:
    ScreenManager(sf::RenderWindow& window, TcpClient& tcpClient);

    void run();

private:
    void handleEvents();
    void update();
    void draw();

    sf::RenderWindow& window;
    TcpClient& tcpClient;
    LoginScreen loginScreen;
    MainMenu mainMenu;
    Screen currentScreen;
};