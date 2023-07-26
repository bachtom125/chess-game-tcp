#include "UserEntry.hpp"
#include <iostream>

UserEntry::UserEntry(const std::string& username, int elo, sf::Font font, sf::RenderWindow& window)
    : username(username), elo(elo), selected(false), font(font), window(window)
{
    // Customize the appearance of the user entry rectangle here
    entryRect.setSize(sf::Vector2f(200.0f, 60.0f));
    entryRect.setFillColor(sf::Color(120, 120, 120));
    entryRect.setOutlineThickness(2.0f);
    entryRect.setOutlineColor(sf::Color::Black);

    // Customize the text appearance for username and elo
    usernameText.setString(username);
    usernameText.setFont(font);
    usernameText.setCharacterSize(20);
    usernameText.setFillColor(sf::Color::White);

    eloText.setString("ELO: " + std::to_string(elo));
    eloText.setFont(font);
    eloText.setCharacterSize(16);
    eloText.setFillColor(sf::Color::White);
}

void UserEntry::setPosition(float x, float y)
{
    entryRect.setPosition(x, y);
    usernameText.setPosition(x + 10.0f, y + 10.0f);
    eloText.setPosition(x + 10.0f, y + 35.0f);
}

void UserEntry::setSelected(bool isSelected)
{
    selected = isSelected;
    // Customize the appearance when selected
    if (selected)
    {
        entryRect.setOutlineColor(sf::Color::Red);
    }
    else
    {
        entryRect.setOutlineColor(sf::Color::Black);
    }
}

bool UserEntry::contains(float x, float y) const
{
    return entryRect.getGlobalBounds().contains(x, y);
}

void UserEntry::draw() const
{
    window.draw(entryRect);
    //window.draw(usernameText);
    //window.draw(eloText);//
}