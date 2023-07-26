#pragma once

#include <SFML/Graphics.hpp>
#include "LoginScreen.hpp"
#include "MainMenu.hpp"
#include "TcpClient.hpp"
#include "ChessBoardScreen.hpp"
#include "ResultScreen.hpp"
#include "OnlineUserListScreen.hpp"
#include "ChallengeScreen.hpp"
#include <thread>
#include <atomic>


enum class Screen
{
    Login,
    MainMenu,
    ChessBoardScreen,
    ResultScreen,
    OnlineUserListScreen,
    ChallengeScreen
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
    std::atomic<bool> keepListening;

    User user;
    sf::RenderWindow& window;
    TcpClient& tcpClient;
    LoginScreen loginScreen;
    MainMenu mainMenu;
    ChessBoardScreen chessBoardScreen;
    ResultScreen resultScreen;
    OnlineUserListScreen onlineUserListScreen;
    ChallengeScreen challengeScreen;
    Screen currentScreen;
    std::thread serverResponseThread;

    void handleServerResponses();

    void startListeningToServerResponses();
    void stopListeningToServerResponses();
    bool writeStringToFile(const std::string& filename, const std::string& content);
    void getBackToMainMenuScreen();
};