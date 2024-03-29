#include	"unp.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<pthread.h>
#include	<locale.h>
#include	"extend.h"
#include	"common.h"
#define false 0
#define true 1

FILE	*fp;

struct question_form{
    char question[10000];
    int answer;
}q;
/*
struct question_form q1(int n){
    struct question_form qq;
    float n1,n2,a,b,ans,ch1,ch2,ch3,a2,b2,a3,b3,b4;
    srand(time(NULL));
    switch(n){
        case 1:
            qq.answer = rand() % 4 + 1;
            n1 = rand() % 10;
            n2 = rand() % 10 + 11;
            a = rand() % 20 - 10;
            b = rand() % 20 - 10;
            sprintf(qq.question,"1<定積分(x from %.0f --> %.0f) %.0fx + (%.0f)?>\n",n1,n2,a,b);
            ans = (n2*n2 - n1*n1)/2*a + (n2-n1)*b;
            ch1 = ch2 = ch3 = ans;
            while (ch1 == ans || ch2 == ans || ch3 == ans || ch1 == ch2 || ch2 == ch3 || ch1 == ch3){
                ch1 = ans + rand() % 50;
                ch2 = ans + rand() % 50;
                ch3 = ans + rand() % 50;
            }
            if (qq.answer == 1) sprintf(qq.question,"%s,<%.0f>,<%.0f>,<%.0f>,<%.0f>\n",qq.question,ans,ch1,ch2,ch3);
            if (qq.answer == 2) sprintf(qq.question,"%s,<%.0f>,<%.0f>,<%.0f>,<%.0f>\n",qq.question,ch1,ans,ch2,ch3);
            if (qq.answer == 3) sprintf(qq.question,"%s,<%.0f>,<%.0f>,<%.0f>,<%.0f>\n",qq.question,ans,ch1,ch2,ans,ch3);
            if (qq.answer == 4) sprintf(qq.question,"%s,<%.0f>,<%.0f>,<%.0f>,<%.0f>\n",qq.question,ch1,ch2,ch3,ans);
            return qq;
            break;
        case 2:
            qq.answer = rand() % 4 + 1;
            a = rand() % 5 * 4 + 4;
            b = rand() % 5 + 3;
            n1 = a/2;
            n2 = a/4;//negative
            sprintf(qq.question,"%s,<對以下式子做積分: (%.0fx^2)*ln(%.0fx)>",qq.question,a,b);
            b4 = b;
            b = b2 = b3  = n2;
            a = a2 = a3 = n1;
            while (a == n1 || a2 == n1 || a3 == n1 || a == a2 || a2 == a3 || a == a3 || b == n2 || b2 == n2 || b3 == n2 || b == b2 || b2 == b3 || b == b3){
                a = n1 + rand()%10;
                a2 = n1 + rand()%10;
                a3 = n1 * (rand()%5 + 1);
                b2 = n1 + rand()%10;
                b3 = n1 + rand()%10;
                b = n1 * (rand()%5 + 1);
            }
            if (qq.answer == 1) sprintf(qq.question,"%s,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>\n",qq.question,n1,b4,n2,a,b4,b,a2,b4,b2,a3,b4,b3);
            if (qq.answer == 2) sprintf(qq.question,"%s,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>\n",qq.question,a,b4,b,n1,b4,n2,a2,b4,b2,a3,b4,b3);
            if (qq.answer == 3) sprintf(qq.question,"%s,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>\n",qq.question,a2,b4,b2,a,b4,b,n1,b4,n2,a3,b4,b3);
            if (qq.answer == 4) sprintf(qq.question,"%s,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>,<(%.0fx^2)*ln(%.0fx) - %.0fx^2>\n",qq.question,a3,b4,b3,a,b4,b,a2,b4,b2,n1,b4,n2);
            return qq;
            break;
    }
    
}*/

void
sig_chld(int signo)
{
        pid_t   pid;
        int             stat;

        while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
                ;
        return;
}

void sig_pipe(int signo){
	return;
}

