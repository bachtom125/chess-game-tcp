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

    loadPosition();

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
}

void ChessBoardScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        std::cout << "close" << std::endl;
        window.close();
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
            }
        }

        if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left && !firstMouseRelease)
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

    if (!isMatchFound && !startFindingMatchMaking)
    {
        // Send a request to the server to check for match found
        // For example, assuming the request type is RequestType::CheckMatchFound
        json requestData; // Populate the JSON data for the request if needed
        if (tcpClient.sendRequest(RequestType::MatchMaking, requestData))
        {
            startFindingMatchMaking = true;
        }

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
    window.draw(f[n]);
    for (int i = 0; i < 32; i++)
     f[i].move(-offset);

    window.draw(meText);
    window.draw(opponentText);

}

void ChessBoardScreen::loadPosition()
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
            f[k].setTextureRect(sf::IntRect(size * x, size * y, size, size));
            f[k].setPosition(size * j, size * (7 - i));
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

std::string ChessBoardScreen::toChessNote(sf::Vector2f p)
{
    std::string s = "";
    s += static_cast<char>(p.x / size + 97);
    s += static_cast<char>(7 - p.y / size + 49);
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

void ChessBoardScreen::handleMatchMakingResponse(json response) {
    bool found = response["data"]["success"].get<bool>();

    if (found)
    {
        isMatchFound = true;
        std::cout << "Match Found!" << std::endl;
        std::cout << response["data"]["user1"]["username"].get<std::string>() << std::endl;
        std::cout << "Me from Client: " << user.username <<  '/' << response["data"]["user1"]["username"].get<std::string>() << '/' << std::endl;
        if(user.username == response["data"]["user1"]["username"].get<std::string>())
        {
            std::cout << "meText: " << "Me: " + response["data"]["user1"]["username"].get<std::string>() + std::to_string(response["data"]["user1"]["elo"].get<int>()) << std::endl;
            meText.setString("Me: " + response["data"]["user1"]["username"].get<std::string>()+ " " + std::to_string(response["data"]["user1"]["elo"].get<int>())); // Update "me" with the actual player name
            opponentText.setString("Opponent: " + response["data"]["user2"]["username"].get<std::string>() + " " + std::to_string(response["data"]["user2"]["elo"].get<int>()));
        }
        else {
            opponentText.setString("Opponent: " + response["data"]["user1"]["username"].get<std::string>() + std::to_string(response["data"]["user1"]["elo"].get<int>())); // Update "me" with the actual player name
            meText.setString("Me: " + response["data"]["user2"]["username"].get<std::string>() + std::to_string(response["data"]["user2"]["elo"].get<int>()));
        }
    }
}

void ChessBoardScreen::receiveGameStateResponse(json response)
{
    // Process the game state response from the server
    // Update the game state, chess board, and other relevant data based on the received data


    if (response["data"]["success"].get<bool>())
    {
        myTurn = response["data"]["myTurn"];

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
    loadPosition();
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

