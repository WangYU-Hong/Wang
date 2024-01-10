#include <string.h>
#include <wchar.h>
#include <pthread.h>
#include "common.h"
#include "screen.h"

static int sockfd;
FILE *fp;

/*If reader recv a msg, des into msg, write into pipe
If main needn't read yet, and reader recv another msg,
It just overwrites -> bad
Select pipe_fd[0] in reader with timeout = 0 to poll the fd
If can read (return 1) then wait
else (return 0) then can write

-> read/write pipe and modify message are atomic
*/

// reader
// pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t msg_cond = PTHREAD_COND_INITIALIZER;
// static int hasmsg;
static int pipe_fd[2]; // 1 for msg, 0 for server closed
static struct question q[MAXNUMQ];
static struct player_result res[MAXPLAYER];
static struct servmsg msg = {.questions = q, .resultdata = res};

// writer
static ssize_t pktlen;
static char out[MAXLINE];

static char myid[LOGIN_MAXLEN];

void *reader(void *arg)
{
    
    char buf[MAXLINE];
    ssize_t n;
    int ret, nready;
    // used to poll pipe_fd[0]
    fd_set chkset;
    struct timeval t;
    while (1)
    {
        
        n = Readline(sockfd, buf, sizeof(buf));
        if (n == 0)
        {
            // server closed? signal
            write(pipe_fd[1], "0", 1);
            break;
        }
        // check if main has consumed the signal yet
        // bad (busy waiting)
        do {
            t.tv_sec = 0;
            t.tv_usec = 0;
            FD_ZERO(&chkset);
            FD_SET(pipe_fd[0], &chkset);
            nready = select(
                pipe_fd[0] + 1,
                &chkset,
                NULL,
                NULL,
                &t);
        } while (nready == 1);
        // parse packet
        ret = deserialize_servmsg(&msg, buf, n);
        if (ret < 0)
        {
            // bad pkt
            continue;
        }
        // signal
        write(pipe_fd[1], "1", 1);
    }
    close(pipe_fd[1]);
    return NULL;
}

// void* writer(void* arg) {

//     while (1) {
//         // get write signal
//         // write out: critical
//         Writen(sockfd, out, pktlen);

//     }

// }

int login()
{
    struct climsg outmsg;
    relogin:
    // draw login options
    drawloginmenu();
    
    while (1) {
        char key = getch();
        switch (key)
        {
        case '1':
            outmsg.type = CLI_LOGIN;
            break;

        case '2':
            outmsg.type = CLI_REGISTER;
            break;

        case 'q':
            exitwin(0);
            break;
        
        default:
            continue;
            break;
        }
        break;
    }


    // draw login screen
    clear();
    box(stdscr, 0, 0);
    char idprompt[] = "ID:       ";
    char pwdprompt[] = "Password: ";
    int idinputy, idinputx, pwinputy, pwinputx;
    mvaddstr(
        LINES / 2 - 1,
        (COLS - LOGIN_MAXLEN - strlen(idprompt)) / 2,
        idprompt);
    idinputx = getcurx(stdscr);
    idinputy = getcury(stdscr);
    mvaddstr(
        LINES / 2,
        (COLS - LOGIN_MAXLEN - strlen(pwdprompt)) / 2,
        pwdprompt);
    pwinputx = getcurx(stdscr);
    pwinputy = getcury(stdscr);
    refresh();

    // get user input
    curs_set(1);
    nocbreak();
    echo();
    move(idinputy, idinputx);
    refresh();
    getnstr(outmsg.id, LOGIN_MAXLEN);
    move(pwinputy, pwinputx);
    refresh();
    getnstr(outmsg.pw, LOGIN_MAXLEN);
    cbreak();
    noecho();
    curs_set(0); // invisible

    // send id and pwd
    pktlen = serialize_climsg(&outmsg, out, sizeof(out));
    Writen(sockfd, out, pktlen);
    // wait until server login response
    mvaddstr(
        pwinputy + 1,
        pwinputx - strlen(pwdprompt),
        "Waiting...");
    refresh();

    int n = 0;
    char buf[1];
    struct servmsg inmsg; // no need to prepare buffers
    fd_set readset;
    do {
        FD_ZERO(&readset);
        FD_SET(pipe_fd[0], &readset);
        // 10 sec timeout
        struct timeval t = {.tv_sec = 10};
        n = select(pipe_fd[0]+1, &readset, NULL, NULL, &t);

        switch (n)
        {
        case 0: // timeout
            return TIMEOUT;
            break;
        
        case 1:
            // read msg
            cpy_servmsg(&inmsg, &msg);
            n = read(pipe_fd[0], buf, 1);
            if (n == 0 || buf[0] == '0') {
                return SERVCLOSED;
            }
            break;
        
        default:
            return OTHERERROR;
            break;
        }
    }while (
        !(inmsg.type == SERV_LOGIN && outmsg.type == CLI_LOGIN)
        && !(inmsg.type == SERV_REGISTER && outmsg.type == CLI_REGISTER)
    );
    if (inmsg.success == '0') {
        // draw login fail screen
        switch (inmsg.type) {
            case SERV_LOGIN:
                drawendscreen(L"登入失敗");
                break;
            case SERV_REGISTER:
                drawendscreen(L"註冊失敗");
                break;
            default:
                drawendscreen(L"大失敗");
                break;
        }
        getch();
        goto relogin;
    }
    // login or register success
    strncpy(myid, outmsg.id, sizeof(myid));
    return 0;
}

