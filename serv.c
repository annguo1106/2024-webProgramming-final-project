#include	"unp.h"
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

// locations that holds objects
char *hand[] = {NULL, NULL, NULL};
char *chop[] = {NULL, NULL, NULL};
char *assem[] = {NULL, NULL, NULL};
bool components[2][4] = {0};
int assem_cnt[2] = {0};

// message to client buffer
char buffer[2][MAXLINE];
char *iptr[2], *optr[2];

// order settings
// order settings
int order_cnt[2] = {0, 0, 0};
char **orders[2];
bool start_cnt = 0;
bool start_cnt = 0;

// timer settings
timer_t timer_cus[2];
int sec_cus[2] = {100, 100, 0};
timer_t timer_chop[2];
int sec_chop = 5;

// game logic settings
int goal = 3;
int fin_cnt[2] = {0};

struct timerData{
    int player_id;
    char *task;
    int x, y;
};

void sig_chld(int signo){
	int status;
	pid_t pid;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0);
    return;
}

ssize_t safe_write(int fd, const void *buf, size_t count) {
    const char *buffer = (const char *)buf; // Cast buffer to char* for processing
    char temp_buffer[MAXLINE];             // Temporary buffer to hold the tokenized string
    const char delim[] = "\n";             // Delimiter for messages
    char *token;
    char *saveptr;                         // Save pointer for strtok_r
    ssize_t total_written = 0;             // Total bytes written
    ssize_t nwritten;

    // Ensure we don't modify the input buffer
    if (count >= MAXLINE) {
        fprintf(stderr, "Input buffer too large for MAXLINE\n");
        return -1;
    }
    strncpy(temp_buffer, buffer, count);   // Copy buffer to temporary buffer
    temp_buffer[count] = '\0';             // Null-terminate the buffer

    // Tokenize the string and write each message
    token = strtok_r(temp_buffer, delim, &saveptr);
    while (token != NULL) {
        size_t message_len = strlen(token) + 1; // Include the newline character

        // Append the newline to the token
        char message_with_newline[MAXLINE];
        snprintf(message_with_newline, sizeof(message_with_newline), "%s\n", token);

        // Write the message to the file descriptor
        while (message_len > 0) {
            nwritten = write(fd, message_with_newline, message_len);
            if (nwritten < 0) {
                if (errno == EINTR) {
                    printf("Write EINTR\n");
                    continue;
                }
                return -1;
            }
            printf("write: %s", message_with_newline);
            message_len -= nwritten;
            total_written += nwritten;
        }

        // Get the next token
        token = strtok_r(NULL, delim, &saveptr);
    }

    return total_written; // Return total bytes written
}

ssize_t safe_read(int fd, void *buf, size_t count) {
    ssize_t nread;
    
    while (1) {
        nread = read(fd, buf, count);
        if (nread >= 0) {
            return nread;
        }
        
        if (errno == EINTR) {
            continue;
        }
        
        return -1;
    }
}

void timer_handler(int signum, siginfo_t *info, void *context) {
    printf("\n- handler\n");
    struct timerData *data = (struct timerData *)info->si_value.sival_ptr;
    int player_id = data->player_id;
    char *task = data->task;
    int x = data->x;
    int y = data->y;

    if(player_id == 0 || player_id == 1){
        if(strcmp(task, "cus") == 0){
            mes12(player_id, 0, 0);
            mes12(player_id, 0, 1);
            order_cnt[player_id]--;
        }
        else if(strcmp(task, "chop") == 0){
            // put chopped lettuce on chopping board
            char *obj;
            if(strcmp(chop[player_id], "l0") == 0)
                obj = strdup("l1");
            else if(strcmp(chop[player_id], "t0") == 0)
                obj = strdup("t1");

            // remove unchopped
            mes10(player_id, x, y, "empty", 1, 0);
            mes10(player_id, x, y, "emtpy", 1, 1);
            // put chopped
            mes10(player_id, x, y, obj, 1, 0);
            mes10(player_id, x, y, obj, 1, 1);
            chop[player_id] = strdup(obj);
        }
        else{
            printf("handler error: task error\n");
        }
    }
    else{
        printf("handler error: player_id error\n");
    }

    printf("Timer expired for player ID: %d, task: %s, order_cnt: %d\n", player_id, task, order_cnt[player_id]);
}

