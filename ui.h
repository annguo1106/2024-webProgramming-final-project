#ifndef UI_H
#define UI_H

# ifdef __cplusplus
extern "C" {
#endif

extern int ui_running;

void add_msg (const char* msg);
void run_ui ();
void user_input(int x, int y);

# ifdef __cplusplus
}
# endif

# endif
