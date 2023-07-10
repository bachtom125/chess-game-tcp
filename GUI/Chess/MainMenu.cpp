#include "MainMenu.hpp"
#include <iostream>

MainMenu::MainMenu(sf::RenderWindow& window)
    : window(window),
    chessBoardScreen(window)
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

    challengerText.setFont(font);
    challengerText.setCharacterSize(20);
    challengerText.setString("1. Challenger");
    challengerText.setPosition(300, 200);

    randomMatchText.setFont(font);
    randomMatchText.setCharacterSize(20);
    randomMatchText.setString("2. Random Match");
    randomMatchText.setPosition(300, 250);

    onlineUsersText.setFont(font);
    onlineUsersText.setCharacterSize(20);
    onlineUsersText.setString("3. Online Users");
    onlineUsersText.setPosition(300, 300);

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

    if (isMouseOver(challengerText)) {
        challengerText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_Challenger;
            std::cout << "Selected option: Challenger" << std::endl;
        }
    }
    else {
        challengerText.setFillColor(defaultTextColor);
    }

    if (isMouseOver(randomMatchText)) {
        randomMatchText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_RandomMatch;
            std::cout << "Selected option: Random Match" << std::endl;
        }
    }
    else {
        randomMatchText.setFillColor(defaultTextColor);
    }

    if (isMouseOver(onlineUsersText)) {
        onlineUsersText.setFillColor(hoverTextColor);
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            currentOption = Option_OnlineUsers;
            std::cout << "Selected option: Online Users" << std::endl;
        }
    }
    else {
        onlineUsersText.setFillColor(defaultTextColor);
    }

    chessBoardScreen.handleEvent(event);
}

void MainMenu::update()
{
    if (currentOption == Option_Challenger) {
        // Perform actions specific to the "Challenger" option
    }
    else if (currentOption == Option_RandomMatch) {
        // Perform actions specific to the "Random Match" option
    }
    else if (currentOption == Option_OnlineUsers) {
        // Perform actions specific to the "Online Users" option
    }

    chessBoardScreen.update();
}

void MainMenu::draw()
{
    window.clear();
    window.draw(backgroundSprite);

    titleText.setFillColor(defaultTextColor);
    window.draw(titleText);

    window.draw(challengerText);
    window.draw(randomMatchText);
    window.draw(onlineUsersText);

    chessBoardScreen.draw();

}

bool MainMenu::isMouseOver(const sf::Text& text)
{
    sf::FloatRect bounds = text.getGlobalBounds();
    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    return bounds.contains(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
}