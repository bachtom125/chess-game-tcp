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
    std::cout << "Connect: " << std::endl;

    if (socket.connect(serverAddress, serverPort) == sf::Socket::Done) {
        std::cout << "Connect successfull: " << std::endl;

        return true;
    }
    else {
        std::cout << "Connect fail: " << std::endl;

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
    char buffer[1024];
    std::size_t received;
    if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {


        receivedData = std::string(buffer, received);
        std::cout << "receieved: " << received << std::endl;
        std::cout << "receievedData: " << receivedData << std::endl;
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

    std::cout << "Serialized request: " << serializedRequest << std::endl;

    sf::Socket::Status status = socket.send(packet);

    std::cout << "Serialized request after sent: " << serializedRequest << std::endl;

    return (status == sf::Socket::Done);
}