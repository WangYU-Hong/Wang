#include "unp.h"
#include <locale.h>
#include <wchar.h>
#include <stdio.h>

#define N 256
/*
int func(int sockfd){
    int maxfdp1, stdineof=0;
    fd_set rset;
    char recv[MAXLINE], send[MAXLINE];

    for(;;){
        FD_ZERO(&rset);
        if(stdineof == 0 )FD_SET(fileno(fp));
        FD_SET(sockfd);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        Select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)){
            n = Read(sockfd, recv, MAXLINE);
            if(n==0){
                if(sidineof ==1)return;
                else err_quit("server terminated prematurely");
            }
            Fputs(recv, stdout);
        }
        if(FD_ISSET(fileno(fp), &rset){
            if (Fgets(sendline, MAXLINE, fp) == NULL){
                stdineof = 1;
                Shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            Writen(sockfd, send, MAXLINE);
        }
    }
}*/

void print(){
    printf("0 : Quit\n");
    printf("1 : Commit question.\n");
    printf("2 : Back up question list.\n");
    printf("3 : Back up user list.\n");
}
void func(int sockfd){
    char send[MAXLINE], recv[MAXLINE];
    print();
    int op, stat, n;
    char c;
    for(;;){
        op = getchar();
        snprintf(send, MAXLINE, "%d", op);
        Writen(sockfd, send, strlen(send));
        switch(op){
        case 0:
            close(sockfd);
            break;
        case 1:
            wchar_t buf[N];
            n = Readline(sockfd, buf, N * sizeof(wchar_t));
            printf("題目:%ls\n",buf);
            n = Readline(sockfd, buf, N * sizeof(wchar_t));
            printf("正解:%ls\n",buf);
            for(int i=0;i<3;i++){
                n = Readline(sockfd, buf, N * sizeof(wchar_t));
                printf("其他:%ls\n",buf);
            }
            printf("[y/n]\n");
            c = getchar();
            snprintf(send, MAXLINE, "%c\n", c);
            Writen(sockfd, send, strlen(send));
            sscanf(recv, "%d", &stat);
            if(stat!=0)printf("commit fail!");
            else printf("commit success.");
            break;
        case 2:
        case 3:
            int n = Readline(sockfd, recv, MAXLINE);
            sscanf(recv, "%d", &stat);
            if(stat!=0)printf("back up fail!");
            else printf("back up success.");
            break;
        default:
            printf("invalid input");
            print();
            continue;
        }
    }
}

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;
    setlocale(LC_ALL,"");
    if (argc != 2)err_quit("usage: syscontroller <IPaddress>");
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	func(sockfd);
	exit(0);
}

