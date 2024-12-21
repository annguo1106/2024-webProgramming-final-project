// ui.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>

// Game constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const sf::Vector2f ITEM_SIZE(80.f, 80.f);

struct GameItem {
    sf::RectangleShape shape;
    std::string name;
    bool isDragging = false;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game Screen");

    // Font setup
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
        return -1;
    }

    // Text setup
    sf::Text orderText("Take Orders", font, 24);
    orderText.setPosition(300, 20);
    orderText.setFillColor(sf::Color::White);

    // Items setup
    std::vector<GameItem> items;
    std::vector<std::string> itemNames = {"lettuce", "tomato", "meat", "bread", "cone", "1st flavor", "2st flavor"};
    std::vector<sf::Color> itemColors = {sf::Color::Green, sf::Color::Red, sf::Color(139, 69, 19), sf::Color::Yellow, sf::Color(255, 223, 186), sf::Color(255, 228, 196), sf::Color(210, 105, 30)};

    for (size_t i = 0; i < itemNames.size(); ++i) {
        GameItem item;
        item.shape.setSize(ITEM_SIZE);
        item.shape.setFillColor(itemColors[i]);
        item.shape.setPosition(50.f + (i % 4) * 100.f, 100.f + (i / 4) * 100.f);
        item.name = itemNames[i];
        items.push_back(item);
    }

    // Tool areas
    sf::RectangleShape assemblyArea(sf::Vector2f(120.f, 120.f));
    assemblyArea.setPosition(300.f, 400.f);
    assemblyArea.setFillColor(sf::Color::Blue);

    sf::RectangleShape choppingBoard(sf::Vector2f(120.f, 120.f));
    choppingBoard.setPosition(450.f, 400.f);
    choppingBoard.setFillColor(sf::Color(139, 69, 19));

    sf::RectangleShape garbage(sf::Vector2f(120.f, 120.f));
    garbage.setPosition(600.f, 400.f);
    garbage.setFillColor(sf::Color::Black);

    // Drag-and-drop logic
    sf::Vector2f dragOffset;
    GameItem* draggedItem = nullptr;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                for (auto& item : items) {
                    if (item.shape.getGlobalBounds().contains(mousePos)) {
                        item.isDragging = true;
                        dragOffset = mousePos - item.shape.getPosition();
                        draggedItem = &item;
                        break;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (draggedItem) {
                    draggedItem->isDragging = false;
                    draggedItem = nullptr;
                }
            }

            if (event.type == sf::Event::MouseMoved && draggedItem) {
                sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
                draggedItem->shape.setPosition(mousePos - dragOffset);
            }
        }

        // Clear and draw everything
        window.clear();

        window.draw(orderText);
        for (const auto& item : items) {
            window.draw(item.shape);
        }
        window.draw(assemblyArea);
        window.draw(choppingBoard);
        window.draw(garbage);

        window.display();
    }

    return 0;
}
