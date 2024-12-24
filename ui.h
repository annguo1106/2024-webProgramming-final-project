#ifndef UI_H
#define UI_H

#ifdef __cplusplus
#include "config.h"
extern "C" {
#endif

extern int ui_running;

void add_msg(S2C servInst);
void run_ui();
void user_input(int x, int y, char* obj);

#ifdef __cplusplus
}
#endif

#endif
