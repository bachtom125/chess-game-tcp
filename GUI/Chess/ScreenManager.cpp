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
    startListeningToServerResponses();
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

void ScreenManager::handleServerResponses()
{
    while (keepListening)
    {
        std::string receivedData;
        if (tcpClient.receive(receivedData))
        {
            std::cout << "receivedData thread: " << receivedData << std::endl;

            // Parse the received data and handle based on the response type
            json response = json::parse(receivedData);
            RespondType responseType = response["type"];

            if (responseType == RespondType::Move)
            {
                // Process the move response from the server
                chessBoardScreen.receiveGameStateResponse(response["data"].dump());
            }
            else if (responseType == RespondType::MatchMaking)
            {
                
                chessBoardScreen.handleMatchMakingResponse(response);
                // Handle other response types if needed
            }
            else if (responseType == RespondType::Login) {
                loginScreen.handleLoginResponse(response);
            }
         }
    }
}

void ScreenManager::startListeningToServerResponses()
{
    // Start the server response thread to handle incoming responses
    keepListening = true;
    serverResponseThread = std::thread(&ScreenManager::handleServerResponses, this);
}

void ScreenManager::stopListeningToServerResponses()
{
    // Stop the server response thread
    keepListening = false;
    if (serverResponseThread.joinable())
    {
        serverResponseThread.join();
    }
}