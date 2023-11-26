#pragma once

#include "CoordinatePlane.h"


// Main class that runs the application
class Application {
private:
	sf::RenderWindow* m_window;
	sf::Clock m_clock;

	CoordinatePlane* m_plane;
	bool m_guiInFocus = false;

private:
	// Initializes the application
	void initialize();

	// Handles input from the user
	void handleInput();
	// Updates the application
	void update();
	// Deals with the gui
	void gui();
	// Renders onto the window
	void render();

public:
	// Constructor
	Application();
	// Destructor
	~Application();

	// Starts and runs the application
	void run();
};
