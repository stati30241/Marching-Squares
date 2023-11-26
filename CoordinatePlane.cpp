#include "CoordinatePlane.h"


CoordinatePlane::CoordinatePlane(sf::RenderWindow* window, const sf::View& viewport)
	: m_window{ window }, m_viewport{ viewport } {
	m_window->setView(viewport);

	m_totalZoom = viewport.getSize().x / static_cast<float>(window->getSize().x);
}


void CoordinatePlane::pan(const sf::Vector2f& newMousePos) {
	// Return if we are not supposed to be panning right now
	if (!m_panning) return;

	// Calculates the offset and moves the view accordingly
	const sf::Vector2f offset = m_oldMousePos - newMousePos;
	sf::View view = m_window->getView();
	view.move(offset * m_totalZoom);
	m_window->setView(view);

	// Sets the old mouse position to the current mouse position
	m_oldMousePos = newMousePos;
}


void CoordinatePlane::zoomAtPixel(float scrollDelta) {
	// Calculates if the user is zooming in or out
	float zoom = 1.0f;

	if (scrollDelta < 0.0f) {
		// User is zooming in
		if (m_totalZoom >= m_zoomLimitHigh) return;
		zoom = m_zoomFactor;
	} else if (scrollDelta > 0.0f) {
		// User is zooming out
		if (m_totalZoom <= m_zoomLimitLow) return;
		zoom = 1.0f / m_zoomFactor;
	}
	m_totalZoom *= zoom;

	// Calculates and zooms the view
	const sf::Vector2i pixel = sf::Mouse::getPosition(*m_window);
	const sf::Vector2f oldPixelPos = m_window->mapPixelToCoords(pixel);

	sf::View view = m_window->getView();
	view.zoom(zoom);
	m_window->setView(view);

	const sf::Vector2f newPixelPos = m_window->mapPixelToCoords(pixel);
	const sf::Vector2f offset = oldPixelPos - newPixelPos;

	view.move(offset);
	m_window->setView(view);
}


void CoordinatePlane::handleInputs(const sf::Event& sfmlEvent) {
	switch (sfmlEvent.type) {
	// The mouse wheel is scrolled
	case sf::Event::MouseWheelScrolled:
		zoomAtPixel(sfmlEvent.mouseWheelScroll.delta);
		break;

	// A mouse button is pressed
	case sf::Event::MouseButtonPressed:
		if (sfmlEvent.mouseButton.button == sf::Mouse::Left) {
			m_oldMousePos = sf::Vector2f{ sf::Mouse::getPosition(*m_window) };
			m_panning = true;
		}
		break;

	// A mouse button is released
	case sf::Event::MouseButtonReleased:
		if (sfmlEvent.mouseButton.button == sf::Mouse::Left) {
			m_panning = false;
		}
		break;

	// The mouse has been moved
	case sf::Event::MouseMoved:
		pan(sf::Vector2f{ sf::Mouse::getPosition(*m_window) });
		break;
	}
}


void CoordinatePlane::renderContour(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight,
	std::function<float(float, float)> func, float threshold) {
	sf::VertexArray contour{ sf::Lines };
	for (float x = topLeft.x; x < bottomRight.x; x += resolution) {
		for (float y = topLeft.y; y < bottomRight.y; y += resolution) {
			// Stores the values of the function at the corners (and middle) of the square
			std::vector<float> values = {
				func(x, -y),
				func(x + resolution, -y),
				func(x + resolution, -(y + resolution)),
				func(x, -(y + resolution)),
				func(x + resolution / 2.0f, -(y - resolution / 2.0f))
			};
			// Stores the vertex infromation of the square to be used later
			std::vector<sf::Vector2f> vertecies = {
				{ x, y },
				{ x + resolution, y },
				{ x + resolution, y + resolution },
				{ x, y + resolution},
				{ x, y }
			};

			// Calculates the approximate points at which the contour may pass
			std::vector<sf::Vector2f> linePoints;
			for (size_t i = 0; i < 4; ++i) {
				float a = values.at(i);
				float b = values.at((i + 1) % 4); // to go back to the value 0 and not 4

				if ((a < threshold) == (b < threshold)) continue;
				
				float xt = vertecies.at(i).x + ((threshold - a) / (b - a)) * (vertecies.at(i + 1).x - vertecies.at(i).x);
				float yt = vertecies.at(i).y + ((threshold - a) / (b - a)) * (vertecies.at(i + 1).y - vertecies.at(i).y);
				linePoints.push_back({ xt, yt });
			}

			// Determines how the points are going to be connected in the contour
			if (linePoints.size() == 2) {
				contour.append({ linePoints.at(0) });
				contour.append({ linePoints.at(1) });
			} else if (linePoints.size() == 4) {
				if (values.at(0) == values.at(4)) {
					contour.append({ linePoints.at(0) });
					contour.append({ linePoints.at(1) });
					contour.append({ linePoints.at(2) });
					contour.append({ linePoints.at(3) });
				} else {
					contour.append({ linePoints.at(0) });
					contour.append({ linePoints.at(3) });
					contour.append({ linePoints.at(1) });
					contour.append({ linePoints.at(2) });
				}
			}
		}
	}
	m_window->draw(contour);
}


