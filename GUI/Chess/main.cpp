#include "ScreenManager.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Screen Manager");

    ScreenManager screenManager(window);
    screenManager.run();

    return 0;
}