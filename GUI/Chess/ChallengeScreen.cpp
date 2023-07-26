#include "ChallengeScreen.hpp"

ChallengeScreen::ChallengeScreen(sf::RenderWindow& window,TcpClient& tcpClient)
    : window(window), tcpClient(tcpClient)
{
    if (!font.loadFromFile("fonts/inter.ttf")) {
        // Handle font loading error
        // Print an error message, throw an exception, or take appropriate action
        // ...
    }

    challengeText.setFont(font);
    challengeText.setCharacterSize(16);
    challengeText.setFillColor(sf::Color::White);
    challengeText.setPosition(200, 200);

    acceptText.setFont(font);
    acceptText.setCharacterSize(16);
    acceptText.setFillColor(sf::Color::Black);
    acceptText.setPosition(250, 250);
    acceptText.setString("Accept");

    declineText.setFont(font);
    declineText.setCharacterSize(16);
    declineText.setFillColor(sf::Color::Black);
    declineText.setPosition(400, 250);
    declineText.setString("Decline");

    acceptButton.setSize(sf::Vector2f(100, 30));
    acceptButton.setPosition(240, 240);
    acceptButton.setFillColor(sf::Color::White);

    declineButton.setSize(sf::Vector2f(100, 30));
    declineButton.setPosition(390, 240);
    declineButton.setFillColor(sf::Color::White);
}

void ChallengeScreen::handleEvent(const sf::Event& event)
{
    if (!challengeVisible) return;

    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        json requestData;

        if (acceptButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            // Handle Accept button logic here
            accepted = true;
            requestData["message"] = "accept";
            tcpClient.sendRequest(RequestType::Challenge, requestData);
            challengeVisible = false;
        }
        else if (declineButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            // Handle Decline button logic here
            accepted = false;
            requestData["message"] = "reject";
            tcpClient.sendRequest(RequestType::Challenge, requestData);
            challengeVisible = false;
        }
    }
}

void ChallengeScreen::draw()
{
    if (challengeVisible)
    {
        // Draw the challenge screen if it's visible
        window.clear(sf::Color(0, 0, 0, 150));
        window.draw(challengeText);
        window.draw(acceptButton);
        window.draw(declineButton);
        window.draw(acceptText);
        window.draw(declineText);
    }
    else
    {
        // Clear the screen if the challenge is not visible
        window.clear();
    }
}

void ChallengeScreen::showChallenge(const std::string& challengerName)
{
    // Show the challenge screen with the challenger's name
    challengeText.setString("You have been challenged by " + challengerName);
    challengeVisible = true;
    accepted = false;
}

void ChallengeScreen::hideChallenge()
{
    // Hide the challenge screen
    challengeVisible = false;
}