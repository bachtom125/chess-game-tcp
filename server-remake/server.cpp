#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include <fstream> // Add this line

using json = nlohmann::json;

constexpr int PORT = 3000;

enum class RequestType
{
    Login,
    SomeOtherRequest,
    // Add more request types as needed
};

constexpr const char *ACCOUNTS_FILE = "accounts.txt";

struct User
{
    std::string username;
    std::string password;
    int elo;
};

std::vector<User> readAccountsFile()
{
    std::vector<User> users;
    std::ifstream accountsFile("accounts.txt");
    if (!accountsFile)
    {
        std::cerr << "Failed to open accounts file" << std::endl;
        return users;
    }

    std::string line;
    while (std::getline(accountsFile, line))
    {
        std::istringstream iss(line);
        User user;
        if (iss >> user.username >> user.password >> user.elo)
        {
            users.push_back(user);
        }
    }

    return users;
}

User findUserByUsername(const std::string &username)
{
    std::vector<User> users = readAccountsFile();
    for (const User &user : users)
    {
        if (user.username == username)
        {
            return user;
        }
    }

    // Return a default-constructed User object if the user is not found
    return User();
}

bool isUserValid(const std::string &username, const std::string &password)
{
    std::ifstream accounts(ACCOUNTS_FILE);
    if (!accounts)
    {
        std::cerr << "Failed to open accounts file" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(accounts, line))
    {
        std::string storedUsername, storedPassword;
        std::istringstream iss(line);
        if (iss >> storedUsername >> storedPassword)
        {
            if (storedUsername == username && storedPassword == password)
            {
                return true;
            }
        }
    }

    return false;
}

void handleLoginRequest(const json &requestData, int clientSocket)
{
    std::string username = requestData["username"];
    std::string password = requestData["password"];

    std::cout << username << std::endl;
    std::cout << password << std::endl;

    // Perform login validation/authentication logic
    User user = findUserByUsername(username);
    bool isValid = (user.username == username && user.password == password);
    // Craft the response JSON
    json response;
    response["type"] = static_cast<int>(RequestType::Login);
    response["success"] = isValid;
    response["message"] = isValid ? "Login successful" : "Invalid username or password";

    // Serialize the response JSON
    std::string responseStr = response.dump();

    // Send the response back to the client
    if (send(clientSocket, responseStr.c_str(), responseStr.size(), 0) == -1)
    {
        std::cerr << "Failed to send response to client" << std::endl;
    }
}

void handleSomeOtherRequest(const json &requestData)
{
    // Handle the specific logic for some other type of request
    // ...
}

int main()
{
    std::cout << "Server started" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Listening for connections on port " << PORT << std::endl;

    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    while (true)
    {
        std::cout << "Waiting for incoming connections..." << std::endl;

        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocket == -1)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        std::cout << "Accepted client connection" << std::endl;

        std::array<char, 1024> buffer{};
        ssize_t bytesRead = recv(clientSocket, buffer.data(), buffer.size(), 0);
        if (bytesRead <= 0)
        {
            std::cerr << "Error receiving data" << std::endl;
            close(clientSocket);
            continue;
        }

        // Parse the received data into JSON
        std::string requestData(buffer.data(), bytesRead);
        std::cout << requestData << std::endl;
        // Find the position of the first opening brace '{'
        size_t bracePos = requestData.find_first_of('{');
        if (bracePos != std::string::npos)
        {
            // Extract the substring from the brace position until the end of the string
            std::string jsonSubstring = requestData.substr(bracePos);

            // Parse the extracted JSON substring
            json jsonData = json::parse(jsonSubstring);

            // Determine the type of request and dispatch to the appropriate handler
            int requestType = jsonData["type"];

            if (requestType == static_cast<int>(RequestType::Login))
            {
                std::cout << "Received login request" << std::endl;
                handleLoginRequest(jsonData["data"], clientSocket);
            }
            else if (requestType == static_cast<int>(RequestType::SomeOtherRequest))
            {
                std::cout << "Received some other request" << std::endl;
                handleSomeOtherRequest(jsonData["data"]);
            }
            else
            {
                // Handle unknown or unsupported request types
                std::cerr << "Unknown request type: " << requestType << std::endl;
            }
        }
        else
        {
            // Handle the case where no opening brace is found
            std::cerr << "Invalid request data: " << requestData << std::endl;
        }

        // Close the server socket
    }

    close(serverSocket);
    std::cout << "Server shut down" << std::endl;
    return 0;
}