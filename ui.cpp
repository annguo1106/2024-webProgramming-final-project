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
    // std::lock_guard<std::mutex> lock(msg_mutex);
    msg = servInst;
    // msg_queue.push(std::string(message));
} 

int def_obj (string obj, int x) {
    if (obj == "l0") return 20;  // on chop board
    if (obj == "l1" && 210 <= x && x <= 340) return 20;  // on chop board
    if (obj == "l1" && 118 <= x && x <= 184) return 16;  // on assemb board
    if (obj == "l1") return 16;
    if (obj == "t0") return 19;
    if (obj == "t1" && 210 <= x && x <= 340) return 18;  // on chop board
    if (obj == "t1" && 118 <= x && x <= 184) return 21;  // on assemb board
    if (obj == "t1") return 21;
    if (obj == "m") return 17;
    if (obj == "b") return 4;
    if (obj == "c") return 12;
    if (obj == "b1111") return 0;
    if (obj == "b1011") return 1;
    if (obj == "b0111") return 3;
    if (obj == "b0011") return 2;
    if (obj == "i0") return 13;
    if (obj == "i1") return 14;
    if (obj == "i2") return 15;
    return 50;
    // if (obj == "empty") return -1;
}


void move (int client, float &x, float &y, float &tox, float &toy, float dt) {
    if (x < tox) {
        x += 100 * dt;
        if (x > tox) x = tox;
    }
    else if (x > tox) {
        x -= 100 * dt;
        if (x < tox) x = tox;
    }
    if (x == tox && y < toy) {
        y += 100 * dt;
        if (y > toy) y = toy;
    }
    else if (x == tox && y > toy) {
        y -= 100 * dt;
        if (y < toy) y = toy;
    }
}

