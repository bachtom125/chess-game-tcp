#include "LoginScreen.hpp"
#include <iostream>
#include <windows.h>

LoginScreen::LoginScreen(sf::RenderWindow& window, TcpClient& tcpClient)
	: window(window), tcpClient(tcpClient)
{
	if (!font.loadFromFile("fonts/inter.ttf")) {
		// Handle font loading error
		// Print an error message, throw an exception, or take appropriate action
		char currentDir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, currentDir);
		std::cout << "Current Directory: " << currentDir << std::endl;
		std::cout << "Error loading font" << std::endl;
		return;
	}

	if (!backgroundTexture.loadFromFile("images/login-bg.jpg")) {
		// Handle background image loading error
		// Print an error message, throw an exception, or take appropriate action
		std::cout << "Error loading background image" << std::endl;
		return;
	}

	backgroundSprite.setTexture(backgroundTexture);
	backgroundSprite.setScale(
		static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
		static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
	);
	usernameBox.setSize(sf::Vector2f(200, 30));
	usernameBox.setPosition(450, 200);
	usernameBox.setFillColor(sf::Color::White);

	passwordBox.setSize(sf::Vector2f(200, 30));
	passwordBox.setPosition(450, 250);
	passwordBox.setFillColor(sf::Color::White);

	usernameText.setFont(font);
	usernameText.setCharacterSize(16);
	usernameText.setFillColor(sf::Color::Black);
	usernameText.setPosition(460, 200);
	usernameText.setString("");

	passwordText.setFont(font);
	passwordText.setCharacterSize(16);
	passwordText.setFillColor(sf::Color::Black);
	passwordText.setPosition(460, 250);
	passwordText.setString("");

	signupText.setFont(font);
	signupText.setCharacterSize(16);
	signupText.setFillColor(sf::Color::Black);
	signupText.setPosition(460, 300);
	signupText.setString("Signin");

	signupButton.setSize(sf::Vector2f(100, 30));
	signupButton.setPosition(450, 300);
	signupButton.setFillColor(sf::Color::White);

}

void LoginScreen::handleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		std::cout << "Button pressed" << std::endl;
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		if (usernameBox.getGlobalBounds().contains(mousePos.x, mousePos.y))
		{
			usernameActive = true;
			passwordActive = false;
		}
		else if (passwordBox.getGlobalBounds().contains(mousePos.x, mousePos.y))
		{
			passwordActive = true;
			usernameActive = false;
		}
		else
		{
			usernameActive = false;
			passwordActive = false;
		}

		std::cout << "usernameActive: " << usernameActive << " - passwordActive: " << passwordActive << std::endl;


		if (signupButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {

			std::string enteredUsername = usernameText.getString();
			std::string enteredPassword = passwordText.getString();

			// Perform your login validation/authentication logic
			validateLogin();
			isRequestSent = true;
		}

	}

	if (event.type == sf::Event::TextEntered)
	{
		std::cout << "Text entered" << std::endl;
		std::cout << "usernameActive: " << usernameText.getString().toAnsiString() << " - passwordActive: " << passwordText.getString().toAnsiString() << std::endl;


		if (usernameActive)
		{
			handleTextInput(usernameText, event);
		}
		else if (passwordActive)
		{
			handleTextInput(passwordText, event);
		}
	}
}

void LoginScreen::validateLogin()
{
	if (startLogin) return;
	// Replace this with your actual login validation/authentication logic
	// For demonstration purposes, let's assume a valid username is "admin" and password is "password"
	json loginRequest;
	loginRequest["username"] = usernameText.getString();
	loginRequest["password"] = passwordText.getString();
	tcpClient.sendRequest(RequestType::Login, loginRequest);
	startLogin = true;
	// Block and wait for server response

}

void LoginScreen::handleLoginResponse(json json_data) {
	if (json_data["data"]["success"].get<bool>())
	{
		User newUser;
		std::cout << "user: " << json_data["data"].dump() << std::endl;
		newUser.username = json_data["data"]["username"];
		newUser.password = json_data["data"]["password"];
		newUser.elo = json_data["data"]["elo"];
		std::cout << "user.username: " << newUser.username << std::endl;
		user = newUser;
		isLoginSuccessful = true;
	}
	else {
		isLoginSuccessful = false;
		displayErrorMessage("Wrong username or password");
	}
	startLogin = false;

}

void LoginScreen::displayErrorMessage(const std::string& message)
{
	// Replace this with your actual implementation to display an error message
	// For example, you can use an sf::Text object to render the message on the screen
	sf::Text errorMessage;
	errorMessage.setFont(font);
	errorMessage.setCharacterSize(16);
	errorMessage.setFillColor(sf::Color::Red);
	errorMessage.setString(message);
	errorMessage.setPosition(300, 475);

	window.draw(errorMessage);
	window.display();

	// Optionally, you can add a delay to show the error message for a certain duration before clearing it
	sf::sleep(sf::seconds(2));
}

void LoginScreen::update()
{
	
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
	{
		// Perform your login validation/authentication logic
		validateLogin();
		isRequestSent = true;
	}
	

	// Handle other update logic for the login screen
	// ...
}
void LoginScreen::draw()
{
	// Draw separate labels for username and password
	sf::Text usernameLabel = usernameText; // Create a copy of usernameText
	sf::Text passwordLabel = passwordText; // Create a copy of passwordText

	usernameLabel.setString("Username:");
	passwordLabel.setString("Password:");
	usernameLabel.setFillColor(sf::Color::White);
	passwordLabel.setFillColor(sf::Color::White);

	// Adjust the positions of the labels
	usernameLabel.move(-100.f, 0.f);
	passwordLabel.move(-100.f, 0.f);

	// Draw
	window.clear();
	window.draw(backgroundSprite);
	window.draw(usernameBox);
	window.draw(passwordBox);
	window.draw(usernameText);
	window.draw(passwordText);
	window.draw(signupButton);
	window.draw(signupText);

	window.draw(usernameLabel);
	window.draw(passwordLabel);

}

void LoginScreen::handleTextInput(sf::Text& text, const sf::Event& event)
{
	if (event.text.unicode == 8 && !text.getString().isEmpty())
	{
		std::string input = text.getString();
		input.pop_back();
		text.setString(input);
	}
	else if (event.text.unicode >= 32 && event.text.unicode <= 126)
	{
		text.setString(text.getString() + static_cast<char>(event.text.unicode));
	}
}