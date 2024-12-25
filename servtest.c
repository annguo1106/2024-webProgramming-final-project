#include "unp.h"
#include <string.h>

void chat_serv(){
	char sendline[MAXLINE];
    int listenfd;
    printf("server started!!\n");
    // prepare TCP serv
    struct sockaddr_in TCPserv;
    bzero(&TCPserv, sizeof(TCPserv));
    TCPserv.sin_family = AF_INET;
    TCPserv.sin_addr.s_addr = htonl(INADDR_ANY);
    TCPserv.sin_port = htons(SERV_PORT);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(listenfd, (SA *)&TCPserv, sizeof(TCPserv));
    Listen(listenfd, LISTENQ);

    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    clilen = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0) {
        if (errno == EINTR){
            return ;
        }
        else err_sys("accept error");
    }
    if (Readline(connfd, sendline, MAXLINE) == 0)
        err_quit("str_cli: server terminated prematurely");
    // main
    while(1) {
        // Writen to first cli
        memset(sendline, 0, MAXLINE);
        if (Fgets(sendline, MAXLINE, stdin) == NULL) return;
        printf("send this to client: %s", sendline);
        Writen(connfd, sendline, strlen(sendline));
    }
}

int main(){
	printf("hello\n");
    chat_serv();

	exit(0);
}