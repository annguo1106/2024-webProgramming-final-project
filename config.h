#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// burger order
// typedef struct Burger {
//     int lettuce;
//     int tomato;
//     int meat;
//     int bread;
// } Burger;

// icecream order
// typedef struct IceCream {
//     int cone;
//     int f1;
//     int f2;
// } IceCream;

// typedef struct Order {
//     int op;  // 0 for icecream, 1 for hamburger
    // IceCream icecream;
    // Burger burger;
// } Order;

// [object] [from] [to] [to location] [action]
typedef struct C2S {
    char object[10];
    int fromX, fromY;
    int toX, toY;
    int toLoc;
    int action;
} C2S;

typedef struct S2C {
    int op;
    int client;
    char object[10];
    int fromX;
    int fromY;
    int toX;
    int toY;
    int location;
    int action;  // 0-slicing
    int complete;  // order complete or customer leaved
    // Order orders[15];  // new orders
    char orders[20][50];
    int time[20];
    char message[500];
} S2C;

typedef struct Sprite {
    char name[20];
    int x, y;
    float scaleX, scaleY;
    char path[50];
} Sprite;

const Sprite spConf[] = {
    {"b1", 123, 687, 0.5, 0.5, "src/b1.png"},  // 0
    {"b2", 123, 687, 0.5, 0.5, "src/b2.png"},  // 1
    {"b3", 123, 687, 0.5, 0.5, "src/b3.png"},  // 2
    {"b4", 123, 687, 0.5, 0.5, "src/b4.png"},  // 3
    {"bread", 123, 687, 0.5, 0.5, "src/bread.png"},  // 4
    {"c1", 164, 130, 1, 1, "src/c1.png"},  // 5
    {"c2", 164, 130, 1, 1, "src/c2.png"},  // 6
    {"c3", 164, 130, 1, 1, "src/c3.png"},  // 7
    {"c4", 164, 130, 1, 1, "src/c4.png"},  // 8
    {"c5", 164, 130, 1, 1, "src/c5.png"},  // 9
    {"c6", 164, 130, 1, 1, "src/c6.png"},  // 10
    {"c7", 164, 130, 1, 1, "src/c7.png"},  // 11
    {"cone", 10, 20, 0.5, 0.5, "src/cone.png"},  // 12
    {"i1", 10, 20, 0.5, 0.5, "src/i2.png"},  // 13
    {"i2", 10, 20, 0.5, 0.5, "src/i1.png"},  // 14
    {"i3", 10, 20, 0.5, 0.5, "src/i3.png"},  // 15
    {"let", 123, 687, 0.5, 0.5, "src/let.png"},  // 16 on assemb board
    {"meat", 123, 687, 0.5, 0.5, "src/meat.png"},  // 17
    {"s_t", 240, 687, 0.5, 0.5, "src/s_t.png"},  // 18 on chop board
    {"tomato", 240, 687, 0.5, 0.5, "src/tomato.png"},  // 19
    {"let", 240, 687, 0.5, 0.5, "src/let.png"},  // 20 on shop board
    {"s_t", 123, 687, 0.5, 0.5, "src/s_t.png"},  // 21 on assemb board
};

#ifdef __cplusplus
}
#endif

#endif