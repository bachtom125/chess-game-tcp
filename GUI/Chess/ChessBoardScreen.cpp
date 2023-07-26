#include <iostream>
#include <string>
#include "ChessBoardScreen.hpp"

ChessBoardScreen::ChessBoardScreen(sf::RenderWindow& window, TcpClient& tcpClient)
    : window(window), tcpClient(tcpClient)
{
    t1.loadFromFile("./images/figures.png");
    t2.loadFromFile("./images/board.png");
    sBoard.setTexture(t2);

    for (int i = 0; i < 32; i++)
        f[i].setTexture(t1);

    loadPosition(true);

    // Load the font
    if (!font.loadFromFile("fonts/inter.ttf")) {
        // Handle font loading error
        // Print an error message, throw an exception, or take appropriate action
        std::cout << "Error loading font" << std::endl;
        return;
    }

    meText.setFont(font);
    meText.setCharacterSize(20);
    meText.setPosition(window.getSize().x - 240, window.getSize().y - 160); // Adjust the position as needed
    meText.setFillColor(sf::Color::White);


    opponentText.setFont(font);
    opponentText.setCharacterSize(20);
    opponentText.setPosition(window.getSize().x - 240, 30 ); // Adjust the position as needed
    opponentText.setFillColor(sf::Color::White);
    opponentText.setString("Waiting for an opponent..."); // Update "me" with the actual player name

    resignButton.setSize(sf::Vector2f(200, 50));
    resignButton.setPosition(100, 550);
    resignButton.setFillColor(sf::Color::White);


    resignButtonText.setFont(font);
    resignButtonText.setString("Resign");
    resignButtonText.setCharacterSize(20);
    resignButtonText.setFillColor(sf::Color::Black);
    resignButtonText.setPosition(110, 560);

    // Challenge Button
    challengeButton.setSize(sf::Vector2f(200, 50));
    challengeButton.setPosition(100, 550);
    challengeButton.setFillColor(sf::Color::White);


    challengeButtonText.setFont(font);
    challengeButtonText.setString("Send Challenge");
    challengeButtonText.setCharacterSize(20);
    challengeButtonText.setFillColor(sf::Color::Black);
    challengeButtonText.setPosition(110, 560);
}

void ChessBoardScreen::init() {
    int initioalBoard[8][8] = {
    {-1, -2, -3, -4, -5, -3, -2, -1},
    {-6, -6, -6, -6, -6, -6, -6, -6},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {6, 6, 6, 6, 6, 6, 6, 6},
    {1, 2, 3, 4, 5, 3, 2, 1}
    };

    int i, j, k = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            board[i][j] = initioalBoard[i][j];
        }
    
    }
    opponentText.setString("Waiting for an opponent..."); // Update "me" with the actual player name
    isMatchFound = false;
    firstMouseRelease = true;
    startFindingMatchMaking = false;
    isInit = true;
    isUpdatingPosition = true;
    position = "";

}

void ChessBoardScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        std::cout << "close" << std::endl;
        window.close();
    }

    if (isChallengeMode && !isMatchFound)
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
                for (int i = 0; i < 32; i++)
                {
                    if (f[i].getGlobalBounds().contains(pos.x, pos.y))
                    {
                        isMove = true;
                        n = i;
                        dx = pos.x - f[i].getPosition().x;
                        dy = pos.y - f[i].getPosition().y;
                        oldPos = f[i].getPosition();

                    }
                }
                // Handle send challenge
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (challengeButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
                {
                    // Send Challenge Logic
                    sendChallengeRequest();
                }

            }
        }

        if (event.type == sf::Event::MouseButtonReleased && n >= 0)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                std::cout << "Start release (possible send move) " << std::endl;
                isMove = false;
                sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
                newPos = sf::Vector2f(size * int(pos.x / size), size * int(pos.y / size));

                BoardPosition newBoardPosition = toBoardPosition(newPos);
                BoardPosition oldBoardPosition = toBoardPosition(oldPos);
                int pieceValue = board[oldBoardPosition.row][oldBoardPosition.col];
                if (newBoardPosition.col < 0 || newBoardPosition.row < 0) return;

                str = toChessNote(oldPos) + toChessNote(newPos);
                std::string moveStr = toChessNote(oldPos) + " " + toChessNote(newPos);

                std::cout << "new position: " << toChessNote(newPos) << std::endl;

                if (oldPos != newPos)
                {
                    position += str + " ";
                    board[oldBoardPosition.row][oldBoardPosition.col] = 0;
                    board[newBoardPosition.row][newBoardPosition.col] = pieceValue;
                    move(str);
                }
                std::cout << "Position: " << position << std::endl;

                f[n].setPosition(newPos);
                n = -1;
            }
        }
    }

    if(isMatchFound && myTurn)
    {

        // Handle events specific to the chess board screen

        if (event.type == sf::Event::MouseButtonPressed && !firstMouseRelease)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
                for (int i = 0; i < 32; i++)
                {
                    if (f[i].getGlobalBounds().contains(pos.x, pos.y))
                    {
                        isMove = true;
                        n = i;
                        dx = pos.x - f[i].getPosition().x;
                        dy = pos.y - f[i].getPosition().y;
                        oldPos = f[i].getPosition();
                    }
                }
                // Handle resign
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (resignButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
                {
                    // Go back to the main menu
                    // Implement your logic here to switch screens or return to the main menu
                    json requestData;
                    requestData["move"] = "surrender";
                    tcpClient.sendRequest(RequestType::Move, requestData);
                }

            }
        }

        if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left && n>=0)
            {
                std::cout << "Start release (possible send move) " << std::endl;
                isMove = false;
                sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
                newPos = sf::Vector2f(size * int(pos.x / size), size * int(pos.y / size));
                str = toChessNote(oldPos) + toChessNote(newPos);
                std::string moveStr = toChessNote(oldPos) + " " + toChessNote(newPos);

                std::cout << "new position: " << toChessNote(newPos) << std::endl;

                if (oldPos != newPos)
                {
                    position += str + " ";
                    std::cout << "Start release (possible send move) " << std::endl;
                    sendMoveToServer(moveStr);
                }
                // f[n].setPosition(newPos);
            }

            firstMouseRelease = false;

        }
    }
}

void ChessBoardScreen::update()
{
    meText.setString("Me: " + user.username + " " + std::to_string(user.elo)); // Update "me" with the actual player name

    if (!isMatchFound && !startFindingMatchMaking && !isChallengeMode)
    {
        // Send a request to the server to check for match found
        // For example, assuming the request type is RequestType::CheckMatchFound
        json requestData; // Populate the JSON data for the request if needed
        if (tcpClient.sendRequest(RequestType::MatchMaking, requestData))
        {
            startFindingMatchMaking = true;
        }

    }
    if ((isMatchFound || isInit) && isUpdatingPosition) {
        loadPosition(true);
        isUpdatingPosition = false;
    }

}

void ChessBoardScreen::draw()
{
    ////// draw  ///////
    window.clear();
    window.draw(sBoard);
    for (int i = 0; i < 32; i++)
        f[i].move(offset);
    for (int i = 0; i < 32; i++)
        window.draw(f[i]);
    if(n >= 0)
        window.draw(f[n]);
    for (int i = 0; i < 32; i++)
     f[i].move(-offset);

    window.draw(meText);
    if(!isChallengeMode || !isMatchFound)
    {
        window.draw(opponentText);
    }

    if(isMatchFound)
    {
        window.draw(resignButton);
        window.draw(resignButtonText);
    }

    if (!isMatchFound && isChallengeMode) {
        window.draw(challengeButton);
        window.draw(challengeButtonText);
    }

}

void ChessBoardScreen::loadPosition(bool loadTexture)
{
    int k = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int n = board[i][j];
            if (!n)
                continue;
            int x = abs(n) - 1;
            int y = n > 0 ? 0 : 1;
            if(loadTexture) f[k].setTextureRect(sf::IntRect(size * x, size * y, size, size));
            f[k].setPosition(size * (isWhite ? j : (7 - j)), size * (isWhite ? (7 - i) : i));
            k++;
        }
    }

   /* for (int i = 0; i < position.length(); i += 5)
        move(position.substr(i, 4));*/
}

