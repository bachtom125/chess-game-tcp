#include "ScreenManager.hpp"

int main()
{
    TcpClient tcpClient("127.0.0.1", 5500   );
    tcpClient.connect();

    sf::RenderWindow window(sf::VideoMode(800, 600), "Screen Manager");

    ScreenManager screenManager(window, tcpClient);
    screenManager.run();

    return 0;
}