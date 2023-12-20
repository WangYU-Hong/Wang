 
#include	"unp.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<pthread.h>

FILE	*fp;
struct client_info{
	int connfd;
	char id[1000];
}clients[1000];

int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int player_num = 0;
int multiplayer_num = 0;

void
sig_chld(int signo)
{
        pid_t   pid;
        int             stat;

        while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
                ;
        return;
}

int roundscore(time_t timer){
	float score = (10 - timer + 1)*1000;
	return (int)score;
}

void sendtoall(char* msg, int curr)
{
	int i;
	pthread_mutex_lock(&mutex);
	
	for (i = 1; i <= 10; i++) 
	{
		if (i != curr && clients[i].connfd != -1) 
		{
			if (send(clients[i].connfd, msg, strlen(msg), 0) < 0) 
			{
				continue;
			}
		}
	}

	pthread_mutex_unlock(&mutex);
}

struct cli_info{
	int fd;
	int number;
}*cli1;

struct twoplayer_battle{
	char id_player1[MAXLINE];
	char id_player2[MAXLINE];
	int connfd1;
	int connfd2;
}*twoplayer_info;

struct multiplayer_battle{
	int total_player;
	int multi_connfd[10];
	char multi_id[10][MAXLINE];
}*multiplayer_info;

void* twoplayergame(void *sock){
	char rec[MAXLINE],sent[MAXLINE];;
	int question_num = 3,question_current = 0;
	int answer_correct[100] = {0,1,3,4};
	int player_score[100] = {0};
	char question[MAXLINE] = "1\n<question: what month is today?>,<1 december>,<2 november>,<3 july>,<4 october>\n<question: evaluate the population of the world>,<1 eighty million>,<2 eighty trillion>,<3 eighty billion>,<4 eighty thosand>\n<question: which date is the deadline of the final project?>,<1 12/25>,<2 12/26>,<3 12/27><4 12/28>\0";
	
	char playerid1[MAXLINE],playerid2[MAXLINE];
	int playerfd1,playerfd2,ans;
	time_t ans_time;
	playerfd1 =  ((struct  twoplayer_battle*) sock)->connfd1;
	playerfd2 =  ((struct  twoplayer_battle*) sock)->connfd2;
	strcpy(playerid1[MAXLINE],((struct  twoplayer_battle*) sock)->id_player1);
	strcpy(playerid2[MAXLINE],((struct  twoplayer_battle*) sock)->id_player2);
	free(sock);
	Pthread_detach(pthread_self());
	
	Writen(playerfd1, question, MAXLINE);//sent question to client
	Writen(playerfd2, question, MAXLINE);//sent question to client
	
	int	maxfdp1 = max(playerfd1,playerfd2);
	fd_set	rset,wset;
	
	for ( ; ; ){
		FD_ZERO(&rset);
		FD_SET(playerfd1, &rset);
		FD_SET(playerfd2, &rset);
		maxfdp1 = max(playerfd1,playerfd2) + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);
		
		if (question_current == question_num * 2) break;
		if (FD_ISSET(playerfd1, &rset)) {	/* socket is readable */
			memset(rec, '\0', sizeof(rec));
			if ((n = Read(playerfd1, rec, MAXLINE)) == 0) {
				//end of connection set
			}
			else if (n > 0){
				question_current++;
				sscanf(rec,"%d,<%d>\0",&ans,&ans_time);
				if (ans == answer_correct[(question_current - 1)/2]){
					player_score[0] += roundscore(ans_time);
					memset(sent, '\0', sizeof(sent));
					sprintf(sent,"2\n<%s>,<%d>,<%d>\0",playerid1,player_score[0],1);
					Writen(playerfd1, sent, MAXLINE);//sent result to client
					Writen(playerfd2, sent, MAXLINE);//sent result to client
				}
				else{
					memset(sent, '\0', sizeof(sent));
					sprintf(sent,"2\n<%s>,<%d>,<%d>\0",playerid1,player_score[0],0);
					Writen(playerfd1, sent, MAXLINE);//sent result to client
					Writen(playerfd2, sent, MAXLINE);//sent result to client
				}
			}
		}
		if (FD_ISSET(playerfd2, &rset)) {	/* socket is readable */
			memset(rec, '\0', sizeof(rec));
			if ((n = Read(playerfd2, rec, MAXLINE)) == 0) {
				//end of connection set
			}
			else if (n > 0){
				question_current++;
				sscanf(rec,"%d,<%d>\0",&ans,&ans_time);
				if (ans == answer_correct[(question_current - 1)/2]){
					player_score[1] += roundscore(ans_time);
					memset(sent, '\0', sizeof(sent));
					sprintf(sent,"2\n<%s>,<%d>,<%d>\0",playerid2,player_score[1],1);
					Writen(playerfd1, sent, MAXLINE);//sent result to client
					Writen(playerfd2, sent, MAXLINE);//sent result to client
				}
				else{
					memset(sent, '\0', sizeof(sent));
					sprintf(sent,"2\n<%s>,<%d>,<%d>\0",playerid2,player_score[1],0);
					Writen(playerfd1, sent, MAXLINE);//sent result to client
					Writen(playerfd2, sent, MAXLINE);//sent result to client
				}
			}
		}
	}
	memset(sent, '\0', sizeof(sent));
	sprintf(sent,"3\n<%s>,<%d>,<%d>\n<%s><%d><%d>\0",playerid1,player_score[0],0,playerid2,player_score[1],0);
	Writen(playerfd1, sent, MAXLINE);//sent result to client
	Writen(playerfd2, sent, MAXLINE);//sent result to client
}

