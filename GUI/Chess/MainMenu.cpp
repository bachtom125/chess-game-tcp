#include "MainMenu.hpp"
#include <iostream>
#include "ScreenManager.hpp"
MainMenu::MainMenu(sf::RenderWindow& window, TcpClient& tcpClient)
    : window(window),tcpClient(tcpClient)
{
    if (!font.loadFromFile("fonts/inter.ttf")) {
        // Handle font loading error
        // Print an error message, throw an exception, or take appropriate action
        std::cerr << "Error loading font" << std::endl;
    }

    if (!backgroundTexture.loadFromFile("images/login-bg.jpg")) {
        // Handle background image loading error
        // Print an error message, throw an exception, or take appropriate action
        std::cerr << "Error loading background image" << std::endl;
    }

    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
    );

    titleText.setFont(font);
    titleText.setCharacterSize(50);
    titleText.setFillColor(defaultTextColor);
    titleText.setString("Chess Game");
    titleText.setPosition(300, 100);

    matchmakingText.setFont(font);
    matchmakingText.setCharacterSize(20);
    matchmakingText.setString("1. Matchmaking");
    matchmakingText.setPosition(300, 200);

    challengeText.setFont(font);
    challengeText.setCharacterSize(20);
    challengeText.setString("2. Challenge");
    challengeText.setPosition(300, 250);

    exitText.setFont(font);
    exitText.setCharacterSize(20);
    exitText.setString("3. Exit");
    exitText.setPosition(300, 300);

    defaultTextColor = sf::Color::White;
    hoverTextColor = sf::Color::Red;
    currentOptionColor = sf::Color::Blue;
}

void MainMenu::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Closed) {
        std::cout << "close" << std::endl;
        window.close();
    }

    if (isMouseOver(matchmakingText)) {
        matchmakingText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_Matchmaking;
            std::cout << "Selected option: matchmakingText" << std::endl;
        }
    }
    else {
        matchmakingText.setFillColor(defaultTextColor);
    }

    if (isMouseOver(challengeText)) {
        challengeText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_Challenge;
            std::cout << "Selected option: challengeText" << std::endl;
            json requestData;
            tcpClient.sendRequest(RequestType::GetOnlinePlayersList, requestData);
        }
    }
    else {
        challengeText.setFillColor(defaultTextColor);
    }

    if (isMouseOver(exitText)) {
        exitText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_Exit;
            std::cout << "Selected option: exitText" << std::endl;
        }
    }
    else {
        exitText.setFillColor(defaultTextColor);
    }

}

void MainMenu::update()
{
    if (currentOption == Option_Matchmaking) {
        // Perform actions specific to the "Challenger" option
        activeScreen = Screen::ChessBoardScreen;
    }
    else if (currentOption == Option_Challenge) {
        // Perform actions specific to the "Random Match" option
        activeScreen = Screen::OnlineUserListScreen;
    }
    else if (currentOption == Option_Exit) {
        // Perform actions specific to the "Online Users" option
    }

}

void MainMenu::draw()
{
    window.clear();
    window.draw(backgroundSprite);

    titleText.setFillColor(defaultTextColor);
    window.draw(titleText);

    window.draw(matchmakingText);
    window.draw(challengeText);
    window.draw(exitText);

    // Display the user information
    if (user.username != "")
    {
        userInfoText.setFont(font);
        userInfoText.setCharacterSize(24);
        userInfoText.setFillColor(defaultTextColor);
        userInfoText.setString(user.username + " - " + std::to_string(user.elo));
        userInfoText.setPosition(50, 50); // Adjust the position as needed
        window.draw(userInfoText);
    }


}

bool MainMenu::isMouseOver(const sf::Text& text)
{
    sf::FloatRect bounds = text.getGlobalBounds();
    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    return bounds.contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
}