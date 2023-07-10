#pragma once

#include <SFML/Network.hpp>

class TcpClient
{
public:
    TcpClient(const std::string& serverAddress, unsigned short serverPort);

    bool connect();
    bool send(const std::string& message);
    bool receive(std::string& receivedData);
    bool sendLoginRequest(const std::string& username, const std::string& password);
    

private:
    sf::TcpSocket socket;
    std::string serverAddress;
    unsigned short serverPort;
};