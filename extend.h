#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include <wchar.h>
#define N 256

//
void question_read();
void question_write();
void question_generate(struct question *q);
void question_to_confirm_add(struct question q);
void coin_update(char *id,int coin);
//user's part
typedef struct users_{
    char id[N];
    char pwd[N];
    int coin;
}User;
void user_read();
void user_add();
void user_write();
//return
// -1 : not found
//  0 : match
//  1 : incorrect
int user_check(char* id, char* pwd);