int roundscore(time_t timer){
	float score = (10 - timer + 1)*1000;
	return (int)score;
}

struct cli_info{
	int fd;
	char id[10000];
};//sent in first thread //bug?

struct twoplayer_battle{
	char id_your[1000];
	char id_opponent[1000];
	int connfd;
	int connfd_opponent;
	int seq;
}*twoplayer_info;	//bug?

struct multiplayer_battle{
	int total_player;
	int multi_connfd[10];
	char multi_id[10][MAXLINE];
	int seq;
}*multiplayer_info,*twoplayer_msg;
//struct multiplayer_battle *twoplayer_msg;
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int player_num = 0;
int multiplayer_num = 0;

int seq_number = 0;
int flag[1000];

void twoplayergame(void *sock){//0->player1   1->player2
	seq_number++;
	char rec[MAXLINE],sent[MAXLINE];
	int question_num = 3;//問題數量，請修改
	int dead = false;
	int question_current = 0;//current question
	int answer_correct[100] = {1,3,4,0};//array store correct answer
	int player_ans[2][100];
	int player_score[100] = {0};//array for player score
	
	struct climsg *client_msg;
	client_msg = (struct climsg*)malloc(sizeof(struct climsg));
	struct servmsg *server_msg;
	server_msg = (struct servmsg*)malloc(sizeof(struct servmsg));
	server_msg->type = INIT_2P;
	server_msg->questions = (struct question*)malloc(sizeof(struct question) * question_num);
	for(int i=0;i<question_num;i++){
		question_generate(&server_msg->questions[i]);
		answer_correct[i] = (int) (server_msg->questions+i)->ans;
		answer_correct[i]++;
	}
	server_msg->numq = question_num;
	//question set 包含 size_t numq; struct question* questions;
	//serialize_question(server_msg->questions,question_num,sent,sizeof(sent));
	

	int total_player,multi_connfd[10],final_score[10] = {0};
	int ans,answer_num, scorechange;
	time_t ans_time;
	char multi_id[10][MAXLINE];
	total_player = ((struct multiplayer_battle*)sock)->total_player;
	int seq = ((struct multiplayer_battle*)sock)->seq;
	for (int i=0;i<total_player;i++){
		strcpy(multi_id[i],((struct multiplayer_battle*)sock)->multi_id[i]);
		multi_connfd[i] = ((struct multiplayer_battle*)sock)->multi_connfd[i];
		//printf("(%d) multi_id: %s multi_connfd: %d numq:%d\n",i,multi_id[i],multi_connfd[i],server_msg->numq);
		final_score[i] = 0;
	}
	free(sock);
	for (int i=0;i<total_player;i++){
		if (i == 0){
			server_msg->assigned = (char)48;
			strcpy(server_msg->oppid,multi_id[1]);
		}
		if (i == 1){
			server_msg->assigned = (char)49;
			strcpy(server_msg->oppid,multi_id[0]);
		}
		//printf("第二則(回圈內)(%d) multi_id: %s multi_connfd: %d numq:%d\n",i,server_msg->oppid,server_msg->assigned,server_msg->numq);
		if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0){
			Writen(multi_connfd[i], sent, n);//sent result to client
			//printf("用函數輸出 %d\n",i);
			//print_servmsg(server_msg);
		}	
		else{
			printf("166 serialize error\n");
		}
		
	}
	
	int	maxfdp1;
	fd_set	rset;
	for ( ; ; ){
		for (int i=0;i<total_player;i++){
			FD_SET(multi_connfd[i], &rset);
			maxfdp1 = max(maxfdp1,multi_connfd[i]);
		}
		struct timeval timeout;
		timeout.tv_sec = 1;  // 5 seconds timeout
		timeout.tv_usec = 0;
		maxfdp1++;
		Select(maxfdp1, &rset, NULL, NULL, &timeout);
		for (int i=0;i<total_player;i++){
			if (FD_ISSET(multi_connfd[i], &rset)){
		 		if ((n = Readline(multi_connfd[i], rec, MAXLINE)) == 0) {
					printf("有client死掉了\n");
					dead = true;
					memset(sent, '\0', sizeof(sent));
					memset(server_msg, 0, sizeof(struct servmsg));
					server_msg->type = EVAL_ANS;
					server_msg->player = (char)(9 + 48);
					server_msg->scorechange = -1;
					server_msg->correct = '9';
					for (int j = 0;j<total_player;j++){
						if (i != j){
							if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0)
							Writen(multi_connfd[j], sent, n);//sent result to client
							else{
								printf("error for serialize server_msg\n");
							}
						}
					}
					flag[seq] = true;
					return;
				}
				else if (n > 0){
					answer_num++;
					memset(client_msg, 0, sizeof(struct climsg));
					n = deserialize_climsg(client_msg,rec,n);
					printf("接收訊息正確? %d\n",n);
					ans = (int)(client_msg->ans) - 48;
					if (answer_num == 1) ans_time = client_msg->anstime;
					player_ans[i][question_current] = ans;
					printf("接收到client訊息(ans:%d anstime:%d)\n",ans,ans_time);
					if (answer_num == 1){//答案正確
						printf("第一則訊息!!! id : %s(current q:%d)\n",multi_id[i],question_current);
						if (ans == answer_correct[question_current]) scorechange = 1000*(10 - ans_time + client_msg->anstime);
						else scorechange = 0;
						memset(sent, '\0', sizeof(sent));
						memset(server_msg, 0, sizeof(struct servmsg));
						server_msg->type = EVAL_ANS;
						server_msg->player = (char)(i + 48);
						server_msg->scorechange = scorechange;
						player_score[i] += scorechange;
						if (ans == answer_correct[question_current]) server_msg->correct = '1';
						else server_msg->correct = '0';
						//sprintf(sent,"1 %s %d %d\0",multi_id[i],player_score[i],ans);
						// print_servmsg(&server_msg);
						printf("id=%s, score=%d\n", multi_id[i], player_score[i]);
						for (int j = 0;j<total_player;j++){
							if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0)
								Writen(multi_connfd[j], sent, n);//sent result to client
							else{
								//error for serialize server_msg
							}
						}
					}
					else if (answer_num == 2){//叫client進入下一輪
						printf("第二則訊息(current q:%d)!!!\n",question_current);
						if (ans == answer_correct[question_current]) scorechange = 1000*(10 - ans_time + client_msg->anstime);
						else scorechange = 0;
						memset(sent, '\0', sizeof(sent));
						memset(server_msg, 0, sizeof(struct servmsg));
						server_msg->ans = (char)(answer_correct[question_current] + 48);
						//sprintf(,"%d",answer_correct[question_current]);
						server_msg->type = EVAL_ANS;
						player_score[i] += scorechange;
						server_msg->scorechange = scorechange;
						for (int j = 0;j<total_player;j++){
							if (j == 0) server_msg->oppans = (char)(player_ans[1][question_current] + 48);
							if (j == 1) server_msg->oppans = (char)(player_ans[0][question_current] + 48);
							//if (j == 0) server_msg->player = '1';
							//if (j == 1) server_msg->player = '0';
							server_msg->player = (char)(i + 48);
							if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0){
								Writen(multi_connfd[j], sent, n);//sent result to client
								printf("傳送結束信號給client!!!(multiconnfd %d, player:%c)\n",multi_connfd[j],server_msg->player);
							}
							else{
								printf("傳送結束信號給client，serialize error!!!\n");
							}
						}
						question_current++;
						answer_num = 0;//assume all player will sent a message to client no matter they answer the question or not
						if (question_current == question_num) {//比賽結束，傳結果
							
							memset(sent, '\0', sizeof(sent));
							memset(server_msg, 0, sizeof(struct servmsg));
							server_msg->type = GAME_RESULT;
							server_msg->numplayer = 2;
							server_msg->resultdata = (struct player_result*)malloc(sizeof(struct player_result) * 2);
							server_msg->resultdata[0].score = player_score[0];
							server_msg->resultdata[1].score = player_score[1];
							if (player_score[0] > player_score[1]){
								server_msg->resultdata[0].coin = 500;
								server_msg->resultdata[1].coin = -100;
								coin_update(multi_id[0],500);
								coin_update(multi_id[1],-100);
							}
							else if(player_score[0] < player_score[1]){
								server_msg->resultdata[1].coin = 500;
								server_msg->resultdata[0].coin = -100;
								coin_update(multi_id[1],500);
								coin_update(multi_id[0],-100);
							}
							else{
								server_msg->resultdata[1].coin = 100;
								server_msg->resultdata[0].coin = 100;
								coin_update(multi_id[1],100);
								coin_update(multi_id[0],100);
							}
							
							for (int j = 0;j<total_player;j++){
								printf("傳送結果\n",ans,ans_time);
								//print_servmsg(server_msg);
								if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0)
									Writen(multi_connfd[j], sent, n);//sent result to client
								else{
									//error for serialize server_msg
								}
							}
							
							//for (int j = 0;j<total_player;j++) sprintf(sent,"%splayerid:%s\nplayer score:%d\n",sent,multi_id[j],player_score[j]);
							//for (int j = 0;j<total_player;j++) Writen(multi_connfd[j], sent, MAXLINE);//sent result to client
							flag[seq] = true;
							
							return;
						}
						
					}
				}
		 	}
		}
		if (question_current == question_num) break;
	}
}


