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
    FILE *fp = fopen("./problem.txt", "r");
    wchar_t line[N];
    question_num_ = 0;
    while(fgetws(line, sizeof(line), fp) != NULL){
        swscanf(line, L"\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"",
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
void question_generate(struct question *q){
    int rd = rand()%question_num_;
    memcpy(q, &questions[rd], sizeof(wchar_t) * N);
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

//void user_modify(){}



