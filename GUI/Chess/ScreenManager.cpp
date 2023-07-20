#include "ScreenManager.hpp"
#include <iostream>

ScreenManager::ScreenManager(sf::RenderWindow& window, TcpClient& tcpClient)
    : window(window),
    tcpClient(tcpClient),
    loginScreen(window, tcpClient),
    mainMenu(window),
    chessBoardScreen(window, tcpClient),
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
                mainMenu.user = loginScreen.user;
                currentScreen = Screen::MainMenu;
                std::cout << "Login success" << std::endl;
            }
        }
        else if (currentScreen == Screen::MainMenu)
        {
            mainMenu.handleEvent(event);
        }
        else if (currentScreen == Screen::ChessBoardScreen) {
            chessBoardScreen.handleEvent(event);
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
            mainMenu.user = loginScreen.user;
            
            currentScreen = Screen::MainMenu;
            user = loginScreen.user;
        }
    }
    else if (currentScreen == Screen::MainMenu)
    {
        mainMenu.update();
        if(mainMenu.activeScreen == Screen::ChessBoardScreen) {
            currentScreen = Screen::ChessBoardScreen;
        }
    }
    else if (currentScreen == Screen::ChessBoardScreen) {
        chessBoardScreen.update();
    }
}

void ScreenManager::draw()
{
    window.clear();
    if (currentScreen == Screen::Login)
    {
        loginScreen.draw();
        window.display();
    }
    else if (currentScreen == Screen::MainMenu)
    {
        mainMenu.draw();
        window.display();
    }
    else if (currentScreen == Screen::ChessBoardScreen) {
        chessBoardScreen.draw();
        window.display();
    }

}