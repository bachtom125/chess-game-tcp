#include "OnlineUserListScreen.hpp"

OnlineUserListScreen::OnlineUserListScreen(sf::RenderWindow& window)
	: window(window)
{
	if (!font.loadFromFile("fonts/inter.ttf")) {
		// Handle font loading error
		// Print an error message, throw an exception, or take appropriate action
		// ...
	}

	if (!backgroundTexture.loadFromFile("images/login-bg.jpg")) {
		// Handle background image loading error
		// Print an error message, throw an exception, or take appropriate action
		// ...
	}

	backgroundSprite.setTexture(backgroundTexture);
	backgroundSprite.setScale(
		static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
		static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
	);

	usernameBox.setSize(sf::Vector2f(200, 30));
	usernameBox.setPosition(100, 100);
	usernameBox.setFillColor(sf::Color::White);

	eloBox.setSize(sf::Vector2f(80, 30));
	eloBox.setPosition(350, 100);
	eloBox.setFillColor(sf::Color::White);

	challengeButton.setSize(sf::Vector2f(80, 30));
	challengeButton.setPosition(450, 100);
	challengeButton.setFillColor(sf::Color::White);

	backButton.setSize(sf::Vector2f(100, 30));
	backButton.setPosition(600, 550);
	backButton.setFillColor(sf::Color::White);

	usernameText.setFont(font);
	usernameText.setCharacterSize(16);
	usernameText.setFillColor(sf::Color::Black);
	usernameText.setPosition(110, 100);
	usernameText.setString("Username");

	eloText.setFont(font);
	eloText.setCharacterSize(16);
	eloText.setFillColor(sf::Color::Black);
	eloText.setPosition(360, 100);
	eloText.setString("ELO");

	challengeText.setFont(font);
	challengeText.setCharacterSize(16);
	challengeText.setFillColor(sf::Color::Black);
	challengeText.setPosition(460, 100);
	challengeText.setString("Challenge");

	backText.setFont(font);
	backText.setCharacterSize(16);
	backText.setFillColor(sf::Color::Black);
	backText.setPosition(610, 550);
	backText.setString("Back");

	selectedItemBackground.setSize(sf::Vector2f(800, 30));
	selectedItemBackground.setPosition(100, 100);
	selectedItemBackground.setFillColor(sf::Color(100, 100, 100, 150));
}

void OnlineUserListScreen::handleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		// Check for Back button click
		if (backButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
		{
			// Handle Back button logic here
			// ...
			isBack = true;
			return;
		}

		// Check for Challenge button click
		if (challengeButton.getGlobalBounds().contains(mousePos.x, mousePos.y) && challengeButtonVisible)
		{
			// Handle Challenge button logic here
			// ...
			selectedUsername = userList[selectedItemIndex].username;
			return;
		}

		// Check for selection from the list
		for (size_t i = 0; i < userList.size(); ++i)
		{
			if (usernameBox.getGlobalBounds().contains(mousePos.x, mousePos.y + static_cast<float>(i * 30)))
			{
				selectedItemIndex = static_cast<int>(i);
				updateChallengeButtonVisibility();
				return;
			}
		}

		// If the user clicks outside the list, reset selection
		selectedItemIndex = -1;
		updateChallengeButtonVisibility();
	}
}

void OnlineUserListScreen::update()
{
	// Add any additional update logic here, if needed
	// ...
}

void OnlineUserListScreen::draw()
{
	window.clear();
	window.draw(backgroundSprite);

	// Draw the user list with scroll functionality
	int maxVisibleItems = 10;
	int startIndex = selectedItemIndex - maxVisibleItems / 2;
	startIndex = std::max(0, std::min(static_cast<int>(userList.size()) - maxVisibleItems, startIndex));

	for (int i = startIndex; i < startIndex + maxVisibleItems && i < userList.size(); ++i)
	{
		float posY = 100.f + static_cast<float>(i - startIndex) * 30.f;

		usernameText.setString(userList[i].username);
		usernameBox.setPosition(100.f, posY);

		eloText.setString(std::to_string(userList[i].elo));
		eloBox.setPosition(350.f, posY);

		window.draw(usernameBox);
		window.draw(usernameText);
		window.draw(eloBox);
		window.draw(eloText);
	}

	// Draw Challenge and Back buttons
	if (selectedItemIndex != -1)
	{
		window.draw(selectedItemBackground);

		challengeButtonVisible = true;
		challengeButton.setPosition(450, 100.f + static_cast<float>(selectedItemIndex - startIndex) * 30.f);
		window.draw(challengeButton);
		window.draw(challengeText);
	}
	else
	{
		challengeButtonVisible = false;
	}

	window.draw(backButton);
	window.draw(backText);

}

void OnlineUserListScreen::scrollUp()
{
	// Scroll up the list by decreasing the selectedItemIndex
	selectedItemIndex = std::max(selectedItemIndex - 1, -1);
	updateChallengeButtonVisibility();
}

void OnlineUserListScreen::scrollDown()
{
	// Scroll down the list by increasing the selectedItemIndex
	selectedItemIndex = std::min(selectedItemIndex + 1, static_cast<int>(userList.size()) - 1);
	updateChallengeButtonVisibility();
}

void OnlineUserListScreen::updateChallengeButtonVisibility()
{
	// Check if the Challenge button should be visible based on selection
	challengeButtonVisible = (selectedItemIndex != -1);
}

void OnlineUserListScreen::receiveUserListData(const nlohmann::json& userListData)
{
	// Clear the current user list
	userList.clear();

	// Parse the received JSON data and update the userList
	for (const auto& userJson : userListData)
	{
		UserItem user;
		user.username = userJson["username"].get<std::string>();
		user.elo = userJson["elo"].get<int>();
		userList.push_back(user);
	}
}