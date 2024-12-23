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

int def_obj (string obj) {
    if (obj == "l0") return 16;
    if (obj == "l1") return 16;
    if (obj == "t0") return 19;
    if (obj == "t1") return 18;
    if (obj == "m") return 17;
    if (obj == "b") return 4;
    if (obj == "c") return 12;
    if (obj == "b1111") return 0;
    if (obj == "b1011") return 1;
    if (obj == "b0111") return 2;
    if (obj == "b0011") return 3;
    if (obj == "i0") return 13;
    if (obj == "i1") return 14;
    if (obj == "i2") return 15;
    return 50;
    // if (obj == "empty") return -1;
}


void move (int &x, int &y, int &tox, int &toy) {
    if (x < tox) {
        x += 5;
        if (x > tox) x = tox;
    }
    else if (x > tox) {
        x -= 5;
        if (x < tox) x = tox;
    }
    if (x == tox && y < toy) {
        y += 5;
        if (y > toy) y = toy;
    }
    else if (x == tox && y > toy) {
        y -= 5;
        if (y < toy) y = toy;
    }
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
                int obj;
                if (msg.op == 10) {  // update object status
                    obj = def_obj(msg.object); 
                    printf("obj is %d %s\n", obj, msg.object);
                    printf("msg location %d\n", msg.location);
                    if (msg.location) {  // not hand
                        if (strcmp(msg.object, "empty") != 0) {
                            printf("hi\n");
                            if (msg.client == 1) showSp1[obj] = 1;
                            else showSp2[obj] = 1;
                        }
                        else {  // clear obj
                            printf("in clear sooooope\n");
                            printf("clear object %d at %d, %d", obj, msg.toX, msg.toY);
                            snprintf(str, sizeof(str), "clear obj %d", obj);
                            displayed_message = str;
                            if (msg.client == 1) {
                                if (msg.toX <= 195) {  // clear on assemb
                                    for (int i = 0; i < 20; i++)
                                        if (spConf[i].x == 240) showSp1[i] = 0;
                                }
                                else {  // clear on chop
                                    for (int i = 0; i < 20; i++) {
                                        if (spConf[i].x == 123) showSp1[i] = 0;
                                    }
                                }
                            }
                            else {
                                if (msg.toX <= 195) {  // clear on assemb
                                    for (int i = 0; i < 20; i++)
                                        if (spConf[i].x == 240) showSp2[i] = 0;
                                }
                                else {  // clear on chop
                                    for (int i = 0; i < 20; i++) {
                                        if (spConf[i].x == 123) showSp2[i] = 0;
                                    }
                                }
                            }
                        }
                    }
                    else {  // update hand obj
                        if (!obj) {  // clear
                            if (msg.client == 1) {
                                cli1take = 0; 
                            }
                            else {
                                cli2take = 0;
                            }
                        }
                        else {
                            if (msg.client == 1) {
                                cli1take = 1;
                                cli1hand.setTexture(tx[obj]);
                                cli1hand.setScale(spConf[obj].scaleX, spConf[obj].scaleY);
                            }
                            else {
                                cli2take = 1;
                                cli2hand.setTexture(tx[obj]);
                                cli2hand.setScale(spConf[obj].scaleX, spConf[obj].scaleY);
                            }
                        }
                    }
                }
                else if (msg.op == 11) { // move character
                    if (msg.client == 1) {
                        cli1tox = msg.toX;
                        cli1toy = msg.toY;
                    }
                    else {
                        cli2tox = msg.toX;
                        cli2toy = msg.toY;
                    }
                }
                else if (msg.op == 12) {  // update customer

                }
                else if (msg.op == 13) {  // render new order

                }
                // displayed_message = msg.op;
                msg.op = -1;
            }
        }

        cli1.setPosition(cli1x, cli1y);
        cli2.setPosition(cli2x, cli2y);
        cli1hand.setPosition(cli1x, cli1y);
        cli2hand.setPosition(cli2x, cli2y);
        move(cli1x, cli1y, cli1tox, cli1toy);
        move(cli2x, cli2y, cli2tox, cli2toy);

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



// testcase
/*
10 1 130 687 l0 1

*/