timer_t create_timer(int seconds, int interval, int player_id, char *task, int x, int y) {
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;

    // Set up signal handler
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Configure the timer
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;

    struct timerData *data = malloc(sizeof(struct timerData));
    data->player_id = player_id;
    data->task = strdup(task);  // Duplicate the task string
    data->x = x;
    data->y = y;
    sev.sigev_value.sival_ptr = data;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set the timer
    its.it_value.tv_sec = seconds;  // Initial expiration
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = interval; // Interval
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    printf("PID %d: Timer created with initial %d seconds, interval %d seconds, player ID: %d\n", getpid(), seconds, interval, player_id);
    return timerid;
}

void update_timer(timer_t timerid, int new_seconds, int new_interval) {
    struct itimerspec its;

    // Set new expiration times
    its.it_value.tv_sec = new_seconds;   // New initial expiration
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = new_interval; // New interval
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    printf("Timer updated to initial expiration %d seconds and interval %d seconds\n", new_seconds, new_interval);
}

char **get_new_order(int count){
    const char *available[] = {"i0", "i1", "i2", "b1111", "b1011", "b0111", "b0011"};
    int num_avai = sizeof(available) / sizeof(available[0]);

    // Allocate memory for the array of string pointers
    char **orders = malloc(count * sizeof(char *));
    if (orders == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Loop to randomly select orders
    for (int i = 0; i < count; i++) {
        int random_index = rand() % num_avai;
        
        // Allocate memory for the order string and copy the selected string into it
        orders[i] = strdup(available[random_index]);
        // printf("order: %s\n", orders[i]);
        if (orders[i] == NULL) {
            perror("Failed to duplicate string");
            exit(EXIT_FAILURE);
        }
    }

    return orders;
}

void free_orders(char **orders, int count) {
    for (int i = 0; i < count; i++) {
        free(orders[i]);  // Free each individual string
    }
    free(orders);  // Free the array of string pointers
}

void mes10(int player_id, int x, int y, char *obj, int isNotHand, int sendto){
    char sendline[MAXLINE];
    bool isSelf = (player_id != sendto);
    snprintf(sendline, sizeof(sendline), "10 %d %d %d %s %d\n\0", isSelf, x, y, obj, isNotHand);
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    // printf("iptr: %d\n", iptr[sendto]);
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes10.\n");
    }

    // printf("iptr after adding: %d\n", iptr[sendto]);
}

void mes11(int player_id, int from_x, int from_y, int to_x, int to_y, int sendto){
    char sendline[MAXLINE];
    bool isSelf = (player_id != sendto);
    snprintf(sendline, sizeof(sendline), "11 %d %d %d %d %d %s\n\0", isSelf, from_x, from_y, to_x, to_y, hand[player_id]);
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes11.\n");
    }
}

void mes12(int player_id, int complete, int sendto){
    char sendline[MAXLINE];
    bool isSelf = (player_id != sendto);
    snprintf(sendline, sizeof(sendline), "12 %d %d\n\0", isSelf, complete);
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        // printf("cpy complete\n");
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes12.\n");
    }
}

void mes13(char **order, int player_id, int sendto){
    char sendline[MAXLINE], str_sec[10];
    bool isSelf = (player_id != sendto);
    bool isSelf = (player_id != sendto);
    snprintf(str_sec, sizeof(str_sec), "%d", sec_cus[player_id]);
    snprintf(sendline, sizeof(sendline), "13 %d", isSelf);
    snprintf(sendline, sizeof(sendline), "13 %d", isSelf);
    for(int i=0; i<5; i++){
        strcat(sendline, " ");
        strcat(sendline, order[i]);
        strcat(sendline, " ");
        strcat(sendline, str_sec);
    }
    strcat(sendline, "\n\0");
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        // printf("cpy complete\n");
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes13.\n");
    }
}

