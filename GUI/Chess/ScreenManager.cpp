#include "ScreenManager.hpp"
#include <iostream>
#include <fstream>
#include <Windows.h> 

ScreenManager::ScreenManager(sf::RenderWindow& window, TcpClient& tcpClient)
    : window(window),
    tcpClient(tcpClient),
    loginScreen(window, tcpClient),
    mainMenu(window, tcpClient),
    chessBoardScreen(window, tcpClient),
    currentScreen(Screen::Login),
    resultScreen(window),
    onlineUserListScreen(window),
    challengeScreen(window, tcpClient)
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

        else if (currentScreen == Screen::ResultScreen) {
            resultScreen.handleEvent(event);
        }

        else if (currentScreen == Screen::OnlineUserListScreen) {
            onlineUserListScreen.handleEvent(event);
        }
        else if (currentScreen == Screen::ChallengeScreen) {
            challengeScreen.handleEvent(event);
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
        else if (mainMenu.activeScreen == Screen::OnlineUserListScreen) {
            currentScreen = Screen::OnlineUserListScreen;
        }
    }
    else if (currentScreen == Screen::ChessBoardScreen) {
        if (mainMenu.currentOption == MainMenuOption::Option_Challenge) {
            chessBoardScreen.isChallengeMode = true;
        }
        chessBoardScreen.update();
    }

    else if (currentScreen == Screen::ResultScreen) {
        resultScreen.update();
        if (resultScreen.backToMainMenu) {
            currentScreen = Screen::MainMenu;
            mainMenu.currentOption = MainMenuOption::Option_MainMenu;
            mainMenu.activeScreen = Screen::MainMenu;
            resultScreen.backToMainMenu = false;
        }
    }

    else if (currentScreen == Screen::OnlineUserListScreen) {
        onlineUserListScreen.update();
        if (onlineUserListScreen.isBack) {
            currentScreen = Screen::MainMenu;
            mainMenu.currentOption = MainMenuOption::Option_MainMenu;
            mainMenu.activeScreen = Screen::MainMenu;
            onlineUserListScreen.isBack = false;
        }
        if (onlineUserListScreen.selectedUsername != "") {
            currentScreen = Screen::ChessBoardScreen;
            chessBoardScreen.isChallengeMode = true;
            chessBoardScreen.opponent = onlineUserListScreen.selectedUsername;
        }
    }
    else if (currentScreen == Screen::ChallengeScreen) {
        if(!challengeScreen.isChallengeVisible()) {
            currentScreen = Screen::MainMenu;
            mainMenu.currentOption = MainMenuOption::Option_MainMenu;
            mainMenu.activeScreen = Screen::MainMenu;
        }
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
    else if (currentScreen == Screen::ResultScreen){
        resultScreen.draw();
        window.display();
    }

    else if (currentScreen == Screen::OnlineUserListScreen) {
        onlineUserListScreen.draw();
        window.display();
    }
    else if (currentScreen == Screen::ChallengeScreen) {
        challengeScreen.draw();
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
                
                bool found = chessBoardScreen.handleMatchMakingResponse(response);
                // Handle other response types if needed
                if (found) currentScreen = Screen::ChessBoardScreen;
            }
            else if (responseType == RespondType::Login) {
                loginScreen.handleLoginResponse(response);
            }
            else if (responseType == RespondType::GameResult) {
                currentScreen = Screen::ResultScreen;
                chessBoardScreen.init();
                mainMenu.currentOption = Option_MainMenu;
                resultScreen.gameLog = response["data"]["log"].get<std::string>();
                resultScreen.winner = response["data"]["winner"]["username"].get<std::string>();
                resultScreen.winnerElo = response["data"]["winner"]["elo"].get<int>();
                resultScreen.loser = response["data"]["loser"]["username"].get<std::string>();
                resultScreen.loserElo = response["data"]["loser"]["elo"].get<int>();
                if (resultScreen.winner == mainMenu.user.username) {
                    mainMenu.user.elo = resultScreen.winnerElo;
                }
                else {
                    mainMenu.user.elo = resultScreen.loserElo;
                }

                writeStringToFile(response["data"]["matchId"].get<std::string>(), response["data"]["log"].get<std::string>());

            }
            else if (responseType == RespondType::OnlinePlayersList) {
                onlineUserListScreen.receiveUserListData(response["data"]);
            }
            else if (responseType == RespondType::Challenge) {
                challengeScreen.showChallenge(response["data"]["challenger"].get<std::string>());
                currentScreen = Screen::ChallengeScreen;
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

bool ScreenManager::writeStringToFile(const std::string& filename, const std::string& content)
{
    std::string folderPath = "gamelog";

    // Create the folder if it doesn't exist
    if (!CreateDirectoryA(folderPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        std::cerr << "Error creating folder: " << folderPath << std::endl;
        return false;
    }

    std::string filePath = folderPath + "\\" + filename + ".txt";
    std::ofstream outputFile(filePath);

    if (!outputFile)
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return false;
    }

    outputFile << content;

    if (!outputFile)
    {
        std::cerr << "Error writing to file: " << filePath << std::endl;
        return false;
    }

    return true;
}