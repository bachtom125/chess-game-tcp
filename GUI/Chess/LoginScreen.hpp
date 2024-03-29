#pragma once

#include <SFML/Graphics.hpp>
#include "TcpClient.hpp"
struct User
{
    std::string username;
    std::string password;
    int elo;
};
class LoginScreen
{

public:
    LoginScreen(sf::RenderWindow& window,TcpClient& tcpClient);
    bool isLoginSuccessful = false;
    User user;

    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    void handleLoginResponse(json data);
    bool startLogin = false;

private:
    sf::RenderWindow& window;
    TcpClient& tcpClient; // TcpClient member variable

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
    bool isRequestSent = false;

    void handleTextInput(sf::Text& text, const sf::Event& event);
    void validateLogin();
    void displayErrorMessage(const std::string& message);
};