char menu()
{
    // display menu screen
    drawmenu();

    char key;
    while (1) {
        key = getch();
        switch (key)
        {
        case '1':
        case '2':
        case '3':
        case '4':
            goto end;
        case 'q':
            exitwin(0);
        
        default:
            break;
        }
    }

    end: // send menuopt climsg
    struct climsg outmsg = {.type = MENU, .menuopt = key};
    pktlen = serialize_climsg(&outmsg, out, sizeof(out));
    Writen(sockfd, out, pktlen);

    return key;
}

int twopgame() {

    struct question inq[MAXNUMQ];
    struct player_result inres[MAXPLAYER];
    struct servmsg inmsg = {.questions = inq, .resultdata = inres};
    struct climsg outmsg;
    char buf[1], assigned;
    int n;

    drawwaiting();
    // init2p
    fd_set readset;
    do {
        FD_ZERO(&readset);
        FD_SET(pipe_fd[0], &readset);
        // FD_SET(STDIN_FILENO, &readset);
        select(
            max(pipe_fd[0], STDIN_FILENO)+1,
            &readset,
            NULL,
            NULL,
            NULL
        );
        // has recved msg
        cpy_servmsg(&inmsg, &msg);        
        n = read(pipe_fd[0], buf, 1);
        if (n == 0 || buf[0] == '0') {
            // server closed
            return SERVCLOSED;
        }
    } while (inmsg.type != INIT_2P); // wait until an INIT_2P message
    
    if (inmsg.numq == 0) {
        return OTHERERROR;
    }
    draw2pgame(myid, inmsg.oppid);
    assigned = inmsg.assigned;
    // wait 5 secs then start game

    struct timeval t = {.tv_sec = 1};
    int i = 5;
    drawtime(i);
    while (1) {
        FD_ZERO(&readset);
        FD_SET(pipe_fd[0], &readset);
        n = select(pipe_fd[0]+1, &readset, NULL, NULL, &t);
        if (n == 1) {
            // recved server msg
            cpy_servmsg(&inmsg, &msg);
            n = read(pipe_fd[1], buf, 1);
            if (n == 0 || buf[0] == '0') {
                // server closed
                return SERVCLOSED;
            }
            else if (inmsg.player == '9') {
                return OPPDC;
            }
        }
        else if (n == 0) {
            // timeout
            if (--i == 0) break;
            drawtime(i);
            // reset timer
            t.tv_sec = 1;
            t.tv_usec = 0;
        }
    }

    // start game
    int myscore = 0, oppscore = 0;
    int myflag = 0, oppflag = 0; // when recved both, go to next question
    int anssent = 0, timeout = 0;
    char key;
    for (int i = 0; i < inmsg.numq; i++) {
        // draw problems
        int remaintime  = 10; // 10 sec for each problem
        drawtime(remaintime);
        drawquestion(&inmsg.questions[i]);
        

        myflag = oppflag = 0;
        anssent = 0;
        timeout = 0;

        t.tv_sec = 1;
        t.tv_usec = 0;

        while (1) {

            // timeout every second
            FD_ZERO(&readset);
            FD_SET(pipe_fd[0], &readset);
            if (!anssent || !timeout)
                FD_SET(STDIN_FILENO, &readset);
            n = select(
                max(pipe_fd[0], STDIN_FILENO) + 1,
                &readset,
                NULL,
                NULL,
                timeout ? NULL : &t
            );

            if (n == 0) {
                // 1 sec passed
                
                if (--remaintime == 0) {
                    // timeout, send ans = '0'
                    outmsg.type = ANSWER;
                    outmsg.ans = '0';
                    outmsg.anstime = time(NULL);
                    pktlen = serialize_climsg(&outmsg, out, sizeof(out));
                    Writen(sockfd, out, pktlen);

                    // next question
                    timeout = 1;
                }
                drawtime(remaintime);
                t.tv_sec = 1;
                t.tv_usec = 0;
            }
            else if (n > 0) {
                // check fds
                if (FD_ISSET(STDIN_FILENO, &readset)) {
                    // user input, send to server
                    key = getch();
                    switch (key) {
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                            outmsg.type = ANSWER;
                            outmsg.ans = key;
                            outmsg.anstime = time(NULL);
                            // send
                            pktlen = serialize_climsg(&outmsg, out, sizeof(out));
                            Writen(sockfd, out, pktlen);
                        default:
                            break; // ignore
                    }
                    anssent = 1;
                }
                if (FD_ISSET(pipe_fd[0], &readset)) {
                    // recved servmsg, read the message out
                    cpy_servmsg(&inmsg, &msg);
                    n = read(pipe_fd[0], buf, 1);
                    if (n == 0 || buf[0] == '0') {
                        // serv closed
                        return SERVCLOSED;
                    }
                    // process servmsg
                    if (inmsg.type == EVAL_ANS) {
                        // only process type EVAL_ANS
                        if (inmsg.player == '9') {
                            return OPPDC;
                        }
                        if (inmsg.player == assigned) {
                            myscore += inmsg.scorechange;
                            updatescore(1, myscore, key, inmsg.correct);
                            myflag = 1;
                        }
                        else {
                            oppscore += inmsg.scorechange;
                            updatescore(0, oppscore, 0, inmsg.correct);
                            oppflag = 1;
                        }
                    }
                }
                if (myflag && oppflag) {
                    // got both responses, move to next question
                    // display correct answer
                    updateans(key, inmsg.oppans, inmsg.ans);
                    // wait 5 secs then next question
                    for (int wait = 5; wait > 0; wait--) {
                        drawtime(wait);
                        sleep(1);
                    }
                    break;
                } 
            }
        }
    }

    // wait until display result screen
    FD_ZERO(&readset);
    FD_SET(pipe_fd[0], &readset);
    // 10 sec timeout
    t.tv_sec = 10;
    t.tv_usec = 0;
    n = select(pipe_fd[0]+1, &readset, NULL, NULL, &t);
    if (n == 1) {
        cpy_servmsg(&inmsg, &msg);
        n = read(pipe_fd[0], buf, 1);
        if (n == 0 || buf[0] == '0') {
            // server closed
            return SERVCLOSED;
        }
        drawresult(inmsg.resultdata, assigned);
    }
    else if (n == 0) {
        // timed out
        return TIMEOUT;
    }
    getch();
    return 0;
}

