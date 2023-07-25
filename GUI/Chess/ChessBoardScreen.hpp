#pragma once

#include <SFML/Graphics.hpp>
#include "TcpClient.hpp"
#include "LoginScreen.hpp"


using Fixed2DArray = int[8][8];


class ChessBoardScreen
{
public:
    ChessBoardScreen(sf::RenderWindow& window, TcpClient& tcpClient);

    void handleEvent(const sf::Event& event);
    void update();
    void draw();
    void receiveGameStateResponse(json response);
    void handleMatchMakingResponse(json data);
    bool startFindingMatchMaking = false;
    User user;

private:
    bool isMatchFound = false;
    bool isMoveAllowed = false;
    bool myTurn = true;
    std::string matchId;

    TcpClient& tcpClient; // TcpClient member variable

    void loadPosition();
    void move(std::string str);
    std::string toChessNote(sf::Vector2f p);
    sf::Vector2f toCoord(char a, char b);

    sf::RenderWindow& window;
    sf::Sprite f[32]; // figures
    std::string position = "";
    int board[8][8] = {
    {-1, -2, -3, -4, -5, -3, -2, -1},
    {-6, -6, -6, -6, -6, -6, -6, -6},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {6, 6, 6, 6, 6, 6, 6, 6},
    {1, 2, 3, 4, 5, 3, 2, 1}
    };
    int size = 56;
    sf::Vector2f offset{ 28, 28 };
    sf::Texture t1, t2;
    sf::Sprite sBoard;
    bool isMove = false;
    float dx = 0, dy = 0;
    sf::Vector2f oldPos, newPos;
    std::string str;
    int n = 0;
    bool firstMouseRelease = true;
    sf::Font font;
    sf::Text meText;
    sf::Text opponentText;

    void handleTextInput(sf::Text& text, const sf::Event& event);
    // Function to handle matchmaking request and response
    bool sendMatchmakingRequest();
    void processMatchmakingResponse(const std::string& response);

    // Add a flag to indicate if the server communication thread should continue running

    // Add a new thread for server communiction

    // New functions to handle receiving and processing game state response

    // New function to send the move to the server and receive the game state response
    void sendMoveToServer(const std::string& move);
    void convertBoardResponse(int[8][8], std::string);
    void displayErrorMessage(const std::string& message);
};