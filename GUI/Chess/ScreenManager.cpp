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
                chessBoardScreen.user = loginScreen.user;
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
            chessBoardScreen.user = loginScreen.user;
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
                chessBoardScreen.receiveGameStateResponse(response);
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

void ChessBoardScreen::displayErrorMessage(const std::string& message)
{
    // Replace this with your actual implementation to display an error message
    // For example, you can use an sf::Text object to render the message on the screen
    sf::Text errorMessage;
    errorMessage.setFont(font);
    errorMessage.setCharacterSize(16);
    errorMessage.setFillColor(sf::Color::Red);
    errorMessage.setString(message);
    errorMessage.setPosition(300, 475);

    window.draw(errorMessage);
    window.display();

    // Optionally, you can add a delay to show the error message for a certain duration before clearing it
    sf::sleep(sf::seconds(2));
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