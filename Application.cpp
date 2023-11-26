#include "Application.h"


Application::Application() {
	initialize();
}


Application::~Application() {
	// Shuts down imgui
	ImGui::SFML::Shutdown(*m_window);

	delete m_window;
}


void Application::initialize() {
	// Initializes the window
	sf::VideoMode windowSize = { 1200u, 600u };
	sf::String windowTitle = "Marching Squares Algorithm";
	size_t windowStyle = sf::Style::Close;

	m_window = new sf::RenderWindow{ windowSize, windowTitle, windowStyle };
	ImGui::SFML::Init(*m_window);

	// Initializes the coordinate plane
	m_plane = new CoordinatePlane{ m_window, sf::View{{ 0.0f, 0.0f }, {4.0f, 2.0f}} };
	m_plane->resolution = 0.2f;
}


void Application::handleInput() {
	sf::Event sfmlEvent;
	while (m_window->pollEvent(sfmlEvent)) {
		// Lets imgui handle its input
		ImGui::SFML::ProcessEvent(sfmlEvent);
		// Lets the coordinate plane handle panning and zooming
		if (!m_guiInFocus) m_plane->handleInputs(sfmlEvent);

		switch (sfmlEvent.type) {
		// The close button is pressed
		case sf::Event::Closed:
			m_window->close();
			break;
		}
	}
}


void Application::update() {
	// Updates imgui
	sf::Time clockRestart = m_clock.restart();
	ImGui::SFML::Update(*m_window, clockRestart);

	// Calculates the time elapsed between frames
	float deltaTime = clockRestart.asSeconds();
}


void Application::gui() {
	ImGui::Begin("Settings");
	m_guiInFocus = ImGui::IsWindowHovered();
	
	ImGui::Checkbox("Show Contour", &m_plane->showContour);
	ImGui::Checkbox("Show Axis", &m_plane->showAxis);
	ImGui::Checkbox("Show Dots", &m_plane->showDots);
	ImGui::Checkbox("Show Grid", &m_plane->showGrid);
	ImGui::SliderFloat("Resolution", &m_plane->resolution, 0.09f, 1.6f);

	ImGui::End();
}
  

void Application::render() {
	// Clears the window
	m_window->clear(sf::Color{ 50, 40, 80 });

	m_plane->render([&](float x, float y)
		{ return x * x + std::pow(y - std::sqrtf(std::fabs(x)), 2); }, 3.0f);
	
	// Renders the imgui
	ImGui::SFML::Render(*m_window);

	// Displays the current frame
	m_window->display();
}


void Application::run() {
	while (m_window->isOpen()) {
		handleInput();
		update();
		gui();
		render();
	}
}