void multiplayergame(void *sock){
	seq_number++;
	char rec[MAXLINE],sent[MAXLINE];
	int question_num = 3;//問題數量，請修改
	int question_current = 0;//current question
	int answer_correct[100] = {1,3,4,0};//array store correct answer
	int player_ans[10][100];
	int player_score[100] = {0};//array for player score
	int finish_flag = 0;
	struct servmsg *server_msg;
	server_msg = (struct servmsg*)malloc(sizeof(struct servmsg));
	server_msg->type = INIT_2P;
	server_msg->questions = (struct question*)malloc(sizeof(struct question) * question_num);
	for(int i=0;i<question_num;i++){
		question_generate(&server_msg->questions[i]);
		answer_correct[i] = (int) (server_msg->questions+i)->ans;
		answer_correct[i]++;
	}
	server_msg->numq = question_num;
	//question set 包含 size_t numq; struct question* questions;
	//serialize_question(server_msg->questions,question_num,sent,sizeof(sent));
	

	int total_player,multi_connfd[10],final_score[10] = {0};
	int ans,answer_num;
	time_t ans_time;
	char multi_id[10][MAXLINE];
	total_player = ((struct multiplayer_battle*)sock)->total_player;
	int seq = ((struct multiplayer_battle*)sock)->seq;
	for (int i=0;i<total_player;i++){
		strcpy(multi_id[i],((struct multiplayer_battle*)sock)->multi_id[i]);
		multi_connfd[i] = ((struct multiplayer_battle*)sock)->multi_connfd[i];
		//printf("(%d) multi_id: %s multi_connfd: %d numq:%d\n",i,multi_id[i],multi_connfd[i],server_msg->numq);
		final_score[i] = 0;
	}
	free(sock);
	for (int i=0;i<total_player;i++){
		server_msg->assigned = (char)48;
		strcpy(server_msg->oppid,multi_id[1]);
		//printf("第二則(回圈內)(%d) multi_id: %s multi_connfd: %d numq:%d\n",i,server_msg->oppid,server_msg->assigned,server_msg->numq);
		if ((n = serialize_servmsg(server_msg,sent,sizeof(sent))) > 0){
			Writen(multi_connfd[i], sent, n);//sent result to client
			//printf("用函數輸出 %d\n",i);
			//print_servmsg(server_msg);
		}	
		else{
			printf("166 serialize error\n");
		}
		
	}
	
	int	maxfdp1 = 1;
	fd_set	rset;
	for ( ; ; ){
		for (int i=0;i<total_player;i++){
			FD_SET(multi_connfd[i], &rset);
			maxfdp1 = max(maxfdp1,multi_connfd[i]);
		}
		struct timeval timeout;
			timeout.tv_sec = 1;  // 5 seconds timeout
			timeout.tv_usec = 0;
		maxfdp1++;
		if (finish_flag == total_player) break;
		Select(maxfdp1, &rset, NULL, NULL, &timeout);
		for (int i=0;i<total_player;i++){
			if (FD_ISSET(multi_connfd[i], &rset)){
		 		if ((n = Read(multi_connfd[i], rec, MAXLINE)) == 0) {
				//end of connection set
				}
				else if (n > 0){
					int over;
					sscanf(rec,"%d",&over);
					if (over == 99999) {
						finish_flag++;	
					}
					else{
						
						sscanf(rec,"%d %lf\0",&ans,&ans_time);
						if (ans == answer_correct[question_current]){//答案正確
							player_score[i] += 1000*(total_player - answer_num + 1)/total_player;
							answer_num = total_player;
							memset(sent, '\0', sizeof(sent));
							sprintf(sent,"1 %s %d %d\0",multi_id[i],player_score[i],ans);
							for (int j = 0;j<total_player;j++) Writen(multi_connfd[j], sent, MAXLINE);//sent result to client
						}
						else{//答案錯誤
							answer_num++;
							memset(sent, '\0', sizeof(sent));
							sprintf(sent,"0 %s %d %d\0",multi_id[i],player_score[i],ans);
							for (int j = 0;j<total_player;j++) Writen(multi_connfd[j], sent, MAXLINE);//sent result to client
						}
						if (answer_num >= total_player){//叫client進入下一輪
							memset(sent, '\0', sizeof(sent));
							sprintf(sent,"2 %d  q_c:%d  q_num:%d\n",answer_correct[question_current],question_current,question_num);
							for (int j = 0;j<total_player;j++) Writen(multi_connfd[j], sent, MAXLINE);//sent result to client//考慮要不要進入下一題時傳一筆特殊訊息給client
							question_current++;
							answer_num = 0;//assume all player will sent a message to client no matter they answer the question or not
						}
					}
				}
		 	}
		}
		if (question_current == question_num){
			memset(sent, '\0', sizeof(sent));
			sprintf(sent,"3 final result:\n");
			for (int i = 0;i<total_player;i++) sprintf(sent,"%splayerid:%s\nplayer score:%d\n",sent,multi_id[i],player_score[i]);
			for (int j = 0;j<total_player;j++) Writen(multi_connfd[j], sent, MAXLINE);//sent result to client
			flag[seq] = true;
			break;
		}
	}
	
	
}