void CoordinatePlane::renderAxis(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight) {
	sf::VertexArray axis{ sf::Quads };
	if ((topLeft.x < 0.0f) != (bottomRight.x < 0.0f)) {
		axis.append({ { -2.0f * m_totalZoom, topLeft.y }, sf::Color::Black });
		axis.append({ { +2.0f * m_totalZoom, topLeft.y }, sf::Color::Black });
		axis.append({ { +2.0f * m_totalZoom, bottomRight.y }, sf::Color::Black });
		axis.append({ { -2.0f * m_totalZoom, bottomRight.y }, sf::Color::Black });
	} if ((topLeft.y < 0.0f) != (bottomRight.y < 0.0f)) {
		axis.append({ { topLeft.x    , -2.0f * m_totalZoom}, sf::Color::Black });
		axis.append({ { topLeft.x    , +2.0f * m_totalZoom}, sf::Color::Black });
		axis.append({ { bottomRight.x, +2.0f * m_totalZoom}, sf::Color::Black });
		axis.append({ { bottomRight.x, -2.0f * m_totalZoom}, sf::Color::Black });
	}
	m_window->draw(axis);
}


void CoordinatePlane::renderDots(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight,
	std::function<float(float, float)> func, float threshold) {
	sf::VertexArray dots{ sf::Quads };
	for (float i = topLeft.x; i <= bottomRight.x; i += resolution) {
		for (float j = topLeft.y; j <= bottomRight.y; j += resolution) {
			sf::Color color = (func(i, -j) < threshold) ? sf::Color::Green : sf::Color::Red;
			dots.append({ { i - 0.015f, j }, color });
			dots.append({ { i, j + 0.015f }, color });
			dots.append({ { i + 0.015f, j }, color });
			dots.append({ { i, j - 0.015f }, color });
		}
	}
	m_window->draw(dots);
}


void CoordinatePlane::renderGrid(const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight) {
	sf::VertexArray grid{ sf::Lines };
	for (float i = topLeft.x; i <= bottomRight.x; i += resolution) {
		grid.append({ { i, topLeft.y }, sf::Color::Black });
		grid.append({ { i, bottomRight.y }, sf::Color::Black });
	}
	for (float j = topLeft.y; j <= bottomRight.y; j += resolution) {
		grid.append({ { topLeft.x, j }, sf::Color::Black });
		grid.append({ {bottomRight.x, j}, sf::Color::Black });
	}
	m_window->draw(grid);
}


void CoordinatePlane::render(std::function<float(float, float)> func, float threshold) {
	// Calculates the top left and bottom right of the view
	sf::View view = m_window->getView();
	sf::Vector2f topLeft = view.getCenter() - view.getSize() / 2.0f;
	sf::Vector2f bottomRight = view.getCenter() + view.getSize() / 2.0f;

	// Modular arithmetic to determine closest multiple of cellSize
	sf::Vector2f topLeftOffset, bottomRightOffset;
	topLeftOffset.x = mod(topLeft.x, resolution);
	topLeftOffset.y = mod(topLeft.y, resolution);
	bottomRightOffset.x = mod(bottomRight.x, resolution);
	bottomRightOffset.y = mod(bottomRight.x, resolution);

	// Gets the start and end positions, these are what we wanted
	topLeft -= topLeftOffset;
	bottomRight -= bottomRightOffset - sf::Vector2f{ resolution, resolution };

	// Renders based on the options selected
	if (showGrid) {
		renderGrid(topLeft, bottomRight);
	} if (showAxis) {
		renderAxis(topLeft, bottomRight);
	} if (showDots) {
		renderDots(topLeft, bottomRight, func, threshold);
	} if (showContour) {
		renderContour(topLeft, bottomRight, func, threshold);
	}
}
