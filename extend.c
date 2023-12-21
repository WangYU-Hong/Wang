
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