void mes14(int player_id, int sendto){
    char sendline[MAXLINE];
    bool isSelf = (player_id != sendto);
    snprintf(sendline, sizeof(sendline), "14 %d\n\0", isSelf);
    printf("send to connfd[%d]: %s", sendto, sendline);

    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        // printf("cpy complete\n");
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes14.\n");
    }
}

void mes98(bool win, int sendto){
    char sendline[5];
    snprintf(sendline, sizeof(sendline), "98 %d\n\0", win);
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes98.\n");
    }
}

void mes99(int sendto){
    char sendline[5];
    snprintf(sendline, sizeof(sendline), "99\n\0");
    printf("send to connfd[%d]: %s", sendto, sendline);
    size_t len = strlen(sendline);

    // Check if there is enough space in the buffer
    if (iptr[sendto] + len < &buffer[sendto][MAXLINE]) {
        memcpy(iptr[sendto], sendline, len);
        // printf("cpy complete\n");
        iptr[sendto] += len;
    }
    else {
        fprintf(stderr, "Error: Not enough space in the buffer to add mes99.\n");
    }
}

void assem_counter(char* obj, int player_id, int from_x, int from_y, int to_x, int to_y){
    // check if obj can be put on the assemble counter

    if(strcmp(assem[player_id], "") != 0){
        // there is something complete on the assemble counter
        printf("Assem error: assem full\n");
        mes14(player_id, player_id);
        return;
    }

    int obj_num = -1;
    if(strcmp(obj, "l1") == 0 && !components[player_id][0]){
        // no lettuce yet
        obj_num = 0;
    }
    else if(strcmp(obj, "t1") == 0 && !components[player_id][1]){
        // no tomato yet
        obj_num = 1;
    }
    else if(strcmp(obj, "m") == 0 && !components[player_id][2]){
        // no meat yet
        obj_num = 2;
    }
    else if(strcmp(obj, "b") == 0 && !components[player_id][3]){
        // no bread yet
        obj_num = 3;
    }

    if(obj_num != -1){
        // empty hands
        mes10(player_id, from_x, from_y, "empty", 0, 0);
        mes10(player_id, from_x, from_y, "empty", 0, 1);
        hand[player_id] = strdup("");
        // put on counter
        mes10(player_id, to_x, to_y, obj, 1, 0);
        mes10(player_id, to_x, to_y, obj, 1, 1);
        components[player_id][obj_num] = 1;
        printf("add component %d to counter\n", obj_num);
    }
    else{
        printf("Assem error: already present on counter\n");
        mes14(player_id, player_id);
    }
}

void assem_logic(int player_id, int x, int y){
    // check if the current components can be assembled
    // 0: lettuce, 1:tomato, 2: meat, 3: bread
    if(components[player_id][2] && components[player_id][3]){
        mes10(player_id, x, y, "empty", 1, 0);   // clean assemble counter
        mes10(player_id, x, y, "empty", 1, 1);

        if(components[player_id][0] && components[player_id][1]){
            // all
            mes10(player_id, x, y, "b1111", 1, 0);
            mes10(player_id, x, y, "b1111", 1, 1);
            assem[player_id] = strdup("b1111");
        }
        else if(components[player_id][0]){
            // lettuce + meat + bread
            mes10(player_id, x, y, "b1011", 1, 0);
            mes10(player_id, x, y, "b1011", 1, 1);
            assem[player_id] = strdup("b1011");
        }
        else if(components[player_id][1]){
            // tomato + meat + bread
            mes10(player_id, x, y, "b0111", 1, 0);
            mes10(player_id, x, y, "b0111", 1, 1);
            assem[player_id] = strdup("b0111");
        }
        else{
            // only meat + bread
            mes10(player_id, x, y, "b0011", 1, 0);
            mes10(player_id, x, y, "b0011", 1, 1);
            assem[player_id] = strdup("b0011");
        }
        
        // clear components
        for(int i=0; i<4; i++){
            components[player_id][i] = 0;
        }
        printf("hand[%d]: %s, assem: %s\n", player_id, hand[player_id], assem[player_id]);
    }
    else{
        // unable to assemble
        printf("Assem error: not enough components\n");
        mes14(player_id, player_id);
    }
}

