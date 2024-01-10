#include <string.h>
#include <wchar.h>
#include <pthread.h>
#include "common.h"
#include "screen.h"

static int sockfd;

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

void *reader(void *arg)
{
    char buf[MAXLINE];
    ssize_t n;
    int ret;
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
            n = select(
                pipe_fd[0] + 1,
                &chkset,
                NULL,
                NULL,
                &t);
        } while (n == 1);
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
    char id[LOGIN_MAXLEN], pw[LOGIN_MAXLEN];
    nocbreak();
    echo();
    move(idinputy, idinputx);
    refresh();
    getnstr(id, LOGIN_MAXLEN);
    move(pwinputy, pwinputx);
    refresh();
    getnstr(pw, LOGIN_MAXLEN);
    cbreak();
    noecho();
    curs_set(0); // invisible

    // send id and pwd
    strcpy(out, id);
    Writen(sockfd, out, strlen(out));
    strcpy(out, pw);
    Writen(sockfd, out, strlen(out));
    // wait until server response
    mvaddstr(
        pwinputy + 1,
        pwinputx - strlen(pwdprompt),
        "Waiting...");
    refresh();

    int n = 0;
    // pthread_mutex_lock(&msg_mutex);
    // while (!hasmsg) {
    //     err = pthread_cond_timedwait(&msg_cond, &msg_mutex, &t);
    //     if (err) break;
    //     else {
    //         hasmsg = 0;
    //     }
    // }
    // pthread_mutex_unlock(&msg_mutex);
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(pipe_fd[0], &readset);
    // 10 sec timeout
    struct timeval t = {.tv_sec = 10};
    n = select(pipe_fd[0]+1, &readset, NULL, NULL, &t);

    switch (n)
    {
    case 0: // timeout
        return -1;
        break;
    
    case 1:
        char buf[1];
        read(pipe_fd[0], buf, 1);
        // read msg
        break;
    
    default:
        return -2;
        break;
    }
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
            exit(0);
        
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
    char buf[1];
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
            return -1;
        }
    } while (inmsg.type != INIT_2P); // wait until an INIT_2P message
    
    draw2pgame();
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
            n = read(pipe_fd[1], buf, 1);
            if (n == 0 || buf[0] == '0') {
                // server closed
                return -1;
            }
            // else ignore
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

    int myscore = 0, oppscore = 0;
    int myflag = 0, oppflag = 0; // when recved both, go to next question
    char key;
    // start game
    for (int i = 0; i < inmsg.numq; i++) {
        // draw problems
        int remaintime  = 10; // 10 sec for each problem
        drawtime(remaintime);
        drawquestion(&inmsg.questions[i]);
        // timeout every second
        t.tv_sec = 1;
        t.tv_usec = 0;

        myflag = oppflag = 0;

        while (1) {
            FD_ZERO(&readset);
            FD_SET(pipe_fd[0], &readset);
            FD_SET(STDIN_FILENO, &readset);
            n = select(
                max(pipe_fd[0], STDIN_FILENO),
                &readset,
                NULL,
                NULL,
                &t
            );

            if (n == 0) {
                // 1 sec passed
                if (--remaintime == 0) {
                    // timeout, send ans = '0'
                    outmsg.type = ANSWER;
                    outmsg.ans = '0';
                    outmsg.anstime = time(NULL);
                    pktlen = serialize_climsg(&outmsg, out, sizeof(out));
                    write(sockfd, out, pktlen);

                    // next question
                    break;
                }
                drawtime(remaintime);
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
                }
                if (FD_ISSET(pipe_fd[0], &readset)) {
                    // recved servmsg, read the message out
                    cpy_servmsg(&inmsg, &msg);
                    n = read(pipe_fd[0], buf, 1);
                    if (n == 0 || buf[0] == '0') {
                        // serv closed
                        return -1;
                    }
                    // process servmsg
                    if (inmsg.type == EVAL_ANS) {
                        // only process type EVAL_ANS
                        if (inmsg.player == '0') {
                            myscore += inmsg.scorechange;
                            updatescore(inmsg.player, myscore, key, inmsg.correct);
                            myflag = 1;
                        }
                        else if (inmsg.player == '1') {
                            oppscore += inmsg.scorechange;
                            updatescore(inmsg.player, oppscore, '0', inmsg.correct);
                            oppflag = 1;
                        }

                        if (myflag && oppflag) {
                            // got both responses, move to next question
                            // #TODO display correct answer

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
            return -1;
        }
        drawresult(inmsg.resultdata);
    }
    else if (n == 0) {
        // timed out
        return -2;
    }
    getch();
    return 0;
}

void endscreen(const wchar_t* endmsg) {
    drawendscreen(endmsg);
    getch();
    endwin();
    exit(0);
}

int main(int argc, char **argv)
{
    initscreen();
    Pipe(pipe_fd);

    // // const wchar_t wstr[] = L"早安";

    // // mvaddwstr(0, 0, wstr);

    // while (1) {
    //     int key = getch();
    //     clear();
    //     mvaddch(0, 0, key);
    //     refresh();
    // }

    // getch();

    // if (argc != 2)
    //     err_quit("usage: client <IPaddress>");

    // sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // struct sockaddr_in servaddr = {AF_INET};
    // servaddr.sin_port = htons(SERV_PORT + 3);
    // Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    // Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

    // pthread_t readthread;
    // // pthread_t writethread;
    // pthread_create(&readthread, NULL, reader, NULL);
    // pthread_detach(readthread);
    // pthread_create(&writethread, NULL, writer, NULL);
    int ret;
    ret = login();
    if (ret == -1) {
        endscreen(L"Timed out");
    }
    else if (ret == -2) {
        endscreen(L"Unknown error has occured");
    }

    char menuopt = menu();
    switch (menuopt) {
        case '2':
            ret = twopgame();
            break;
        default:
            endscreen(L"Not yet implemented");
    }

    switch (ret)
    {
    case -1:
        endscreen(L"伺服器已斷線");
        break;
    case 0:
        endscreen(L"成功結束");
        break;
    case -2:
        endscreen(L"Timed out");
    
    default:
        break;
    }


    endscreen(NULL);
}