void* guestroom(void* sock)
{
	int connfd,n,ch;
	
	char rec[MAXLINE],id[MAXLINE],send[MAXLINE];//\e[31m sample text \e[0m //1 for single, 2 for two player,3 for multiple player, 4 for player rank
	pthread_t two;
	pthread_t three;
	
	connfd = ((struct cli_info *) sock)->fd;
	strcpy(id,((struct cli_info *) sock)->id);
	printf("大廳(1) id:%s\n",id);
	
	free(sock);//cause double free error
	//sock = NULL;
	Pthread_detach(pthread_self());
	
	int	maxfdp1 = connfd + 1;
	fd_set	rset;
	clock_t time1,time2;
	//struct twoplayer_battle *your_info = Malloc(sizeof(struct twoplayer_battle));
	struct twoplayer_battle *your_info = (struct twoplayer_battle *)malloc(sizeof(struct twoplayer_battle));
	//snprintf(send, MAXLINE, "Please enter your choice, 1 (single) ，2 (two player battle)， 3 (multiplayer battle):\n");
	//Writen(connfd,send,MAXLINE);

	memset(rec, '\0', sizeof(rec));
	int guest_seq = seq_number;
	int persec = 1000;
	struct climsg *client_msg = (struct climsg *)malloc(sizeof(struct climsg));
	for ( ; ; ){
		FD_SET(connfd, &rset);
		maxfdp1 = connfd + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(connfd, &rset)){
			if ((n = Readline(connfd, rec, MAXLINE)) == 0) {//read player choice, 2 for duel
				return;
			}
			else if (n > 0){
				memset(client_msg,0,sizeof(struct climsg));
				deserialize_climsg(client_msg,rec,n);
				ch = (int)(client_msg->menuopt) - 48;
				//printf("this is server, 選擇(%d)\n",ch);
				guest_seq = seq_number;
				//sscanf(rec,"%d",&ch);//記得關掉
				if (ch == 2){//twoplayer game
					player_num += 1;
					if (player_num == 1) {
						twoplayer_msg = (struct multiplayer_battle*)malloc(sizeof(struct multiplayer_battle));
						twoplayer_msg->seq = guest_seq;
						flag[guest_seq] = false;
					}
					twoplayer_msg->total_player = player_num;
					twoplayer_msg->multi_connfd[player_num-1] = connfd;//multiplayer_num-1
					strcpy(twoplayer_msg->multi_id[player_num-1],id);
					printf("大廳(2) id:%s\n",twoplayer_msg->multi_id[player_num-1]);
					//char test[1000]; 
					//sprintf(test,"測試有無執行? tp:%d playernum = %d  connfd[0] = %d  connfd = %d\n",twoplayer_msg->total_player,player_num,twoplayer_msg->multi_connfd[0],connfd);
					//Writen(connfd,test,MAXLINE);
					for ( ; ; ){
						if (flag[guest_seq] == true) break;
						//char test[1000]; 
						//sprintf(test,"測試有無執行? tp:%d playernum = %d  connfd[0] = %d  connfd = %d\n",twoplayer_msg->total_player,player_num,twoplayer_msg->multi_connfd[0],connfd);
						//Writen(connfd,test,MAXLINE);
						if (player_num == 2 && twoplayer_msg->multi_connfd[1] == connfd){	
							player_num = 0;
							pthread_create(&two,NULL,&twoplayergame,(void*)twoplayer_msg);
							ch = -1;
						}
					}
					//char test[1000]; 
					//sprintf(test,"測試失敗? playernum = %d  connfd[0] = %d  connfd = %d\n",player_num,twoplayer_msg->multi_connfd[0],connfd);
					//Writen(connfd,test,MAXLINE);
					ch = -1;
				}
				if (ch == 3){
					multiplayer_num++;
					if (multiplayer_num == 1) {
						time1 = clock();
						time2 = time1;
						//free(multiplayer_info);
						multiplayer_info = (struct multiplayer_battle*)malloc(sizeof(struct multiplayer_battle));
						multiplayer_info->seq = guest_seq;
						flag[guest_seq] = false;
					}
					multiplayer_info->total_player = multiplayer_num;
					multiplayer_info->multi_connfd[multiplayer_num-1] = connfd;//multiplayer_num-1
					strcpy(multiplayer_info->multi_id[multiplayer_num-1],id);
					
					for ( ; ; ){
						ch = -1;
						if (flag[guest_seq] == true) break;
						if (connfd == multiplayer_info->multi_connfd[0]) time2 = clock();
						if (time2 >= time1 + 60*CLOCKS_PER_SEC && connfd == multiplayer_info->multi_connfd[0]){
							//starting message?
							//sprintf(send,"time1:%d    time2:%d\n",time1,time2);
							//Writen(connfd,send,MAXLINE);
							multiplayer_num = 0;
							pthread_create(&three,NULL,&multiplayergame,(void*)multiplayer_info);
							ch = -1;
							break;
						}
					}
				}
				if(ch == '4'){
				    struct question q;
				    char send[MAXLINE], recv[MAXLINE];

				    snprintf(send, MAXLINE, "enter question:\n");
				    Writen(connfd, send, MAXLINE);
				    int n = Read(connfd, recv, MAXLINE);
				    wcscpy(q.q , recv);

				    snprintf(send, MAXLINE, "enter correct anwer:");
				    Writen(connfd, send, MAXLINE);
				    n = Read(connfd, recv, MAXLINE);
				    wcscpy(q.option[0], recv);
				    for(int i=1;i<4;i++){
				        snprintf(send, MAXLINE, "enter other choices(%d):", i);
				        Writen(connfd, send, MAXLINE);
				        n = Read(connfd, recv, MAXLINE);
				        wcscpy(q.option[i], recv);
				    }
				    question_to_confirm_add(q);
				}
			}
					
		}
	}		
}
void sign_in(void* ptr){
    Pthread_detach(pthread_self());
    pthread_t tid;
    struct cli_info *cli = (struct cli_info*)ptr;
    int connfd = cli -> fd;
    char recv[sizeof(struct climsg)], send[sizeof(struct servmsg)];
    struct climsg cmsg;
    struct servmsg smsg;
    int n, again;
    n = Readline(connfd, recv, MAXLINE);
    if(n==0)return;
    n = deserialize_climsg(&cmsg, recv, n);
    snprintf(cli->id, N, "%s", cmsg.id);
    char id[LOGIN_MAXLEN], pwd[LOGIN_MAXLEN];
    int valid = user_check(cmsg.id, cmsg.pw);
printf("zz%dzz", valid);
    switch(cmsg.type){
    	case CLI_LOGIN:
    		smsg.type = SERV_LOGIN;
    		if(valid == 0){
    			//match
				again = 0;
    			smsg.success = '1';
    		}else if(valid == 1){
    			//incorrect
    			again = 1;
    			smsg.success = '0';
    		}else if(valid == -1){
    			//not found
    			again = 1;
    			smsg.success = '0';
    		}
    		break;
    	case CLI_REGISTER:
    		smsg.type = SERV_REGISTER;
    		if(valid == -1){
				again = 0;
    			user_add(cmsg.id, cmsg.pw);
    			smsg.success = '1';
    		}else{
    			again = 1;
    			smsg.success = '0';
    		}
    		break;
    	default :
    		//error
    		break;
    }
    n = serialize_servmsg( &smsg, send, sizeof(send));
    Writen(connfd, send, n);
    if(again==1)pthread_create(&tid,NULL,&sign_in,(void*)cli);
    else pthread_create(&tid,NULL,&guestroom,(void*)cli);
}


