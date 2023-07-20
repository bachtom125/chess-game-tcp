#pragma once

#include <SFML/Graphics.hpp>
#include "LoginScreen.hpp"
#include "MainMenu.hpp"
#include "TcpClient.hpp"
#include "ChessBoardScreen.hpp"


enum class Screen
{
    Login,
    MainMenu,
    ChessBoardScreen
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
    User user;
    sf::RenderWindow& window;
    TcpClient& tcpClient;
    LoginScreen loginScreen;
    MainMenu mainMenu;
    ChessBoardScreen chessBoardScreen;
    Screen currentScreen;
};