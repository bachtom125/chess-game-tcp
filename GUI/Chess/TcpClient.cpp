#include "TcpClient.hpp"
#include "nlohmann/json.hpp"
#include <iostream>

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

bool TcpClient::sendRequest(RequestType type, const json& requestData)
{
    // Create the request JSON
    std::cout << "Request Sent: " << requestData << std::endl;
    json request;
    request["type"] = static_cast<int>(type);
    request["data"] = requestData;

    // Serialize the request JSON
    std::string serializedRequest = request.dump();

    sf::Packet packet;
    packet << serializedRequest;

    sf::Socket::Status status = socket.send(packet);
    return (status == sf::Socket::Done);
}