// void ctrlroom(int connfd){
// 	int n;
// 	char recv[MAXLINE], send[MAXLINE];
// 	wchar_t wsend[MAXLINE];
// 	for(;;){

// 		//snprintf(send, MAXLINE, "0:quit.\n");
// 		//swprintf(send, MAXLINE, L"0:quit");
// 		//Writen(connfd, send, MAXLINE);
		
// 		//snprintf(send, MAXLINE, "1:commit question.\n");
// 		//swprintf(send, MAXLINE, L"1:commit question.");
// 		//Writen(connfd, send, MAXLINE);
		
// 		//snprintf(send, MAXLINE, "2:back up question list.\n");
// 		//swprintf(send, MAXLINE, L"2:back up question list.\n");
// 		//Writen(connfd, send, MAXLINE);
		
// 		//snprintf(send, MAXLINE, "3:back up user list.\n");
// 		//swprintf(send, MAXLINE, L"2:back up user list.\n");
// 		//Writen(connfd, send, MAXLINE);

// 		//Readline(connfd, recv, MAXLINE);
		
// 		int op = 0;
// 		sscanf(recv, "%d", &op);
// 		switch(op){
// 			case 0:
// 				close(connfd);
// 				return;
// 			case 1:
// 				struct question q;
// 				wchar_t buf[Q_MAXLEN];
// 				char r[Q_MAXLEN], t[Q_MAXLEN];
// 				//send problem and all option
// 				question_to_confirm_get(&q);
// 				wcscpy(buf, q.q);
// 				Writen(connfd, buf, sizeof(wchar_t)*Q_MAXLEN);
// 				for(int i=0;i<4;i++){
// 					wcscpy(buf, q.option[i]);
// 					Writen(connfd, buf, sizeof(wchar_t)*Q_MAXLEN);
// 				}
				