void ChessBoardScreen::move(std::string str)
{
    sf::Vector2f oldPos = toCoord(str[0], str[1]);
    sf::Vector2f newPos = toCoord(str[2], str[3]);

    for (int i = 0; i < 32; i++)
    {
        if (f[i].getPosition() == newPos)
            f[i].setPosition(-100, -100);
    }

    for (int i = 0; i < 32; i++)
    {
        if (f[i].getPosition() == oldPos)
            f[i].setPosition(newPos);
    }

}

BoardPosition ChessBoardScreen::toBoardPosition(sf::Vector2f p)
{
    int boardSize = 8; // Assuming the chessboard is an 8x8 grid
    int row = 7 - static_cast<int>(p.y / size);
    int col = static_cast<int>(p.x / size);

    if (row < 0 || row >= boardSize || col < 0 || col >= boardSize) {
        std::cerr << "Invalid board position: (" << p.x << ", " << p.y << ")" << std::endl;
        return { -1, -1 }; // Return an invalid position if the input is out of range
    }

    return { row, col };
}

std::string ChessBoardScreen::toChessNote(sf::Vector2f p)
{
    std::string s = "";
    s += static_cast<char>(isWhite ? (p.x / size + 97) : (7 - p.x / size + 97));
    s += static_cast<char>(isWhite ? (7 - p.y / size + 49) : (p.y / size + 49));
    return s;
}

sf::Vector2f ChessBoardScreen::toCoord(char a, char b)
{
    int x = static_cast<int>(a) - 97;
    int y = 7 - static_cast<int>(b) + 49;   
    return sf::Vector2f(x * size, y * size);
}

bool ChessBoardScreen::sendMatchmakingRequest()
{
    // Prepare the matchmaking request data (example)
    json matchmakingRequestData = {
        {"username", "player123"} // Replace "player123" with the actual username of the player
        // Add any other relevant data for matchmaking
    };

    // Send the matchmaking request to the server
    if (tcpClient.sendRequest(RequestType::MatchMaking, matchmakingRequestData))
    {
        std::cout << "Matchmaking request sent successfully!" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Failed to send matchmaking request to the server." << std::endl;
        return false;
    }
}

bool ChessBoardScreen::sendChallengeRequest() {
    if (isMatchFound || !isChallengeMode) return false;
    json requestData;
    requestData["opponent"] = opponent;
    requestData["board"] = reverseConvert(board);
    requestData["challenger"] = user.username;

    return tcpClient.sendRequest(RequestType::Challenge, requestData);
}

void ChessBoardScreen::processMatchmakingResponse(const std::string& response)
{
    // Process the matchmaking response from the server
    json jsonResponse = json::parse(response);
    // Assuming the server responds with JSON data as follows:
    // {
    //     "matchId": "123456",
    //     "opponentUsername": "opponent123"
    // }
    std::string matchId = jsonResponse["matchId"];
    std::string opponentUsername = jsonResponse["opponentUsername"];
    std::cout << "Match found! Match ID: " << matchId << ", Opponent: " << opponentUsername << std::endl;

    // Now you have the match information, and you can proceed with your game logic and rendering.
    // Set the appropriate flags or update the game state to indicate that a match is found.
    isMatchFound = true;
    isMoveAllowed = true;
    // Store the match details received from the server
    matchId = matchId;
    opponentUsername = opponentUsername;
}

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool containsSubstring(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}

