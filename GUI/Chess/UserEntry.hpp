#include <SFML/Graphics.hpp>
#include <string>

class UserEntry
{
public:
    UserEntry(const std::string& username, int elo, sf::Font font, sf::RenderWindow& window);
    void setPosition(float x, float y);
    void setSelected(bool selected);
    bool contains(float x, float y) const;
    void draw() const;

private:
    std::string username;
    int elo;
    bool selected;
    sf::RenderWindow& window;

    sf::RectangleShape entryRect;
    sf::Text usernameText;
    sf::Text eloText;
    sf::Font font;

};