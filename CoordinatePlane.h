#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>

#include <cmath>
#include <functional>
#include <vector>


// Better mod function as it always returns a positive value
inline float mod(float a, float b) {
	return std::fmod((std::fmod(a, b) + b), b);
}


// Class that is responsible for all the heavy lifting
// It handles zooming and panning of the coordinate plane
// and renders everything
class CoordinatePlane {
private:
	sf::RenderWindow* m_window;
	sf::View m_viewport;

	bool m_panning = false;
	sf::Vector2f m_oldMousePos;

	float m_zoomFactor = 1.1f;
	float m_zoomLimitLow = 0.0f;
	float m_zoomLimitHigh = 10000.0f;
	float m_totalZoom = 1.0f;

private:
	// Pans the view according to the movement of the mouse
	void pan(const sf::Vector2f& newMousePos);
	// Zooms the view at the given pixel
	void zoomAtPixel(float scrollDelta);

	// Renders the contour onto the window
	void renderContour(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight,
		std::function<float(float, float)> func, float threshold);
	// Renders the axis onto the window
	void renderAxis(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight);
	// Renders the dots onto the grid
	void renderDots(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight,
		std::function<float(float, float)> func, float threshold);
	// Renders the grid onto the window
	void renderGrid(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight);

public:
	// Options that can be freely configured
	bool showContour = true, showAxis = true, showDots = true, showGrid = true;
	float resolution;

public:
	// Constructor
	CoordinatePlane(sf::RenderWindow* window, const sf::View& viewport);

	// Zoom settings
	float getZoomFactor() const { return m_zoomFactor; }
	void setZoomFactor(float zoomFactor) { m_zoomFactor = zoomFactor; }
	void setZoomLimitLow(float zoomLimitLow) { m_zoomLimitLow = zoomLimitLow; }
	void setZoomLimitHigh(float zoomLimitHigh) { m_zoomLimitHigh = zoomLimitHigh; }

	// Handles user input in regards to zooming and panning
	void handleInputs(const sf::Event& sfmlEvent);

	// Renders the coordinate plane onto the window
	void render(std::function<float(float, float)> func, float threshold);
};
