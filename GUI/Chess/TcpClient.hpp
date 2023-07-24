#pragma once

#include <SFML/Network.hpp>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class RequestType
{
    Login,
    Logout,
    MatchMaking,
    Challenge,
    Move,
    GetOnlinePlayersList,
    SomeOtherRequest,
    // Add more request types as needed
};

enum class RespondType
{
    Login,
    Logout,
    MatchMaking,
    Move, // Working ... needs implementing
    Challenge,
    GameResult, // Working ... needs implementing
    OnlinePlayersList,
};

class TcpClient
{
public:
    TcpClient(const std::string& serverAddress, unsigned short serverPort);

    bool connect();
    bool send(const std::string& message);
    bool receive(std::string& receivedData);
    bool sendRequest(RequestType type, const json& requestData);

private:
    sf::TcpSocket socket;
    std::string serverAddress;
    unsigned short serverPort;
};