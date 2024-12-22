#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// burger order
typedef struct Burger {
    int lettuce;
    int tomato;
    int meat;
    int bread;
} Burger;

// icecream order
typedef struct IceCream {
    int cone;
    int f1;
    int f2;
} IceCream;

typedef struct Order {
    int op;  // 0 for icecream, 1 for hamburger
    IceCream icecream;
    Burger burger;
} Order;

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
    Order orders[15];  // new orders
} S2C;

typedef struct Sprite {
    char name[20];
    int x, y;
    float scaleX, scaleY;
    char path[50];
} Sprite;

const Sprite spConf[] = {
    {"b1", 10, 20, 0.5, 0.5, "src/b1.png"},
    {"b2", 10, 20, 0.5, 0.5, "src/b2.png"},
    {"b3", 10, 20, 0.5, 0.5, "src/b3.png"},
    {"b4", 10, 20, 0.5, 0.5, "src/b4.png"},
    {"bread", 10, 20, 0.5, 0.5, "src/bread.png"},
    {"c1", 10, 20, 0.5, 0.5, "src/c1.png"},
    {"c2", 10, 20, 0.5, 0.5, "src/c2.png"},
    {"c3", 10, 20, 0.5, 0.5, "src/c3.png"},
    {"c4", 10, 20, 0.5, 0.5, "src/c4.png"},
    {"c5", 10, 20, 0.5, 0.5, "src/c5.png"},
    {"c6", 10, 20, 0.5, 0.5, "src/c6.png"},
    {"c7", 10, 20, 0.5, 0.5, "src/c7.png"},
    {"cone", 10, 20, 0.5, 0.5, "src/cone.png"},
    {"i1", 10, 20, 0.5, 0.5, "src/i1.png"},
    {"i2", 10, 20, 0.5, 0.5, "src/i2.png"},
    {"i3", 10, 20, 0.5, 0.5, "src/i3.png"},
    {"let", 10, 20, 0.5, 0.5, "src/let.png"},
    {"meat", 10, 20, 0.5, 0.5, "src/meat.png"},
    {"s_t", 10, 20, 0.5, 0.5, "src/s_t.png"},
    {"tomato", 10, 20, 0.5, 0.5, "src/tomato.png"},
};

#ifdef __cplusplus
}
#endif

#endif