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

typedef struct C2S {
    int op;
    char object[10];
    int from;
    int to;
    int action;
} C2S;

typedef struct S2C {
    int op;
    int client;
    char object[10];
    int from;
    int to;
    int action;  // 0-slicing
    int complete;  // order complete or customer leaved
    Order orders[15];  // new orders
} S2C;

#ifdef __cplusplus
}
#endif

#endif