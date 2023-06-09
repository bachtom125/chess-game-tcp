#include <iostream>
#include "ChessBoardScreen.hpp"

ChessBoardScreen::ChessBoardScreen(sf::RenderWindow& window)
    : window(window)
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

    // Handle events specific to the chess board screen

    if (event.type == sf::Event::MouseButtonPressed)
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
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            isMove = false;
            sf::Vector2i pos = sf::Mouse::getPosition(window) - sf::Vector2i(offset);
            newPos = sf::Vector2f(size * int(pos.x / size), size * int(pos.y / size));
            str = toChessNote(oldPos) + toChessNote(newPos);
            move(str);
            if (oldPos != newPos)
                position += str + " ";
            f[n].setPosition(newPos);
        }
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
}

void ChessBoardScreen::draw()
{
    window.draw(sBoard);
    for (int i = 0; i < 32; i++)
        f[i].move(offset);
    for (int i = 0; i < 32; i++)
        window.draw(f[i]);
    for (int i = 0; i < 32; i++)
        f[i].move(-offset);
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