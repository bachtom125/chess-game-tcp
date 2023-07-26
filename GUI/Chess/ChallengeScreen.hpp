#pragma once

#include <SFML/Graphics.hpp>
#include "TcpClient.hpp"

class ChallengeScreen
{
public:
    ChallengeScreen(sf::RenderWindow& window, TcpClient& tcpClient);

    void handleEvent(const sf::Event& event);
    void draw();

    bool isChallengeVisible() const { return challengeVisible; }
    bool isAccepted() const { return accepted; }

    void showChallenge(const std::string& challengerName);
    void hideChallenge();

private:
    sf::RenderWindow& window;
    TcpClient& tcpClient;
    sf::Font font;
    sf::Text challengeText;
    sf::Text acceptText;
    sf::Text declineText;

    sf::RectangleShape acceptButton;
    sf::RectangleShape declineButton;

    bool challengeVisible = false;
    bool accepted = false;
};

