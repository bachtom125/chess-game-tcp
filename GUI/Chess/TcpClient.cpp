#include "TcpClient.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

TcpClient::TcpClient(const std::string& serverAddress, unsigned short serverPort)
    : serverAddress(serverAddress), serverPort(serverPort)
{
}

bool TcpClient::connect()
{
    if (socket.connect(serverAddress, serverPort) == sf::Socket::Done) {
        return true;
    }
    else {
        return false;
    }
}

bool TcpClient::send(const std::string& message)
{
    if (socket.send(message.c_str(), message.size() + 1) == sf::Socket::Done) {
        return true;
    }
    else {
        return false;
    }
}

bool TcpClient::receive(std::string& receivedData)
{
    char buffer[128];
    std::size_t received;
    if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {
        receivedData = std::string(buffer, received);
        return true;
    }
    else {
        return false;
    }
}


bool TcpClient::sendLoginRequest(const std::string& username, const std::string& password)
{
    json loginRequest;
    loginRequest["username"] = username;
    loginRequest["password"] = password;

    std::string serializedRequest = loginRequest.dump();

    sf::Packet packet;
    packet << serializedRequest;

    sf::Socket::Status status = socket.send(packet);
    return (status == sf::Socket::Done);
}