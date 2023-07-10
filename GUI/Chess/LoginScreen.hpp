#pragma once

#include <SFML/Graphics.hpp>

class LoginScreen
{

public:
    LoginScreen(sf::RenderWindow& window);
    bool isLoginSuccessful = false;

    void handleEvent(const sf::Event& event);
    void update();
    void draw();

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text usernameText;
    sf::Text passwordText;
    sf::Text signupText;

    sf::RectangleShape usernameBox;
    sf::RectangleShape passwordBox;
    sf::RectangleShape signupButton;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    bool usernameActive = false;
    bool passwordActive = false;

    void handleTextInput(sf::Text& text, const sf::Event& event);
    bool validateLogin(const std::string& username, const std::string& password);
    void displayErrorMessage(const std::string& message);
};