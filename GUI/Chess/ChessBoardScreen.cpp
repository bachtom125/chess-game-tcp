#include <iostream>
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
}

void ChessBoardScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        std::cout << "close" << std::endl;
        window.close();
    }

    if (!isMatchFound && !isMoveAllowed) return;


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
            isMove = false;
            sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
            newPos = sf::Vector2f(size * int(pos.x / size), size * int(pos.y / size));
            str = toChessNote(oldPos) + toChessNote(newPos);
            std::string moveStr = toChessNote(oldPos) + " " + toChessNote(newPos);

            if (oldPos != newPos)
            {
                position += str + " ";
                sendMoveToServer(moveStr);
            }
            f[n].setPosition(newPos);
        }

        firstMouseRelease = false;

    }
}

void ChessBoardScreen::update()
{
    // Update the chess board screen logic
    // ...

    // For example, you can add an option to make the computer move
    //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    //{
    //    str = getNextMove(position);

    //    oldPos = toCoord(str[0], str[1]);
    //    newPos = toCoord(str[2], str[3]);

    //    for (int i = 0; i < 32; i++)
    //    {
    //        if (f[i].getPosition() == oldPos)
    //            n = i;
    //    }

    //    // Animation
    //    for (int k = 0; k < 50; k++)
    //    {
    //        sf::Vector2f p = newPos - oldPos;
    //        f[n].move(p.x / 50, p.y / 50);
    //        window.draw(sBoard);
    //        for (int i = 0; i < 32; i++)
    //            f[i].move(offset);
    //        for (int i = 0; i < 32; i++)
    //            window.draw(f[i]);
    //        window.draw(f[n]);
    //        for (int i = 0; i < 32; i++)
    //            f[i].move(-offset);
    //        window.display();
    //    }

    //    move(str);
    //    position += str + " ";
    //    f[n].setPosition(newPos);
    //}

    if (!isMatchFound)
    {
        // Send a request to the server to check for match found
        // For example, assuming the request type is RequestType::CheckMatchFound
        json requestData; // Populate the JSON data for the request if needed
        if (tcpClient.sendRequest(RequestType::MatchMaking, requestData))
        {
            std::string receivedData;
            if (tcpClient.receive(receivedData))
            {
                // Parse the received data (you may need to adjust this based on your server response)
                json response = json::parse(receivedData);
                bool found = response["success"].get<bool>();

                if (found)
                {
                    isMatchFound = true;
                    std::cout << "Match Found!" << std::endl;
                }
            }
        }
    }
}

void ChessBoardScreen::draw()
{
    /////animation///////
    /*int n = 0;
    for (int i = 0; i < 32; i++)
        if (f[i].getPosition() == oldPos)
            n = i;
    for (int k = 0; k < 50; k++)
    {
        sf::Vector2f p = newPos - oldPos;
        f[n].move(p.x / 50, p.y / 50);
        window.draw(sBoard);
        for (int i = 0; i < 32; i++)
            f[i].move(offset);
        for (int i = 0; i < 32; i++)
            window.draw(f[i]);
        window.draw(f[n]);
        for (int i = 0; i < 32; i++)
            f[i].move(-offset);
    }*/
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
            int y = n > 0 ? 1 : 0;
            f[k].setTextureRect(sf::IntRect(size * x, size * y, size, size));
            f[k].setPosition(size * j, size * i);
            k++;
        }
    }

    for (int i = 0; i < position.length(); i += 5)
        move(position.substr(i, 4));
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

    // Castling
    if (str == "e1g1")
    {
        if (position.find("e1") == std::string::npos)
            move("h1f1");
    }
    if (str == "e8g8")
    {
        if (position.find("e8") == std::string::npos)
            move("h8f8");
    }
    if (str == "e1c1")
    {
        if (position.find("e1") == std::string::npos)
            move("a1d1");
    }
    if (str == "e8c8")
    {
        if (position.find("e8") == std::string::npos)
            move("a8d8");
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

void ChessBoardScreen::receiveGameStateResponse(const std::string& responseString)
{
    // Process the game state response from the server
    // Update the game state, chess board, and other relevant data based on the received data

    std::cout << responseString << std::endl;
    
    Fixed2DArray& newBoard = convertBoardResponse(responseString);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = newBoard[i][j];
        }
    }
    loadPosition();
}

void ChessBoardScreen::sendMoveToServer(const std::string& move)
{
    // Prepare the move data (example)
    json moveData = {
        {"data",{"move", move},}
    };

    // Send the move data to the server
    if (tcpClient.sendRequest(RequestType::Move, moveData))
    {
        std::cout << "Move sent to the server: " << move << std::endl;

        // Receive the game state response from the server after sending the move
        std::string receivedData;
        if (tcpClient.receive(receivedData))
        {
            std::cout << "receivedData: " << receivedData << std::endl;
            // Call the function to process the received game state response
            receiveGameStateResponse(receivedData);
        }
    }
    else
    {
        std::cerr << "Failed to send move to the server." << std::endl;
    }
}


Fixed2DArray& ChessBoardScreen::convertBoardResponse(std::string s)
{
    int a[8][8];
    int i, j, k = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            switch (s[k++])
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
        }
    }

    return a;
}