bool ChessBoardScreen::handleMatchMakingResponse(json response) {
    bool found = response["data"]["success"].get<bool>();

    if (found)
    {
        isMatchFound = true;
        std::cout << "Match Found!" << std::endl;
        std::cout << response["data"]["user1"]["username"].get<std::string>() << std::endl;
        std::cout << "Me from Client: " << user.username <<  '/' << response["data"]["user1"]["username"].get<std::string>() << '/' << std::endl;
        if(user.username == response["data"]["user1"]["username"].get<std::string>())
        {
            isWhite = true;
            std::cout << "meText: " << "Me: " + response["data"]["user1"]["username"].get<std::string>() + std::to_string(response["data"]["user1"]["elo"].get<int>()) << std::endl;
            meText.setString("Me: " + response["data"]["user1"]["username"].get<std::string>()+ " " + std::to_string(response["data"]["user1"]["elo"].get<int>())); // Update "me" with the actual player name
            opponentText.setString("Opponent: " + response["data"]["user2"]["username"].get<std::string>() + " " + std::to_string(response["data"]["user2"]["elo"].get<int>()));
        }
        else {
            isWhite = false;
            opponentText.setString("Opponent: " + response["data"]["user1"]["username"].get<std::string>() + std::to_string(response["data"]["user1"]["elo"].get<int>())); // Update "me" with the actual player name
            meText.setString("Me: " + response["data"]["user2"]["username"].get<std::string>() + std::to_string(response["data"]["user2"]["elo"].get<int>()));
        }
    }
    return found;
}

void ChessBoardScreen::receiveGameStateResponse(json response)
{
    // Process the game state response from the server
    // Update the game state, chess board, and other relevant data based on the received data

    myTurn = response["data"]["myTurn"].get<bool>();

    if (response["data"]["success"].get<bool>())
    {

        std::string responseString = response["data"]["board"].get<std::string>();
        std::cout << "gamestate responseString: " << response["data"]["board"].get<std::string>() << std::endl;

        convertBoardResponse(board, responseString);
        std::cout << "gamestate responseString: " << responseString << std::endl;
        std::cout << "Server shouted: " << response["data"]["message"].get<std::string>() << std::endl;
    }
    else {
        displayErrorMessage(response["data"]["message"].get<std::string>());
        return;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << board[i][j];
        }
        std::cout << std::endl;
    }
    isUpdatingPosition = true;
}

void ChessBoardScreen::sendMoveToServer(const std::string& move)
{
    // Prepare the move data (example)
    json moveData;
    json moveJson = { {"move", move} };

    // Send the move data to the server
    if (tcpClient.sendRequest(RequestType::Move, moveJson))
    {
        std::cout << "Move sent to the server: " << moveJson << std::endl;
    }
    else
    {
        std::cerr << "Failed to send move to the server." << std::endl;
    }
}


void ChessBoardScreen::convertBoardResponse(int a[8][8], std::string s)
{
    int i, j, k = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            switch (s[k])
            {
            case 'p':
                a[i][j] = 6;
                break;

            case 'P':
                a[i][j] = -6;
                break;

            case 'k':
                a[i][j] = 5;
                break;

            case 'K':
                a[i][j] = -5;
                break;

            case 'c':
                a[i][j] = 2;
                break;

            case 'C':
                a[i][j] = -2;
                break;

            case 'r':
                a[i][j] = 1;
                break;

            case 'R':
                a[i][j] = -1;
                break;

            case 'b':
                a[i][j] = 3;
                break;

            case 'B':
                a[i][j] = -3;
                break;

            case 'q':
                a[i][j] = 4;
                break;

            case 'Q':
                a[i][j] = -4;
                break;

            case '-':
                a[i][j] = 0;
                break;
            default:
                break;
            }
            std::cout << a[i][j] << ":" << s[k];
            k++;
        }
        std::cout << std::endl;

    }
}


std::string ChessBoardScreen::reverseConvert(int a[8][8])
{
    std::string s = "";

    for (int i = 0; i <= 7; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            switch (a[i][j])
            {
            case 6:
                s.push_back('p');
                break;
            case -6:
                s.push_back('P');
                break;
            case 5:
                s.push_back('k');
                break;
            case -5:
                s.push_back('K');
                break;
            case 2:
                s.push_back('c');
                break;
            case -2:
                s.push_back('C');
                break;
            case 1:
                s.push_back('r');
                break;
            case -1:
                s.push_back('R');
                break;
            case 3:
                s.push_back('b');
                break;
            case -3:
                s.push_back('B');
                break;
            case 4:
                s.push_back('q');
                break;
            case -4:
                s.push_back('Q');
                break;
            case 0:
                s.push_back('-');
                break;
            default:
                break;
            }
        }
    }
    return s;
}