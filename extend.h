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
void question_read();
void question_generate(Question *question);



void question_to_confirm_add(Question question);



//user's part

typedef struct users_{
    char id[N];
    char pwd[N];
    int coin;
}User;
void user_read();
void user_write();

//return
// -1 : not found
//  0 : match
//  1 : incorrect
int user_check(char* id, char* pwd);

