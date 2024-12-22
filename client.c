# include "unp.h"
# include "config.h"
# include "ui.h"
# include <pthread.h>
# include <string.h>

int sockfd;
struct sockaddr_in servaddr;
char sendline[MAXLINE];
char recvline[MAXLINE];

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

void pass_msg (int sockfd) {
    fd_set rset;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    FD_ZERO(&rset);
    while (1) {
        // user mouse action
        pthread_mutex_lock(&mutex);
        nowmouseX = mouseX; nowmouseY = mouseY;
        // printf("now mouse x, y = %d, %d", nowmouseX, nowmouseY);
        if (nowmouseX != premouseX || nowmouseY != premouseY) {
            printf("mouse coord: %d, %d\n", nowmouseX, nowmouseY);
            premouseX = nowmouseX; premouseY = nowmouseY;
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
            add_msg(recvline);
        }
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