int parse_order (string order) {
    if (order == "b1111") {
        // c1
        return 5;
    }
    else if (order == "b1011") {
        // c2
        return 6;
    }
    else if (order == "b0011") {
        // c3
        return 7;
    }
    else if (order == "b0111") {
        // c4
        return 8;
    }
    else if (order == "i0") {
        // c5
        return 9;
    }
    else if (order == "i1") {
        // c6
        return 10;
    }
    else if (order == "i2") {
        // c7
        return 11;
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
    std::string displayed_message;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition(10, 10);
    // set timer text 1
    sf::Text timetext1, timetext2;
    timetext1.setFont(font);
    timetext1.setCharacterSize(20);
    timetext1.setFillColor(sf::Color::White);
    timetext1.setPosition(183, 148);
    int time1[10];
    // set timer text 2
    timetext2.setFont(font);
    timetext2.setCharacterSize(20);
    timetext2.setFillColor(sf::Color::White);
    timetext2.setPosition(748, 148);
    int time2[10];

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
    vector<bool> showSp1(22, false);
    vector<bool> showSp2(22, false);
    vector<sf::Texture> tx(22);
    vector<sf::Sprite> sp1(22);
    for (int i = 0; i < 22; i++) {
        if (!tx[i].loadFromFile(std::string(spConf[i].path))) {
            std::cerr << "Error: Could not load background image.\n";
            return;
        }
        sp1[i].setTexture(tx[i]);
        sp1[i].setPosition(spConf[i].x, spConf[i].y);
        sp1[i].setScale(spConf[i].scaleX, spConf[i].scaleY);
    }

    // object sprite for cli2
    vector<sf::Sprite> sp2(22);
    for (int i = 0; i < 22; i++) {
        sp2[i].setTexture(tx[i]);
        sp2[i].setPosition(spConf[i].x + 600, spConf[i].y);
        sp2[i].setScale(spConf[i].scaleX, spConf[i].scaleY);
    }
    
    bool cli1moving = false;
    // bool cli2moving = false;
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
    float cli1x = 228, cli1y = 470;
    float cli2x = 870, cli2y = 470;
    float cli1tox = 228, cli1toy = 470;
    float cli2tox = 870, cli2toy = 470;
    bool cli1take = 0, cli2take = 0;
    char handobj[50];
    cli1.setPosition(int(cli1x), int(cli1y));
    cli2.setPosition(int(cli2x), int(cli2y));

    // customer sp
    vector<sf::Sprite> customer1(11);
    vector<sf::Sprite> customer2(11);
    int custx = 130, custy = 160;
    int stcust1 = -1, stcust2 = -1;
    int mvx1 = 600, mvx2 = 1200;
    int row1 = 0, row2 = 0;
    int leav1x = 0;
    int leav2x = 600;

    // set clock
    sf::Clock clock;
    sf::Clock clock1, clock2;

    msg.op = -1;
    user_input(1, 1, "50");
    while (ui_running && window.isOpen()) {
        // update clock
        sf::Time deltaTime = clock.restart();
        float dt = deltaTime.asSeconds();
        // update events
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
                    // snprintf(str, sizeof(str), "click at %d %d\n", x, y);
                    // displayed_message = str;
                    if (!cli1take) strcpy(handobj, "0");
                    if (!cli1moving) user_input(x, y, handobj);
                }
            }
        }

        // Retrieve messages from the queue
        {
            // std::lock_guard<std::mutex> lock(msg_mutex);
            // printf("retrieve msg\n");
            if (msg.op != -1) {
                // char str[50];
                // snprintf(str, sizeof(str), "%d", msg.op);
                // // printf("received op = %d\n", msg.op);
                // displayed_message = str;
                int obj;
                if (msg.op == 10) {  // update object status
                    obj = def_obj(msg.object, msg.toX); 
                    printf("now obj is %d %s, location: %d, tox: %d\n", obj, msg.object, msg.location, msg.toX);
                    if (msg.location) {  // not hand
                        if (strcmp(msg.object, "empty") != 0) {
                            // printf("obj is %d\n", obj);
                            if (msg.client == 0) {
                                showSp1[obj] = 1;
                                printf("cli1 show %d\n", obj);
                            }
                            else {
                                showSp2[obj] = 1;
                                printf("cli2 show %d\n", obj);
                            }
                        }
                        else {  // clear obj
                            // printf("clear obj\n");
                            // snprintf(str, sizeof(str), "clear obj %d\n", obj);
                            // displayed_message = str;
                            if (msg.client == 0) {
                                if (msg.toX <= 195) {  // clear on assemb
                                    printf("clear assemb\n");
                                    for (int i = 0; i < 22; i++)
                                        if (spConf[i].x == 123) showSp1[i] = 0;
                                }
                                else {  // clear on chop
                                    printf("clear chop\n");
                                    for (int i = 0; i < 22; i++) {
                                        if (spConf[i].x == 240) showSp1[i] = 0;
                                    }
                                }
                            }
                            else {
                                printf("clear cli2\n");
                                if (msg.toX <= 195) {  // clear on assemb
                                    printf("clear assemb\n");
                                    for (int i = 0; i < 22; i++)
                                        if (spConf[i].x == 123) showSp2[i] = 0;
                                }
                                else {  // clear on chop
                                    printf("clear chop\n");
                                    for (int i = 0; i < 22; i++) {
                                        if (spConf[i].x == 240) showSp2[i] = 0;
                                    }
                                }
                            }
                        }
                    }
                    else {  // update hand obj
                        printf("update hand\n");
                        if (obj == 50) {  // clear
                            if (msg.client == 0) {
                                cli1take = 0; 
                            }
                            else {
                                cli2take = 0;
                            }
                        }
                        else {
                            printf("obj: %d\n", obj);
                            if (msg.client == 0) {
                                cli1take = 1;
                                strcpy(handobj, msg.object);
                                // obj = def_obj(msg.object, msg.toX); 
                                cli1hand.setTexture(tx[obj]);
                                cli1hand.setScale(spConf[obj].scaleX, spConf[obj].scaleY);
                            }
                            else {
                                cli2take = 1;
                                // strcpy(handobj, msg.object);
                                // obj = def_obj(msg.object, msg.toX); 
                                cli2hand.setTexture(tx[obj]);
                                cli2hand.setScale(spConf[obj].scaleX, spConf[obj].scaleY);
                            }
                        }
                    }
                }
                else if (msg.op == 11) { // move character
                    if (msg.client == 0) {
                        cli1tox = msg.toX - 58;
                        cli1toy = msg.toY - 75;
                        
                        if (cli1tox > 380) cli1tox = 380;
                        if (cli1tox < 90) cli1tox = 90;
                        if (cli1toy > 530) cli1toy = 530;
                        if (cli1toy < 355) cli1toy = 355;
                        cli1moving = 1;
                    }
                    else {
                        cli2tox = msg.toX + 600 - 58;
                        cli2toy = msg.toY - 75;
                        if (cli2tox > 980) cli2tox = 980; 
                        if (cli2tox < 690) cli2tox = 690;
                        if (cli2toy > 530) cli2toy = 530;
                        if (cli2toy < 335) cli2toy = 335;
                        // printf("cli2 move from %.1f %.1f to %.1f %.1f\n", cli2x, cli2y, cli2tox, cli2toy);
                    }
                }
                else if (msg.op == 12) {  // update customer
                    if (msg.client == 0) {
                        stcust1 ++;
                        leav1x = custx;
                        clock1.restart();
                    }
                    else {
                        stcust2 ++;
                        leav2x = custx + 600;
                        clock2.restart();
                    }
                }
                else if (msg.op == 13) {  // render new order
                    // printf("do op = 13\n");
                    if (msg.client == 0) {
                        stcust1 = 0;
                        mvx1 = 600;
                        row1 = 0;
                        for (int i = 0; i < 5; i++) {
                            customer1[i].setTexture(tx[parse_order(msg.orders[i])]);
                            time1[i] = msg.time[i];
                        }
                        clock1.restart();
                    }
                    else {
                        // printf("set up cust2\n");
                        stcust2 = 0;
                        mvx2 = 1200;
                        row2 = 0;
                        for (int i = 0; i < 5; i++) {
                            customer2[i].setTexture(tx[parse_order(msg.orders[i])]);             
                            time2[i] = msg.time[i];
                        }
                        clock2.restart();
                    }
                }
                // else if (msg.op == 97) {
                //     char str[500];
                //     if (msg.action == 0) { // win
                //         strcpy(str, "you win!\n");
                //         displayed_message = str;
                //     }
                //     else {
                //         strcpy(str, "you lose!\n");
                //         displayed_message = str;
                //     }
                // }
                else if (msg.op == 98 || msg.op == 98) {
                    char str[500];
                    strcpy(str, msg.message);
                    displayed_message = str;
                }
                // displayed_message = msg.op;
                msg.op = -1;
            }
        }


        cli1.setPosition(int(cli1x), int(cli1y));
        cli2.setPosition(int(cli2x), int(cli2y));
        cli1hand.setPosition(int(cli1x), int(cli1y));
        cli2hand.setPosition(int(cli2x), int(cli2y));
        move(0, cli1x, cli1y, cli1tox, cli1toy, dt);
        move(1, cli2x, cli2y, cli2tox, cli2toy, dt);
        if (cli1x == cli1tox && cli1y == cli1toy) cli1moving = 0;
        // if (cli2x == cli2tox && cli2y == cli2toy) cli2moving = 0;
        
        // if (cli1x < 100 || cli1x > 390) cli1x -= 5;

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
        for (int i = 0; i < 22; i++) {
            if (showSp1[i]) {
                window.draw(sp1[i]);
            }
        }
        for (int i = 0; i < 22; i++) {
            if (showSp2[i]) {
                window.draw(sp2[i]);
            }
        }
        // draw custoer
        int tmpx = mvx1, tmpy = custy;
        if (stcust1 != -1) {
            // printf("mvx = %d\n", mvx1);
            for (int i = stcust1; i < 5; i++) {
                // printf("x, y = %d %d\n", tmpx, tmpy);
                if (tmpx > 600) continue;
                customer1[i].setPosition(tmpx, tmpy);
                window.draw(customer1[i]);
                tmpx += 80;                
            }
        }
        if (mvx1 > custx) mvx1 -= 5 * dt;
        tmpy = custy;
        if (stcust2 != -1) {
            // printf("cust2 is not -1\n");
            tmpx = mvx2;
            for (int i = stcust2; i < 5; i++) {
                if (tmpx > 1200){
                    // printf("continue");
                    continue;
                }
                customer2[i].setPosition(tmpx, tmpy);
                window.draw(customer2[i]);
                // printf("draw customer2\n");
                tmpx += 80;
            }
        } 
        if (mvx2 > custx + 600) mvx2 -= 5 * dt;
        if (leav1x > 0 && stcust1-1 >= 0) {
            // printf("move customer1\n");
            customer1[stcust1-1].setPosition(leav1x, custy);
            window.draw(customer1[stcust1-1]);
            leav1x -= 5 * dt;
        }
        if (leav2x > 600 && stcust2-1 >= 0) {
            customer2[stcust2-1].setPosition(leav2x, custy);
            window.draw(customer2[stcust2-1]);
            leav2x -= 5 * dt;
        }
        // render timer1
        float rendertime;
        if (stcust1 != -1) {
            rendertime = time1[stcust1] - clock1.getElapsedTime().asSeconds();
            if (rendertime < 0) rendertime = 0;
            std::ostringstream oss;
            oss << std::fixed << rendertime;
            timetext1.setString(oss.str());
            window.draw(timetext1);
        }
        // render timer2
        if (stcust2 != -1) {
            rendertime = time2[stcust2] - clock2.getElapsedTime().asSeconds();
            if (rendertime < 0) rendertime = 0;
            std::ostringstream oss;
            oss << std::fixed << rendertime;
            timetext2.setString(oss.str());
            window.draw(timetext2);
        }
        if (stcust1 == 5) stcust1 = -1;
        if (stcust2 == 5) stcust2 = -1;

        // Render messages
        float y_offset = 10.0f;
        text.setString(displayed_message);
        window.draw(text);
        
        
        window.display();

        // sf::sleep(sf::milliseconds(10));
    }
}



// testcase
/*
10 1 130 687 l0 1

*/