#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include "common.h"
#define N 256

struct question questions[N]; // all question
int question_num_ = 0;
struct question question_to_confirm[N];
int question_to_confirm_num = 0;

void question_read(){
    srand(time(NULL));
    FILE *fp = fopen("./problem.txt", "r");
    wchar_t line[N];
    question_num_ = 0;
    while(fgetws(line, sizeof(line), fp) != NULL){
        swscanf(line, L"\"%l[^\"]\",\"%l[^\"]\",\"%l[^\"]\",\"%l[^\"]\",\"%l[^\"]\"",
            questions[question_num_].q,
            questions[question_num_].option[0],
            questions[question_num_].option[1],
            questions[question_num_].option[2],
            questions[question_num_].option[3]);
            questions[question_num_].ans = 0;
        ++question_num_;
    }
    fclose(fp);
}
void question_write() {
    FILE *fp = fopen("./problem.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }
    for (int i = 0; i < question_num_; ++i) {
        fwprintf(fp, L"\"%ls\",\"%ls\",\"%ls\",\"%ls\",\"%ls\"\n",
                 questions[i].q,
                 questions[i].option[0],
                 questions[i].option[1],
                 questions[i].option[2],
                 questions[i].option[3]);
    }
    fclose(fp);
}
void question_generate(struct question *q){
    int rd = rand()%question_num_;
    memcpy(q, &questions[rd], sizeof(struct question));
    q->ans = rand() % 4;
    wchar_t temp[N];
    wcscpy(temp, q->option[(int)q->ans]);
    wcscpy(q->option[(int)q->ans], q->option[0]);
    wcscpy(q->option[0], temp);
}



void question_to_confirm_add(struct question q){
    memcpy(&question_to_confirm[question_to_confirm_num], &q, sizeof(struct question));
    ++question_to_confirm_num;
}

//user's part
typedef struct users_{
    char id[N];
    char pwd[N];
    int coin;
}User;
User users[N];
int user_num = 0;
void user_read(){
    FILE *fp = fopen("./users.txt","r");
    char line[N*2];
    user_num = 0;
    while(fgets(line, sizeof(line), fp) != NULL && user_num < N){
        sscanf(line,"%s %s %d",
            users[user_num].id,
            users[user_num].pwd,
            &users[user_num].coin);
        ++user_num;
    }
    fclose(fp);
}
void user_write(){
    FILE *fp = fopen("./users.txt","w");
    for(int i=0;i<user_num;i++){
        fprintf(fp, "%s %s %d\n",
            users[i].id,
            users[i].pwd,
            users[i].coin);
    }
    fclose(fp);
}
void user_add(char* id, char* pwd){
    snprintf(users[user_num].id, N, id);
    snprintf(users[user_num].pwd, N, pwd);
    users[user_num].coin = 0;
    ++user_num;
}
void coin_update(char *id, int coin){
	for(int i;i<user_num;i++){
	    if(strcmp(id,users[i].id)==0){
	    	users[i].coin+=	coin;
	    }
	}
}
//return
// -1 : not found
//  0 : match
//  1 : incorrect
int user_check(char* id, char* pwd){
    int i = 0;
    for(;i<user_num;i++){
        if(strcmp(id,users[i].id)==0)break;
    }
    if(i==user_num)return -1;
    else{
        if(strcmp(users[i].pwd, pwd) == 0)return 0;
        else return 1;
    }
}

// for test 
/*
int main(){
    printf("%s\n",setlocale(LC_ALL, ""));
    question_read();
    for (int i = 0;i<13;i++){
        printf("%ls\n", questions[i].q);
        printf("%ls\n", questions[i].option[0]);
        printf("%ls\n", questions[i].option[1]);
        printf("%ls\n", questions[i].option[2]);
        printf("%ls\n\n", questions[i].option[3]);
    }
    struct question *q = malloc(sizeof(question));
    question_generate(question);
    printf("%ls\n", q->q);
    printf("%ls\n", q->option[0]);
    printf("%ls\n", q->option[1]);
    printf("%ls\n", q->option[2]);
    printf("%ls\n\n", q->option[3]);*/



