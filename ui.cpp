# include <SFML/Graphics.hpp>
# include <SFML/Window.hpp>
# include <SFML/System.hpp>
# include <mutex>
# include <bits/stdc++.h>
# include "ui.h"

int ui_running = 1;
std::mutex msg_mutex;
// std::queue<std::string> msg_queue;
std::string msg = "-1";

void add_msg(const char* message) {
    std::lock_guard<std::mutex> lock(msg_mutex);
    msg = std::string(message);
    // msg_queue.push(std::string(message));
}

void run_ui() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Cooking Master");
    window.setVerticalSyncEnabled(false);

    if (!window.isOpen()) {
        fprintf(stderr, "Failed to create the SFML window!\n");
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        fprintf(stderr, "Error: Could not load font file (arial.ttf)\n");
        ui_running = 0;
        return; // Font file not found
    }

    // std::vector<std::string> displayed_messages;
    std::string displayed_message;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);

    while (ui_running && window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                ui_running = 0;
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;
                    char str[200];
                    // displayed_message = "mouse pressed\n";
                    snprintf(str, sizeof(str), "click at %d %d\n", x, y);
                    displayed_message = str;
                    user_input(x, y);
                }
            }
        }

        // Retrieve messages from the queue
        {
            std::lock_guard<std::mutex> lock(msg_mutex);
            if (msg != "-1") {
                displayed_message = msg;
                msg = "-1";
            }
            // while (!msg_queue.empty()) {
            //     displayed_messages.push_back(msg_queue.front());
            //     msg_queue.pop();
            // }
        }

        // Render messages
        window.clear(sf::Color::Black);
        float y_offset = 10.0f;
        // for (const auto& msg : displayed_messages) {
        text.setString(displayed_message);
        text.setPosition(10, y_offset);
        window.draw(text);
        y_offset += 30.0f; // Adjust for line spacing
        // }
        window.display();

        // sf::sleep(sf::milliseconds(10));
    }
}
