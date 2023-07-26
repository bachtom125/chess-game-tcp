#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <vector>

struct UserItem
{
    std::string username;
    int elo;
};

class OnlineUserListScreen
{
public:
    OnlineUserListScreen(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    void receiveUserListData(const nlohmann::json& userListData);

    bool isBack = false;
    std::string selectedUsername = "";

private:
    void scrollUp();
    void scrollDown();
    void updateChallengeButtonVisibility();

    sf::RenderWindow& window;

    sf::Font font;
    sf::Text usernameText;
    sf::Text eloText;
    sf::Text challengeText;
    sf::Text backText;
    sf::Text selectedItemText;

    sf::RectangleShape usernameBox;
    sf::RectangleShape eloBox;
    sf::RectangleShape challengeButton;
    sf::RectangleShape backButton;
    sf::RectangleShape selectedItemBackground;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    bool challengeButtonVisible = false;
    int selectedItemIndex = -1;

    std::vector<UserItem> userList; // Dummy data for users

    void handleTextInput(sf::Text& text, const sf::Event& event);
};