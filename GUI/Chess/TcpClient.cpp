#include "TcpClient.hpp"
#include "nlohmann/json.hpp"
#include <iostream>


using json = nlohmann::json;
#define BUFF_SIZE 1024
const std::string DELIMITER = "-|";
const std::string END_DELIMITER = "=|"; // End delimiter to signify the complete message

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
    char buffer[BUFF_SIZE];
    std::size_t received;
    //if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {


    //    receivedData = std::string(buffer, received);

    //    // Find the position of the first opening brace '{'
    //    size_t bracePos = receivedData.find_first_of('{');
    //    if (bracePos != std::string::npos)
    //    {
    //        // Extract the substring from the brace position until the end of the string
    //        receivedData = receivedData.substr(bracePos);
    //    }

    //    std::cout << "receieved: " << received << std::endl;
    //    std::cout << "receievedData: " << receivedData << std::endl;
    //    return true;
    //}
    //else {     
    //    return false;
    //}

    std::string responseData = "";
    std::string currentData = "";
    while (true)
    {
        // Receive data from the client
        if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done)
        {
            // Append the received data to the current data
            currentData += std::string(buffer, received);
            // cout << "currentData :" << currentData << "::" << endl;

            // Check if the end delimiter is received

            // Process the message in chunks, using the delimiter
            size_t delimiter_pos;
            while ((delimiter_pos = currentData.find(DELIMITER)) != std::string::npos)
            {
                std::string chunk = currentData.substr(0, delimiter_pos);
                responseData += chunk;
                // Remove the processed chunk (including the delimiter) from the current data
                currentData.erase(0, delimiter_pos + DELIMITER.length());
            }
            if (currentData.find(END_DELIMITER) != std::string::npos)
            {
                receivedData = responseData;
                return true;
            }
        }
        else return false;
    }
    return false;
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

    std::cout << "Serialized request: " << packet << std::endl;

    sf::Socket::Status status = socket.send(packet);

    std::cout << "Serialized request after sent: " << serializedRequest << std::endl;

    return (status == sf::Socket::Done);
}