// 				//recv y/n
// 				n = Readline(connfd, r, Q_MAXLEN);
// 				if(n==0){}//connect out
// 				else{
// 					r[n] = '\0';
// 					sscanf(r, "%s", t);
// 					if(strcmp(t, "y") == 0){
// 						snprintf(send, MAXLINE, "%d\n", question_to_confirm_confirm());
// 						Writen(connfd, send, strlen(send));
// 					}else if(strcmp(t, "n")==0){
// 						snprintf(send, MAXLINE, "%d\n", question_to_confirm_not_confirm());
// 						Writen(connfd, send, strlen(send));
// 					}
// 				}
// 				break;	
// 			case 2:
// 				snprintf(send, MAXLINE, "%d\n", question_write());
// 				Writen(connfd, send, strlen(send));
// 				break;
// 			case 3:
// 				snprintf(send, MAXLINE, "%d\n", user_write());
// 				Writen(connfd, send, strlen(send));
// 				break;
// 			default:
// 				break;
		
		
// 		}
// 	}
	
	
	
	
	
	
// }


// void ctrl_listen(void* ptr){
// 	int			listenfd, connfd;
// 	socklen_t		clilen;
// 	struct sockaddr_in	cliaddr, servaddr;
//         char                    buff[MAXLINE];
//         time_t			ticks;
        
