#include	"unp.h"
#include	"common.h"
#include  <string.h>
#include  <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define true 1
#define false 0

char id[MAXLINE];




void clr_scr() {
	printf("\x1B[2J");
};

void set_scr() {		// set screen to 80 * 25 color mode
	printf("\x1B[=3h");
};

void xchg_data(FILE *fp, int sockfd)
{
    int       maxfdp1, stdineof, peer_exit, n;
    fd_set    rset;
    char      sendline[MAXLINE], recvline[MAXLINE];

	set_scr();
	clr_scr();

    Read(sockfd, recvline, MAXLINE);
	printf("recv(for sign): %s", recvline);

    Fgets(sendline, MAXLINE, fp);
    Writen(sockfd, sendline, strlen(sendline));

    Read(sockfd, recvline, MAXLINE);//read server id msg
	printf("recv(for id): %s", recvline); 

    Fgets(sendline, MAXLINE, fp);
    Writen(sockfd, sendline, strlen(sendline));

    Read(sockfd, recvline, MAXLINE);//read server 
	printf("recv (password message): %s", recvline);

    Fgets(sendline, MAXLINE, fp);//read cli password
    Writen(sockfd, sendline, strlen(sendline));
    printf("ready to enter thread guestroom\n");

    Read(sockfd, recvline, MAXLINE);//read server 
	printf("recv (cli choice): %s", recvline);
	struct climsg *client_msg = (struct climsg*)malloc(sizeof(struct climsg));
	for ( ; ; ){//start choice loop
		Fgets(sendline, MAXLINE, fp);//read choice
		
		int ch;
		sscanf(sendline,"%d",&ch);

		memset(sendline,'\0',sizeof(sendline));
		client_msg->type = MENU;
		client_msg->menuopt = (char)(ch + 48);
		serialize_climsg(client_msg,sendline,sizeof(sendline));
		Writen(sockfd, sendline, strlen(sendline));

		printf("please wait for the game to start!!!\n");

		//Read(sockfd, recvline, MAXLINE);//read server (start message)
		//printf("recv (cli choice): %s", recvline); 


		memset(recvline,'\0',strlen(recvline));
		printf("waiting for the question\n");
		//for ( ; ; ){
		if ( (n = Read(sockfd, recvline, MAXLINE)) > 0) printf("recv (all question): %s\n", recvline);
		//}
		stdineof = 0;
		peer_exit = 0;

		int type;
		int num;
		int count = 0;
		int countq = 0;
		int current = 0;
		sscanf(recvline,"%d,%d",&type,&num);
		char qq[30][MAXLINE];
		for (int i=0;i<strlen(recvline);i++){
			//printf("checking question %d\n",i);
			if (recvline[i] == '<'){
				count = 0;
				for ( ; ; ){
					i++;
					if (recvline[i] == '>') break;
					qq[countq][count] = recvline[i];
					count++;
				}
				countq++;
			}
			//printf("finish checking  ch: %d\n",ch);
		}
		
		if (ch == 2){
			clock_t a1,a2;
			struct climsg *msg = (struct climsg *)malloc(sizeof(struct climsg));
			num = 3;
			printf("start giving question(雙人經典)? num: %d\n",num);
			for (int i=0;i<num;i++){
				int readserv = 0;
				printf("Here is the question(雙人經典)~ %s\n%s\n%s\n%s\n%s\n",qq[i*5],qq[i*5+1],qq[i*5+2],qq[i*5+3],qq[i*5+4]);//print question
				a1 = clock();
				a2 = a1;
				int persec = 100;
				int stop = false;
				for ( ; ; ) {
					a2 = clock();
					//printf("time %f (a1:%d  a2:%d),(readserv: %d)\n",(double)(a2 - a1)/persec,a1,a2,readserv);
					struct timeval timeout;
					timeout.tv_sec = 1;  // 5 seconds timeout
					timeout.tv_usec = 0;
					if (a2 > a1 + 10*persec && stop == false){//超時
						a2 = clock();
						stop = true;
						printf("time %f (a1:%d  a2:%d),(readserv: %d)\n",(double)(a2 - a1)/persec,a1,a2,readserv);			
						//a1 = 1000*persec;
						memset(msg, 0, sizeof(struct climsg));
						msg->type = ANSWER;
						msg->ans = -1;
						msg->anstime = 10000;
						//serialize_climsg(msg,(void*)sendline, 11);
						sprintf(sendline,"%d %d",-1,10000);
						Writen(sockfd, sendline, strlen(sendline));
						printf("時間到!!! send : %s\n",sendline);
					}	
					FD_ZERO(&rset);
					FD_SET(fileno(fp), &rset);
					FD_SET(sockfd, &rset);
					maxfdp1 = max(sockfd,fileno(fp)) + 1;
					maxfdp1++;
					Select(maxfdp1, &rset, NULL, NULL, &timeout);
					if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
						n = read(sockfd, recvline, MAXLINE);
						readserv++;
						if (n == 0) {
							if (stdineof == 1)
								return;         /* normal termination */
							else {
								printf("(End of input from the peer!)");
								peer_exit = 1;
								return;
							};
						}
						else if (n > 0) {
							recvline[n] = '\0';
							int type,ans_correct;
							sscanf(recvline,"%d %d\n",&type,&ans_correct);
							if (type == 2){
								printf("這一回合結束，正確答案是 %d ，開始下一回合，原始訊息:%s\n",ans_correct,recvline);
								readserv = 0;
								break;
							}
							char ppid[10000];
							int sc,right,ans;
							sscanf(recvline,"%d %s %d %d\0",&right,ppid,&sc,&ans);
							if (right == 0) printf("recv from server ~ id(%s:%d) 錯誤了\n",ppid,sc);
							else{
								printf("recv from server ~ id(%s:%d) 正確\n",id,sc);
							}
						};
					}
					if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
						Fgets(sendline, MAXLINE, fp);
						memset(msg, 0, sizeof(struct climsg));
						int ans;
						sscanf(sendline,"%d",&ans);
						msg->type = ANSWER;
						msg->ans = ans;
						msg->anstime = (time_t)(a2 - a1);
						//serialize_climsg(msg,(void*)sendline, 11);
						a2 = clock();
						sprintf(sendline,"%d %f\n",ans,(double)(a2 - a1)/persec);
						Writen(sockfd, sendline, strlen(sendline));
						printf("你傳送答案,time is %f\n",(double)(a2 - a1)/persec);
						stop = true;
					}
				}
				printf("break out from loop\n");
			}
			for ( ; ; ){
				if ( (n = read(sockfd, recvline, MAXLINE)) > 0 ){
					int type;
					sscanf(recvline,"%d",&type);
					printf("recv from server: %s\n",recvline);
					if (type == 3) break;
				}
				
			}
		}
		else if (ch == 3){
			clock_t a1,a2;
			num = 3;
			printf("start giving question(多人搶答)? num: %d\n",num);
			for (int i=0;i<num;i++){
				int readserv = 0;
				printf("Here is the question(多人搶答)~ %s\n%s\n%s\n%s\n%s\n",qq[i*5],qq[i*5+1],qq[i*5+2],qq[i*5+3],qq[i*5+4]);//print question
				a1 = clock();
				a2 = a1;
				int persec = 100;
				int stop = false;
				for ( ; ; ) {
					a2 = clock();
					//printf("time %f (a1:%d  a2:%d),(readserv: %d)\n",(double)(a2 - a1)/persec,a1,a2,readserv);
					struct timeval timeout;
					timeout.tv_sec = 1;  // 5 seconds timeout
					timeout.tv_usec = 0;
					if (a2 > a1 + 10*persec && stop == false){//超時
						a2 = clock();
						stop = true;
						printf("time %f (a1:%d  a2:%d),(readserv: %d)\n",(double)(a2 - a1)/persec,a1,a2,readserv);			
						//a1 = 1000*persec;
						sprintf(sendline,"%d %d",-1,10000);
						Writen(sockfd, sendline, strlen(sendline));
						printf("時間到!!! send : %s\n",sendline);
					}	
					FD_ZERO(&rset);
					FD_SET(fileno(fp), &rset);
					FD_SET(sockfd, &rset);
					maxfdp1 = max(sockfd,fileno(fp)) + 1;
					maxfdp1++;
					Select(maxfdp1, &rset, NULL, NULL, &timeout);
					if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
						n = read(sockfd, recvline, MAXLINE);
						readserv++;
						if (n == 0) {
							if (stdineof == 1)
								return;         /* normal termination */
							else {
								printf("(End of input from the peer!)");
								peer_exit = 1;
								return;
							};
						}
						else if (n > 0) {
							recvline[n] = '\0';
							int type,ans_correct;
							sscanf(recvline,"%d %d\n",&type,&ans_correct);
							if (type == 2){
								printf("這一回合結束，正確答案是 %d ，開始下一回合，原始訊息:%s\n",ans_correct,recvline);
								readserv = 0;
								break;
							}
							char ppid[1000];
							int sc,right,ans;
							sscanf(recvline,"%d %s %d %d\0",&right,ppid,&sc,&ans);
							if (right == 0) printf("recv from server ~ id(%s:%d) 錯誤了，選項%d是錯的，\n",ppid,sc,ans);
							else{
								printf("recv from server ~ id(%s:%d) 正確，選項%d是答案，搶答成功進入下一題\n",id,sc,ans);
							}
						};
					}
					if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
						Fgets(sendline, MAXLINE, fp);
						int ans;
						sscanf(sendline,"%d",&ans);
						a2 = clock();
						sprintf(sendline,"%d %f\n",ans,(double)(a2 - a1)/persec);
						Writen(sockfd, sendline, strlen(sendline));
						printf("你傳送答案,time is %f\n",(double)(a2 - a1)/persec);
						stop = true;
					}
				}
				printf("break out from loop\n");
			}
			memset(sendline,'\0',MAXLINE);
			sprintf(sendline,"99999\n");
			Writen(sockfd, sendline, strlen(sendline));
			printf("99999 結束信號以傳送\n");
			for ( ; ; ){
				Read(sockfd, recvline, MAXLINE);//read server question 
				int type;
				sscanf(recvline,"%d",&type);
				printf("recv from server: %s\n",recvline);
				if (type == 3) break;
			}
		}
		printf("回歸大廳,please enter your choice (1 single) (2 twoplayer) (3 multiplayer)\n");
	}
};

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	//if (argc != 3) err_quit("usage: tcpcli <IPaddress> <ID>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+3);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	//strcpy(id, argv[2]);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	xchg_data(stdin, sockfd);		/* do it all */

	exit(0);
}