void discard(int player_id, int x, int y){
    mes10(player_id, x, y, "empty", 0, 0);
    mes10(player_id, x, y, "empty", 0, 1);
    hand[player_id] = strdup("");
    printf("hand[%d]: %s\n", player_id, hand[player_id]);
}

void handle_message(char* recvline, int player_id){
    char obj[20];
    int from_x, from_y, to_x, to_y;
    int to_loc, action;
    printf("\n---- messsage -----\n");
    printf("[%d] recvline content: %s\n", player_id, recvline);
    
    if(sscanf(recvline, "%s %d %d %d %d %d %d", obj, &from_x, &from_y, &to_x, &to_y, &to_loc, &action) == 7){
        if(strcmp(obj, "50") == 0){
            if(start_cnt == 0){
                start_cnt = 1;
                return;
            }
            // send first order to player[player_id]
            mes13(orders[0], 0, 0);
            mes13(orders[1], 1, 1);
            mes13(orders[0], 0, 1);
            mes13(orders[1], 1, 0);

            //set customer timer for player[player_id]
            timer_cus[0] = create_timer(sec_cus[0], sec_cus[0], 0, "cus", 0, 0);
            timer_cus[1] = create_timer(sec_cus[1], sec_cus[1], 1, "cus", 0, 0);
        }
        else if(strcmp(obj, "l0") == 0 || strcmp(obj, "t0") == 0){
            // not chopped lettuce or tomato
            if(to_loc == 40 || to_loc == 41){
                // to hand
                if(strcmp(hand[player_id], "") == 0){
                    // empty hand
                    mes10(player_id, to_x, to_y, obj, 0, 0);
                    mes10(player_id, to_x, to_y, obj, 0, 1);
                    hand[player_id] = strdup(obj);
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else{
                    printf("hand occupied\n");
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 1){
                // to chopping board
                if(strcmp(hand[player_id], obj) == 0 && strcmp(chop[player_id], "") == 0){
                    // delete lettuce in hand
                    mes10(player_id, from_x, from_y, "empty", 0, 0);
                    mes10(player_id, from_x, from_y, "empty", 0, 1);
                    hand[player_id] = strdup("");
                    // put lettuce on chopping board
                    mes10(player_id, to_x, to_y, obj, 1, 0);
                    mes10(player_id, to_x, to_y, obj, 1, 1);
                    chop[player_id] = strdup(obj);
                    printf("hand[%d]: %s, chop: %s\n", player_id, hand[player_id], chop[player_id]);
                    // set timer
                    timer_chop[player_id] = create_timer(sec_chop, 0, player_id, "chop", to_x, to_y);
                }
                else{
                    printf("not fit: hand: %s, chop: %s\n", hand[player_id], chop[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], obj) == 0){
                    // delete obj in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "l1") == 0 || strcmp(obj, "t1") == 0){
            // chopped lettuce or tomato
            if(to_loc == 2){
                // hands to assemble counter
                if(strcmp(hand[player_id], obj) == 0){
                    assem_counter(obj, player_id, from_x, from_y, to_x, to_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], obj) == 0){
                    // delete obj in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "m") == 0 || strcmp(obj, "b") == 0){
            // meat or bread
            if(to_loc == 42 || to_loc == 43){
                // to hand
                if(strcmp(hand[player_id], "") == 0){
                    // empty hand
                    mes10(player_id, to_x, to_y, obj, 0, 0);
                    mes10(player_id, to_x, to_y, obj, 0, 1);
                    hand[player_id] = strdup(obj);
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else{
                    printf("hand occupied\n");
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 2){
                // hands to assemble counter
                if(strcmp(hand[player_id], obj) == 0){
                    assem_counter(obj, player_id, from_x, from_y, to_x, to_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], obj) == 0){
                    // delete obj in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "f0") == 0){
            if(to_loc == 44){
                // from origin to hand
                if(strcmp(hand[player_id], "c") == 0){
                    // cone in hand = 1st flavor + cone
                    mes10(player_id, to_x, to_y, "i0", 0, 0);
                    mes10(player_id, to_x, to_y, "i0", 0, 1);
                    hand[player_id] = strdup("i0");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else if(strcmp(hand[player_id], "i1") == 0){
                    // all flavor + cone
                    mes10(player_id, to_x, to_y, "i2", 0, 0);
                    mes10(player_id, to_x, to_y, "i2", 0, 1);
                    hand[player_id] = strdup("i2");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else{
                    printf("take cone first\n");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "f1") == 0){
            if(to_loc == 45){
                // from origin to hand
                if(strcmp(hand[player_id], "c") == 0){
                    // cone in hand = 2nd flavor + cone
                    mes10(player_id, to_x, to_y, "i1", 0, 0);
                    mes10(player_id, to_x, to_y, "i1", 0, 1);
                    hand[player_id] = strdup("i1");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else if(strcmp(hand[player_id], "i0") == 0){
                    // all flavor + cone
                    mes10(player_id, to_x, to_y, "i2", 0, 0);
                    mes10(player_id, to_x, to_y, "i2", 0, 1);
                    hand[player_id] = strdup("i2");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else{
                    printf("take cone first\n");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "c") == 0){
            if(to_loc == 46){
                // origin to hand
                if(strcmp(hand[player_id], "") == 0){
                    // nothing in hand
                    mes10(player_id, to_x, to_y, "c", 0, 0);
                    mes10(player_id, to_x, to_y, "c", 0, 1);

                    hand[player_id] = strdup("c");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                }
                else{
                    printf("hand occupied\n");
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], "c") == 0){
                    // delete cone in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "b1111") == 0 || strcmp(obj, "b1011") == 0 || strcmp(obj, "b0111") == 0 || strcmp(obj, "b0011") == 0){
            if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], obj) == 0){
                    // delete obj in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 5){
                // hand to customer
                int cur = 10 - order_cnt[player_id];
                if(strcmp(hand[player_id], obj) == 0 && strcmp(orders[player_id][cur], obj) == 0){
                    // reset timer
                    update_timer(timer_cus[player_id], sec_cus[player_id] + 2, sec_cus[player_id]);
                    // delete ice cream 1 in hand
                    mes10(player_id, from_x, from_y, "empty", 0, 0);
                    mes10(player_id, from_x, from_y, "empty", 0, 1);
                    hand[player_id] = strdup("");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                    // check if win
                    fin_cnt[player_id]++;
                    if(fin_cnt[player_id] == goal){
                        // game end
                        mes98(player_id, 1);
                        mes98(!player_id, 0);
                        return;
                    }
                    // complete order
                    order_cnt[player_id]--;
                    mes12(player_id, 1, 0);
                    mes12(player_id, 1, 1);
                }
                else{
                    printf("not fit: hand: %s, order: %s\n", hand[player_id], orders[player_id][cur]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "i0") == 0 || strcmp(obj, "i1") == 0 || strcmp(obj, "i2") == 0){
            if(to_loc == 3){
                // hand to trash
                if(strcmp(hand[player_id], obj) == 0){
                    // delete obj in hand
                    discard(player_id, from_x, from_y);
                }
                else{
                    printf("not fit: hand: %s\n", hand[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 5){
                // hand to customer
                int cur = 10 - order_cnt[player_id];
                if(strcmp(hand[player_id], obj) == 0 && strcmp(orders[player_id][cur], obj) == 0){
                    // reset timer
                    update_timer(timer_cus[player_id], sec_cus[player_id] + 2, sec_cus[player_id]);
                    // delete ice cream 1 in hand
                    mes10(player_id, from_x, from_y, "empty", 0, 0);
                    mes10(player_id, from_x, from_y, "empty", 0, 1);
                    hand[player_id] = strdup("");
                    printf("hand[%d]: %s\n", player_id, hand[player_id]);
                    // check if win
                    fin_cnt[player_id]++;
                    if(fin_cnt[player_id] == goal){
                        // game end
                        mes98(player_id, 1);
                        mes98(!player_id, 0);
                        return;
                    }
                    // complete order
                    order_cnt[player_id]--;
                    mes12(player_id, 1, 0);
                    mes12(player_id, 1, 1);
                }
                else{
                    printf("not fit: hand: %s, order: %s\n", hand[player_id], orders[player_id][cur]);
                    mes14(player_id, player_id);
                }
            }
            else{
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "0") == 0){
            // player
            if(to_loc == 6){
                // floor, player move
                mes11(player_id, from_x, from_y, to_x, to_y, 0);
                mes11(player_id, from_x, from_y, to_x, to_y, 1);
            }
            else if(to_loc == 2 && action == 0){
                // pick up obj on assemble counter
                if(strcmp(assem[player_id], "") && strcmp(hand[player_id], "") == 0){
                    // delete obj on chopping board
                    mes10(player_id, to_x, to_y, "empty", 1, 0);
                    mes10(player_id, to_x, to_y, "empty", 1, 1);
                    // obj in hands
                    mes10(player_id, 0, 0, assem[player_id], 0, 0);
                    mes10(player_id, 0, 0, assem[player_id], 0, 1);

                    hand[player_id] = strdup(assem[player_id]);
                    assem[player_id] = strdup("");
                    printf("hand[%d]: %s, assem: %s\n", player_id, hand[player_id], assem[player_id]);
                }
                else{
                    printf("not fit: hand: %s, assem: %s\n", hand[player_id], assem[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 1 && action == 0){
                // pick up obj on chopping board
                if(strcmp(chop[player_id], "") && strcmp(hand[player_id], "") == 0){
                    // delete obj on chopping board
                    mes10(player_id, to_x, to_y, "empty", 1, 0);
                    mes10(player_id, to_x, to_y, "empty", 1, 1);
                    // obj in hands
                    mes10(player_id, 0, 0, chop[player_id], 0, 0);
                    mes10(player_id, 0, 0, chop[player_id], 0, 1);

                    hand[player_id] = strdup(chop[player_id]);
                    chop[player_id] = strdup("");
                    printf("hand[%d]: %s, chop: %s\n", player_id, hand[player_id], chop[player_id]);
                }
                else{
                    printf("not fit: hand: %s, chop: %s\n", hand[player_id], chop[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                printf("invalid player move\n");
                mes14(player_id, player_id);
            }
        }
        else if(strcmp(obj, "empty") == 0){
            if(to_loc == 2 && action == 1){
                // assemble action
                assem_logic(player_id, to_x, to_y);
            }
            else if(to_loc == 2 && action == 0){
                // pick up obj on assemble counter
                if(strcmp(assem[player_id], "") && strcmp(hand[player_id], "") == 0){
                    // delete obj on chopping board
                    mes10(player_id, to_x, to_y, "empty", 1, 0);
                    mes10(player_id, to_x, to_y, "empty", 1, 1);
                    // obj in hands
                    mes10(player_id, 0, 0, assem[player_id], 0, 0);
                    mes10(player_id, 0, 0, assem[player_id], 0, 1);

                    hand[player_id] = strdup(assem[player_id]);
                    assem[player_id] = strdup("");
                    printf("hand[%d]: %s, assem: %s\n", player_id, hand[player_id], assem[player_id]);
                }
                else{
                    printf("not fit: hand: %s, assem: %s\n", hand[player_id], assem[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else if(to_loc == 1 && action == 0){
                // pick up obj on chopping board
                if(strcmp(chop[player_id], "") && strcmp(hand[player_id], "") == 0){
                    // delete obj on chopping board
                    mes10(player_id, to_x, to_y, "empty", 1, 0);
                    mes10(player_id, to_x, to_y, "empty", 1, 1);
                    // obj in hands
                    mes10(player_id, 0, 0, chop[player_id], 0, 0);
                    mes10(player_id, 0, 0, chop[player_id], 0, 1);

                    hand[player_id] = strdup(chop[player_id]);
                    chop[player_id] = strdup("");
                    printf("hand[%d]: %s, chop: %s\n", player_id, hand[player_id], chop[player_id]);
                }
                else{
                    printf("not fit: hand: %s, chop: %s\n", hand[player_id], chop[player_id]);
                    mes14(player_id, player_id);
                }
            }
            else{
                printf("invalid player move\n");
                mes14(player_id, player_id);
            }
        }
    }
    else{
        printf("Invalid message type\n");
    }
    return;
}



int
main(int argc, char **argv)
{
	int					listenfd, connfd[2], val;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];
    void sig_chld(int);

    // TCP server
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
	Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);

    fd_set rset, wset;
    int maxfd, n;
    char id[2][MAXLINE];
    bool eof[2] = {false, false, false};
    char *cliaddr1, *cliaddr2;
    printf("server listening...\n");

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
        // user 1
		if((connfd[0] = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){
            if (errno == EINTR){
                    printf("error: eintr\n");
                    continue;
                }
            else err_sys("accept error");
        }
        cliaddr1 = inet_ntoa(cliaddr.sin_addr);

        if ((n = Read(connfd[0], id[0], MAXLINE)) == 0){
            err_quit("str_cli: server terminated prematurely");
        }
        id[0][n] = '\0';
        printf("player0: %s\n", id[0]);
        fflush(stdout);
        snprintf(sendline, sizeof(sendline), "You are the 1st player. Wait for the second one!\n");
        Writen(connfd[0], sendline, strlen(sendline));

        // user 2
        if((connfd[1] = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){
            if (errno == EINTR){
                    printf("error: eintr\n");
                    continue;
                }
            else err_sys("accept error");
        }
        cliaddr2 = inet_ntoa(cliaddr.sin_addr);

        if ((n = Read(connfd[1], id[1], MAXLINE)) == 0){
            err_quit("str_cli: server terminated prematurely");
        }
        id[1][n] = '\0';
        printf("player1: %s\n", id[1]);
        fflush(stdout);
        snprintf(sendline, sizeof(sendline), "You are the 2nd player.\n");
        Writen(connfd[1], sendline, strlen(sendline));

        snprintf(sendline, sizeof(sendline), "The second player is %s from %s.\n", id[1], cliaddr2);
        Writen(connfd[0], sendline, strlen(sendline));
        snprintf(sendline, sizeof(sendline), "The first player is %s from %s.\n", id[0], cliaddr1);
        Writen(connfd[1], sendline, strlen(sendline));

        // set fd nonblocking
        val = Fcntl(connfd[0], F_GETFL, 0);
	    Fcntl(connfd[0], F_SETFL, val | O_NONBLOCK);
        val = Fcntl(connfd[1], F_GETFL, 0);
	    Fcntl(connfd[1], F_SETFL, val | O_NONBLOCK);
        maxfd = max(connfd[0], connfd[1]);

		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */

            int nready, nwritten;
            int n_user = 2;
            printf("\n - %s and %s into gameroom -\n", id[0], id[1]);
            fflush(stdout);

            // Seed the random number generator
            srand(time(NULL));

            iptr[0] = optr[0] = buffer[0];	/* initialize buffer pointers */
	        iptr[1] = optr[1] = buffer[1];

            int money_p1 = 0, money_p2 = 0;
            // initialize empty locations
            hand[0] = strdup("");
            hand[1] = strdup("");
            chop[0] = strdup("");
            chop[1] = strdup("");
            assem[0] = strdup("");
            assem[1] = strdup("");

            // init order
            orders[0] = get_new_order(5);
            orders[1] = get_new_order(5);
            order_cnt[0] = 5;
            order_cnt[1] = 5;
            
            for( ; ; ){

                for(int k=0; k<2; k++){
                    if(order_cnt[k] == 0){
                        free(orders[k]);
                        orders[k] = get_new_order(5);
                        mes13(orders[k], k, 0);
                        mes13(orders[k], k, 1);
                        sec_cus[k] -= 3;
                        update_timer(timer_cus[k], sec_cus[k] + 2, sec_cus[k]);
                        order_cnt[k] = 5;
                    }
                }

                FD_ZERO(&rset);
		        FD_ZERO(&wset);

                if (iptr[0] < &buffer[0][MAXLINE] && !eof[0])
                    FD_SET(connfd[0], &rset);
                if (iptr[1] < &buffer[1][MAXLINE] && !eof[1])
                    FD_SET(connfd[1], &rset);
                if (optr[0] != iptr[0])
			        FD_SET(connfd[0], &wset);
                if (optr[1] != iptr[1])
			        FD_SET(connfd[1], &wset);
                
                nready = select(maxfd+1, &rset, &wset, NULL, NULL);
                if(nready < 0){
                    if (errno == EINTR){
                        printf("error: eintr\n");
                        continue;
                    }
                    else err_sys("select error");
                }

                for(int k=0; k<2; k++){
                    // write to fd
                    if(FD_ISSET(connfd[k], &wset) && ( (n = iptr[k] - optr[k]) > 0)){
                        if ( (nwritten = safe_write(connfd[k], optr[k], n)) < 0) {
                            if (errno != EWOULDBLOCK)
                                err_sys("write error to connfd[%d]", k);
                        }
                        else {
                            fprintf(stderr, "%s: wrote %d bytes to connfd[%d]\n", gf_time(), nwritten, k);
                            optr[k] += nwritten;		/* # just written */
                            if (optr[k] == iptr[k])
                                optr[k] = iptr[k] = buffer[k];	/* back to beginning of buffer */
                        }
                    }
                }

                for(int k=0; k<2; k++){
                    // read from fd
                    memset(recvline, 0, sizeof(recvline));
                    if(FD_ISSET(connfd[k], &rset)){
                        if ( (n = safe_read(connfd[k], recvline, MAXLINE)) == 0) {
                            // user quit
                            n_user--;
                            printf("Player %d quit, n_user = %d\n", k, n_user);
                            if(n_user == 1){
                                mes99(!k);
                                eof[k] = true;
                                if (optr[k] == iptr[k])
                                    Shutdown(connfd[k], SHUT_WR);
                            }
                            else{
                                if (connfd[k] >= 0) {
                                    if (shutdown(connfd[k], SHUT_WR) < 0) {
                                        perror("shutdown error");
                                    } else {
                                        printf("Socket connfd[%d] shutdown successfully.\n", k);
                                    }
                                } else {
                                    printf("Socket connfd[%d] is already closed or invalid.\n", k);
                                }
                                eof[k] = true;
                                break;
                                // exit
                            }
                        }
                        else if(n < 0){
                            if (errno != EWOULDBLOCK)
                                err_sys("read error on connfd");
                        }
                        else{
                            handle_message(recvline, k);
                        }
                    }
                }
                if(eof[0] && eof[1])
                    break;
            }
            Close(connfd[0]);
            Close(connfd[1]);

            printf("exit\n");
			exit(0);
		}
		Close(connfd[0]);			/* parent closes connected socket */
        Close(connfd[1]);
	}
}