//         listenfd = Socket(AF_INET, SOCK_STREAM, 0);//listenfd is server fd
// 	bzero(&servaddr, sizeof(servaddr));
// 	servaddr.sin_family      = AF_INET;
// 	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
// 	servaddr.sin_port        = htons(SERV_PORT+2);
// 	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
// 	Listen(listenfd, LISTENQ);

// 	for ( ; ; ) {
// 		clilen = sizeof(cliaddr);
//                 int connfd;
                
//                 if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
//                         if (errno == EINTR)
//                                 continue;               /* back to for() */
//                         else
//                                 err_sys("accept error");
//                 }
// 		struct sockaddr_in myaddr;
// 		int myaddrlen = sizeof(myaddr);
// 		getpeername(connfd, (SA*)&myaddr ,&myaddrlen);
// 		ticks = time(NULL);
// 		fprintf(fp, "===================\n");
// 		fprintf (fp, "CTRL:%.24s: connected from %s, port %d\n",
// 		ctime(&ticks),
// 		Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof (buff)),
// 		ntohs(cliaddr.sin_port));
// 		srand((int) ticks);
// 		ctrlroom(connfd);
// 	}
// }

int main(int argc, char **argv)
{
	int			listenfd, connfd;
	socklen_t		clilen;
	struct sockaddr_in	cliaddr, servaddr;
        char                    buff[MAXLINE];
        
        int *iptr;
        pthread_t tid;
	time_t			ticks;
        

	//init all question array
	setlocale(LC_ALL, "");
	question_read();
	
	
	
	
	
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);//listenfd is server fd
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT+3);//set server port
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
	Listen(listenfd, LISTENQ);
	
	Signal(SIGPIPE, sig_pipe);
        Signal(SIGCHLD, sig_chld);      /* must call waitpid() */
        //Signal(SIGINT, sig_chld);
		fp = fopen("finalproject.log", "a");
        if ((fp = fopen("finalproject.log", "w")) == NULL) {
           printf("log file open error!\n");
           exit(0);
        };
        
        //pthread_create(&tid,NULL,&ctrlroom,(void*)cli1);
        
	for ( ; ; ) {
				clilen = sizeof(cliaddr);
                iptr = Malloc(sizeof(int));
                if ( (*iptr = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
                        if (errno == EINTR)
                                continue;               /* back to for() */
                        else
                                err_sys("accept error");
                }
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
		
		struct cli_info *cli1 = Malloc(sizeof(struct cli_info));
		cli1->fd = *iptr;
		free(iptr);
		iptr = NULL;
		pthread_create(&tid,NULL,&sign_in,(void*)cli1);
	}
}