# include "unp.h"
# include "config.h"
# include "ui.h"
# include <pthread.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>

int sockfd;
struct sockaddr_in servaddr;
char sendline[MAXLINE];
char recvline[MAXLINE];
S2C servInst;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int mouseX = -1.0, mouseY = -1.0;
int nowmouseX = -1.0, nowmouseY = -1.0;
int premouseX = -1.0, premouseY = -1.0;
char handobj[50];
int preloc = 6;

void* ui_thread_func(void* arg) {
    run_ui ();
    return NULL;
}

void user_input (int X, int Y, char* obj) {
    // printf("user input: x, y = %d, %d", X, Y);
    pthread_mutex_lock(&mutex);
    mouseX = X;
    mouseY = Y;
    strcpy(handobj, obj);
    // printf("get obj %s\n", obj);
    pthread_mutex_unlock(&mutex);
}

void connect2serv (char **argv) {
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    memset(sendline, 0, MAXLINE);
    // if (Fgets(sendline, MAXLINE, stdin) == NULL) return;
    strcpy(sendline, argv[2]);
    Writen(sockfd, sendline, strlen(sendline));
}

void parse_server_inst (char* inst) {
    int op;
    char tmp[50];
    sscanf(inst, "%d %[^\n]", &op, tmp);
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
        servInst.op = 13;
        char ttmp[100];
        sscanf(tmp, "%d %[^\n]", &servInst.client, ttmp);
        char* token;
        token = strtok(ttmp, " ");
        int count = 0;
        while (token != NULL) {
            if (count % 2 == 0) strcpy(servInst.orders[count/2], token);
            else servInst.time[count/2] = atoi(token);
            token = strtok(NULL, " ");
            count ++;
        }
    }
    else if (op == 97) {  // msg + shutdown
        servInst.op = 97;
        sscanf(tmp, "%[^\n]", servInst.message);
        // sscanf(tmp, "%d", servInst.action);
        // servInst.action = 
    }
    else if (op == 98) {  // server msg
        servInst.op = 98;
        sscanf(tmp, "%[^\n]", servInst.message);
    }
    
}

void location (int x, int y, int* location, char* object) {
    *location = 6;
    // printf("now obj %s\n", object);
    if (strcmp(object, "50") != 0) strcpy(object, "0");
    // printf("now obj %s\n", object);
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
            if (premouseY >= 685 && 118 <= premouseX && premouseX <= 184) {
                *location = 2;
                // object = "0";
                strcpy(object, handobj);
            }            
        }
        else if (210 <= x && x <= 340) {  // chop
            if (premouseY >= 685 && 210 <= premouseX && premouseX <= 340) {
                *location = 1;
                // object = "0";
                strcpy(object, handobj);
            }
        }
        else if (415 <= x && x <= 470) {  // trash
            if (premouseY >= 685 && 415 <= premouseX && premouseX <= 470) {
                *location = 3;
                // object = "0";
                strcpy(object, handobj);
            }
        }
    }
    else if (319 <= y && y <= 395) {  // customer
        if (319 <= premouseY && premouseY <= 395) {
            *location = 5;
            // object = "0";
            strcpy(object, handobj);
        }
    }
    // printf("in proce, obj: %s\n", object);
}

char* parse_input (int x, int y,char* input) {
    // char* input = strdup();
    C2S c2s;
    c2s.fromX = premouseX; c2s.fromY = premouseY;
    c2s.toX = nowmouseX; c2s.toY = nowmouseY;
    // location(premouseX, premouseY, &c2s.fromLoc, c2s.obj);
    memset(c2s.object, 0, sizeof(c2s.object));
    strcpy(c2s.object, handobj);
    // printf("hand obj = %s\n", c2s.object);
    location(nowmouseX, nowmouseY, &c2s.toLoc, c2s.object);
    c2s.action = 0;
    if (preloc == 2 && c2s.toLoc == 2) {  // assemb
        c2s.action = 1;
        preloc = 6;
    }
    preloc = c2s.toLoc;
    snprintf(input, 200, "%s %d %d %d %d %d %d\n", c2s.object, c2s.fromX, c2s.fromY, c2s.toX, c2s.toY, c2s.toLoc, c2s.action);
    return input;
}

void pass_msg (int sockfd) {
    fd_set rset;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    FD_ZERO(&rset);
    while (1) {
        // printf("while\n");
        // user mouse action
        pthread_mutex_lock(&mutex);
        nowmouseX = mouseX; nowmouseY = mouseY;
        if (nowmouseX != premouseX || nowmouseY != premouseY) {
            char userInput[200];
            parse_input(nowmouseX, nowmouseY, userInput);
            printf("send to server: %s\n\n", userInput);
            Writen(sockfd, userInput, strlen(userInput));
        }
        pthread_mutex_unlock(&mutex);

        // receive from host
        FD_SET(sockfd, &rset);
        int n = Select(sockfd+1, &rset, NULL, NULL, &tv);
        if (n > 0) {
            // printf("select detect\n");
            memset(recvline, 0, MAXLINE);
            int n;
            if ((n = Readline(sockfd, recvline, MAXLINE)) == 0)
                err_quit("str_cli: server terminated prematurely");
            // recvline[n] = '\0';
            printf("get host msg: %s", recvline);
            parse_server_inst(recvline);
            add_msg(servInst);
            if (servInst.op == 97) {  // shutdown
                // printf("get op = 97\n");
                sleep(3);
                shutdown(sockfd, SHUT_WR);
                // Close(sockfd);
                break;
            }
        }
        premouseX = nowmouseX; premouseY = nowmouseY;
    }
}

int main (int argc, char **argv) {
    
    if (argc != 3) err_quit("usage: tcpcli <IPaddress>");

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