int replay() {
    drawreplay();
    char key;
    while (1) {
        key = getch();
        switch (key)
        {
        case 'r':
            return 1;
            break;
        case 'q':
            exitwin(0);
        
        default:
            break;
        }
    }
}

void checkerr(int ret) {
    switch (ret)
    {
    case SERVCLOSED:
        endscreen(L"伺服器已斷線");
    case TIMEOUT:
        endscreen(L"Timed out");
    case OTHERERROR:
        endscreen(L"Unknown error has occured");
    case OPPDC:
        drawendscreen(L"對方已斷線");
        getch();
    default:
        break;
    }
}

int main(int argc, char **argv)
{
    

    if (argc != 2)
        err_quit("usage: client <IPaddress>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr = {AF_INET};
    servaddr.sin_port = htons(SERV_PORT + 3);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

    pthread_t readthread;
    pthread_create(&readthread, NULL, reader, NULL);
    pthread_detach(readthread);

    // start client logic

    fp = fopen("checkproject.log", "a");
    initscreen();
    Pipe(pipe_fd);

    int ret;
    ret = login();
    checkerr(ret);

    menu:
    char menuopt = menu();
    switch (menuopt) {
        case '2':
            ret = twopgame();
            break;
        default:
            endscreen(L"Not yet implemented");
    }

    checkerr(ret);
    if (replay()) goto menu;

    endscreen(NULL);
}