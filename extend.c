#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#define N 256

typedef struct questions_{
    wchar_t problem[N];
    wchar_t choice[4][N];
    int answer;
}Question;
Question questions[N]; // all question
int question_num = 0;
void question_read(){
    FILE *fp = fopen("./problem.txt", "r");
    wchar_t line[N];
    question_num = 0;
    while(fgetws(line, sizeof(line), fp) != NULL){
        swscanf(line, L"\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"",
            questions[question_num].problem,
            questions[question_num].choice[0],
            questions[question_num].choice[1],
            questions[question_num].choice[2],
            questions[question_num].choice[3]);
            questions[question_num].answer = 0;
        ++question_num;

    }
    fclose(fp);
}
void question_generate(Question *question){
    int rd = rand()%question_num;
    memcpy(question, &questions[rd], sizeof(wchar_t) * N);
    question->answer = rand() % 4;
    wchar_t temp[N];
    wcscpy(temp, question->choice[question->answer]);
    wcscpy(question->choice[question->answer], question->choice[0]);
    wcscpy(question->choice[0], temp);
}

Question question_to_confirm[N];
int question_to_confirm_num = 0;
void question_to_confirm_add(Question question){
    memcpy(question_to_confirm[question_to_confirm_num], question, sizeof(Question));
    ++question_to_confirm;
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
void user_check(char* id, char* pwd){
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


//for test
int main(){}