void* multiplayergame(void *sock){
	char rec[MAXLINE],sent[MAXLINE];
	int question_num = 3,question_current = 0;
	int answer_correct[100] = {0,1,3,4};
	int player_score[100] = {0};
	char question[MAXLINE] = "1\n<question: what month is today?>,<1 december>,<2 november>,<3 july>,<4 october>\n<question: evaluate the population of the world>,<1 eighty million>,<2 eighty trillion>,<3 eighty billion>,<4 eighty thosand>\n<question: which date is the deadline of the final project?>,<1 12/25>,<2 12/26>,<3 12/27><4 12/28>\0";
	
	int totalplayer;
	//char []
}

void* guestroom(void* sock)
{
	int connfd,len = 1,num,error,ans,n;
	time_t ans_time;
	char rec[MAXLINE],id[MAXLINE],sent[MAXLINE];//\e[31m sample text \e[0m
	char choice;//1 for single, 2 for two player,3 for multiple player, 4 for player rank
	pthread_t two;
	pthread_t three;
	
	connfd = ((struct cli_info *) sock)->fd;
	num = ((struct cli_info *) sock)->number;
	free(sock);
	Pthread_detach(pthread_self());
	
	memset(id, '\0', sizeof(id));
	Read(connfd, id, MAXLINE);//read player id
	
	int	maxfdp1 = connfd + 1;
	fd_set	rset;
	clock_t time1,time2;
	
	for ( ; ; ){
		FD_SET(connfd, &rset);
		maxfdp1 = connfd + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(connfd, &rset)){
			memset(choice, '\0', sizeof(choice));
			
			if ((n = Read(connfd, choice, MAXLINE)) == 0) {//read player choice, 2 for duel
				//end of connection set
			}
			else if (n > 0){
				if (choice == '2'){
					player_num++;
					if (player_num == 1){
						twoplayer_info = Malloc(sizeof(struct twoplayer_battle));
						strcpy(twoplayer_info->id_player1,id);
						twoplayer_info->connfd1 = connfd;
					}
					else if (player_num == 2){
						strcpy(twoplayer_info->id_player2,id);
						twoplayer_info->connfd2 = connfd;
					}
					for ( ; ; ){
						if (player_num == 2){
							pthread_create(&two,NULL,&twoplayergame,twoplayer_info);
							choice = -1;
							player_num = 0;
							break;
						}
					}
				}
				if (choice == '3'){
					multiplayer_num++;
					if (multiplayer_num == 1) multiplayer_info = Malloc(sizeof(struct multiplayer_battle));
					multiplayer_info->total_player = multiplayer_num;
					multiplayer_info->multi_connfd[multiplayer_num-1] = connfd;
					strcpy(multiplayer_info->multi_connfd[multiplayer_num-1],id);
					if (multiplayer_num == 1) time1 = clock();
					for ( ; ; ){
						time2 = clock();
						if (time1 >= time2 + 60000){
							pthread_create(&three,NULL,&multiplayergame,multiplayer_info);
							choice = -1;
							multiplayer_num = 0;
							break;
						}
					}
				}
			}
		}
	}
}

int
main(int argc, char **argv)
{
	int			listenfd, connfd,connfd2;
	pid_t			childpid;
	socklen_t		clilen,clilen2;
	struct sockaddr_in	cliaddr, cliaddr2 , servaddr;
        char                    buff[MAXLINE];
        time_t			ticks;
        int *iptr;
        pthread_t tid;
        

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);//listenfd is server fd

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT+3);//set server port

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

        Signal(SIGCHLD, sig_chld);      /* must call waitpid() */
        //Signal(SIGINT, sig_chld);

        if ((fp = fopen("finalproject.log", "a")) == NULL) {
           printf("log file open error!\n");
           exit(0);
        };
        for (int j=1;j<=10;j++) clients[j].connfd = -1;
	for ( ; ; ) {
		int num;
                char rec[MAXLINE],name[MAXLINE],sent[MAXLINE];
                
		clilen = sizeof(cliaddr);
                iptr = Malloc(sizeof(int));
                if ( (*iptr = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
                        if (errno == EINTR)
                                continue;               /* back to for() */
                        else
                                err_sys("accept error");
                }
                n++;
                struct sockaddr_in myaddr;
		int myaddrlen = sizeof(myaddr);
		getpeername(*iptr, (SA*)&myaddr ,&myaddrlen);
           	ticks = time(NULL);
            	fprintf(fp, "===================\n");
            	fprintf (fp, "%.24s: connected from %s, port %d\n",
                ctime(&ticks),
		Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof (buff)),
                ntohs(cliaddr.sin_port));
            	srand((int) ticks);
            	
		int current;
		for (int j=1;j<=n;j++){
			if (n == 1){
				current = 1;
				break;
			}
			else if (clients[j].connfd == -1){
				current = j;
				break;
			}
		}
		clients[current].connfd = *iptr;
		cli1 = Malloc(sizeof(struct cli_info));
		cli1->fd = *iptr;
		cli1->number = current;
		
		pthread_create(&tid,NULL,&guestroom,cli1);		/* parent closes connected socket */
	}
}
