# include "unp.h"
# include "config.h"
# include "ui.h"
# include <pthread.h>
# include <string.h>

int sockfd;
struct sockaddr_in servaddr;
char sendline[MAXLINE];
char recvline[MAXLINE];
S2C servInst;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int mouseX = -1.0, mouseY = -1.0;
int nowmouseX = -1.0, nowmouseY = -1.0;
int premouseX = -1.0, premouseY = -1.0;

void* ui_thread_func(void* arg) {
    run_ui ();
    return NULL;
}

void user_input (int X, int Y) {
    // printf("user input: x, y = %d, %d", X, Y);
    pthread_mutex_lock(&mutex);
    mouseX = X;
    mouseY = Y;
    pthread_mutex_unlock(&mutex);
}

void connect2serv (char **argv) {
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    printf("connect success!\n");
}

void parse_server_inst (char* inst) {
    int op;
    char tmp[50];
    sscanf(inst, "%d %[^\n]", &op, tmp);
    // printf("tmp: %s\n", tmp);
    printf("op = %d\n", op);
    if (op == 10) {
        servInst.op = 10;
        sscanf(tmp, "%d %d %d %s %d", &servInst.client, &servInst.toX, &servInst.toY, servInst.object, &servInst.location);

    }
    else if (op == 11) {
        servInst.op = 11;
        sscanf(tmp, "%d %d %d %d %d %s", &servInst.client, &servInst.fromX, &servInst.fromY, &servInst.toX, &servInst.toY, servInst.object);
    }
    else if (op == 12) {
        servInst.op = 12;
        sscanf(tmp, "%d %d", &servInst.client, &servInst.complete);
    }
    else if (op == 13) {
        printf("orders:\n");
        servInst.op = 13;
        char ttmp[50];
        sscanf(tmp, "%d %[^\n]", &servInst.client, ttmp);
        // printf("tmp: %s", ttmp);
        char* token;
        token = strtok(ttmp, " ");
        while (token != NULL) {
            char* order = strdup(token);
            printf("order: %s\n", order);
            free(order);
            token = strtok(NULL, " ");
        }
    }
    // else if (op == 14) {
    //     servInst.op = 14;

    // }

}

void location (int x, int y, int* location, char* object) {
    *location = 6;
    strcpy(object, "0");
    if (x <= 90) {  // burger
        if (338 <= y && y <= 410) {  // lettuce
            if (premouseX <= 100 && 338 <= premouseY && premouseY <= 410) {
                strcpy(object, "l0");
                *location = 40;
            }
        }
        else if (425 <= y && y <= 490) {  // tomato
            if (premouseX <= 100 && 425 <= premouseY && premouseY <= 490) {
                strcpy(object, "t0");
                *location = 41;
            }
        }
        else if (505 <= y && y <= 575) {  // meat
            if (premouseX <= 100 && 505 <= premouseY && premouseY <= 575) {
                strcpy(object, "m");
                *location = 42;
            }
        }
        else if (590 <= y && y <= 660) {  // bread
            if (premouseX <= 100 && 590 <= premouseY && premouseY <= 660) {
                strcpy(object, "b");
                *location = 43;
            }
        }
    }
    else if (x >= 480) {  // ice cream
        if (438 <= y && y <= 515) {  // mint
            if (premouseX >= 480 && 438 <= premouseY && premouseY <= 515) {
                // object = "f0";
                strcpy(object, "f0");
                *location = 44;
            }
        }
        else if (522 <= y && y <= 593) {  // coffee
            if (premouseX >= 480 && 522 <= premouseY && premouseY <= 593) {
                // object = "f1";
                strcpy(object, "f1");
                *location = 45;
            }
        }
        else if (605 <= y && y <= 670) {  // cone
            if (premouseX >= 480 && 605 <= premouseY && premouseY <= 670) {
                // object = "c";
                strcpy(object, "c");
                *location = 46;
            }
        }
    }
    else if (y >= 685) {  // work place
        if (118 <= x && x <= 184) {  // assemb
            *location = 2;
            // object = "0";
            strcpy(object, "0");
        }
        else if (210 <= x && x <= 340) {  // chop
            *location = 1;
            // object = "0";
            strcpy(object, "0");
        }
        else if (415 <= x && x <= 470) {  // trash
            *location = 3;
            // object = "0";
            strcpy(object, "0");
        }
    }
    else if (319 <= y && y <= 395) {
        *location = 5;
        // object = "0";
        strcpy(object, "0");
    }
    else {  // just move
        *location = 6;
        // object = "0";
        strcpy(object, "0");
    }
    printf("in proce, obj: %s\n", object);
}

char* parse_input (int x, int y,char* input) {
    // char* input = strdup();
    C2S c2s;
    c2s.fromX = premouseX; c2s.fromY = premouseY;
    c2s.toX = nowmouseX; c2s.toY = nowmouseY;
    // location(premouseX, premouseY, &c2s.fromLoc, c2s.obj);
    memset(c2s.object, 0, sizeof(c2s.object));
    location(nowmouseX, nowmouseY, &c2s.toLoc, c2s.object);
    c2s.action = 0;
    // printf("inst: %s %d %d %d %d %d %d\n", c2s.object, c2s.fromX, c2s.fromY, c2s.toX, c2s.toY, c2s.toLoc, c2s.action);
    // printf("hello?\n");
    // char input[200];
    snprintf(input, 200, "%s %d %d %d %d %d %d\n", c2s.object, c2s.fromX, c2s.fromY, c2s.toX, c2s.toY, c2s.toLoc, c2s.action);
    // printf("input: %s\n", input);
    return input;
}

void pass_msg (int sockfd) {
    fd_set rset;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    FD_ZERO(&rset);
    while (1) {
        // user mouse action
        pthread_mutex_lock(&mutex);
        nowmouseX = mouseX; nowmouseY = mouseY;
        // printf("now mouse x, y = %d, %d", nowmouseX, nowmouseY);
        if (nowmouseX != premouseX || nowmouseY != premouseY) {
            printf("mouse coord: %d, %d\n", nowmouseX, nowmouseY);
            char userInput[200];
            parse_input(nowmouseX, nowmouseY, userInput);
            printf("send to server: %s\n", userInput);
            Writen(sockfd, userInput, strlen(userInput));
        }
        pthread_mutex_unlock(&mutex);

        // receive from host
        FD_SET(sockfd, &rset);
        int n = Select(sockfd+1, &rset, NULL, NULL, &tv);
        if (n > 0) {
            memset(recvline, 0, MAXLINE);
            if (Read(sockfd, recvline, MAXLINE) == 0)
                err_quit("str_cli: server terminated prematurely");
            printf("get host msg: %s", recvline);
            parse_server_inst(recvline);
            // add_msg(recvline);
            add_msg(servInst);
        }
        premouseX = nowmouseX; premouseY = nowmouseY;
    }
}

int main (int argc, char **argv) {
    
    if (argc != 2) err_quit("usage: tcpcli <IPaddress>");

    connect2serv(argv);

    pthread_t ui_thread;
    if (pthread_create(&ui_thread, NULL, ui_thread_func, NULL) != 0) {
        err_quit("failed to create ui thread");
    }

    pass_msg(sockfd);

    ui_running = 0;
    pthread_join(ui_thread, NULL);
    exit(0);
}
