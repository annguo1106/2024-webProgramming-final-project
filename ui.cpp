# include <SFML/Graphics.hpp>
# include <SFML/Window.hpp>
# include <SFML/System.hpp>
# include <mutex>
# include <bits/stdc++.h>
# include "ui.h"
# include "config.h"
using namespace std;

int ui_running = 1;
std::mutex msg_mutex;
// std::queue<std::string> msg_queue;
S2C msg;

void add_msg(S2C servInst) {
    std::lock_guard<std::mutex> lock(msg_mutex);
    msg = servInst;
    // msg_queue.push(std::string(message));
}

void run_ui() {
    sf::RenderWindow window(sf::VideoMode(1200, 765), "Cooking Master");
    window.setVerticalSyncEnabled(false);

    if (!window.isOpen()) {
        fprintf(stderr, "Failed to create the SFML window!\n");
        return;
    }
    
    // set text
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        fprintf(stderr, "Error: Could not load font file (arial.ttf)\n");
        ui_running = 0;
        return; // Font file not found
    }

    // set sprite
    sf::Texture bg1, bg2;
    if (!bg1.loadFromFile("src/kitchen.png") || !bg2.loadFromFile("src/kitchen.png")) {
        std::cerr << "Error: Could not load background image.\n";
        return;
    }

    // bgSprite
    sf::Sprite bgSprite1, bgSprite2;
    bgSprite1.setTexture(bg1);
    bgSprite2.setTexture(bg2);
    bgSprite2.setPosition(600, 0);
    
    // object sprite for cli1
    vector<sf::Texture> tx(20);
    vector<sf::Sprite> sp1(20);
    printf("setting cli1 sprite\n");
    for (int i = 0; i < 20; i++) {
        printf("setting sprite %s\n", spConf[i].name);
        printf("hello?\n");
        if (!tx[i].loadFromFile(std::string(spConf[i].path))) {
            std::cerr << "Error: Could not load background image.\n";
            return;
        }
        printf("load texture success\n");
        sp1[i].setTexture(tx[i]);
        printf("set texture\n");
        sp1[i].setPosition(spConf[i].x, spConf[i].y);
        printf("set pos\n");
        sp1[i].setScale(spConf[i].scaleX, spConf[i].scaleY);
        printf("set scale\n");
    }

    // object sprite for cli2
    vector<sf::Sprite> sp2(20);
    printf("setting cli2 sprite\n");
    for (int i = 0; i < 20; i++) {
        printf("setting sprite %s\n", spConf[i].name);
        sp1[i].setTexture(tx[i]);
        sp1[i].setPosition(spConf[i].x, spConf[i].y);
        sp1[i].setScale(spConf[i].scaleX, spConf[i].scaleY);
    }
    
    bool ismoving = false;
    // client sp
    sf::Sprite cli1, cli2, cli1hand, cli2hand;
    sf::Texture cli1tex, cli2tex;
    if (!cli1tex.loadFromFile("src/c.png")) {
        std::cerr << "Error: Could not load background image.\n";
        return;
    }
    if (!cli2tex.loadFromFile("src/c.png")) {
        std::cerr << "Error: Could not load background image.\n";
        return;
    }
    cli1.setTexture(cli1tex);
    cli2.setTexture(cli2tex);
    int cli1x = 228, cli1y = 470;
    int cli2x = 870, cli2y = 470;
    int cli1tox = 228, cli1toy = 470;
    int cli2tox = 870, cli2toy = 470;
    bool cli1take = 0, cli2take = 0;
    cli1.setPosition(cli1x, cli1y);
    cli2.setPosition(cli2x, cli2y);



    vector<bool> showSp1(20, false);
    vector<bool> showSp2(20, false);

    msg.op = -1;
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
            if (msg.op != -1) {
                char str[50];
                snprintf(str, sizeof(str), "%d", msg.op);
                displayed_message = str;
                if (msg.op == 10) {  // update object status

                }
                else if (msg.op == 11) { // move character

                }
                else if (msg.op == 12) {  // update customer

                }
                else if (msg.op == 13) {  // render new order

                }
                // displayed_message = msg.op;
                msg.op = -1;
            }
        }

        // Render messages
        window.clear(sf::Color::Black);
        // draw bg
        window.draw(bgSprite1);
        window.draw(bgSprite2);
        // draw character
        window.draw(cli1);
        window.draw(cli2);
        if (cli1take) window.draw(cli1hand);
        if (cli2take) window.draw(cli2hand);
        // draw objects
        for (int i = 0; i < 20; i++) {
            if (showSp1[i]) {
                window.draw(sp1[i]);
            }
        }
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
