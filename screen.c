#include "screen.h"

void initscreen() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak(); // disable line buffer
    noecho(); // disable echoing of input
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE); // enable arrow key
    curs_set(0); // set cursor invisible
}

void drawloginmenu() {
    clear();
    box(stdscr, 0, 0);
    wchar_t* buf = L"[1] 登入";
    mvaddwstr(LINES/2, (COLS-width(buf))/2, buf);
    buf = L"[2] 註冊";
    mvaddwstr(LINES/2+1, (COLS-width(buf))/2, buf);
    buf = L"[Q] 退出";
    mvaddwstr(LINES-1, (COLS-width(buf))/2, buf);
    refresh();
}

void drawmenu() {
    clear();
    box(stdscr, 0, 0);
    const wchar_t *MENUOPTIONS[] = {
        L"單人遊戲",
        L"雙人對戰",
        L"多人對戰",
        L"排行榜",
        L"上傳題目"
    };
    const size_t optionnum = sizeof(MENUOPTIONS) / sizeof(void *);

    wchar_t buf[Q_MAXLEN];
    for (int i = 0; i < optionnum; i++)
    {
        swprintf(buf,
                 sizeof(buf) / sizeof(wchar_t),
                 L"[%d] %ls",
                 i + 1,
                 MENUOPTIONS[i]);
        mvaddwstr((LINES - optionnum) / 2 + i,
                  (COLS - width(buf)) / 2,
                  buf);
    }
    // quit option
    swprintf(buf,
             sizeof(buf) / sizeof(wchar_t),
             L"[Q] %ls",
             L"退出");
    mvaddwstr(LINES - 1,
              (COLS - width(buf)) / 2,
              buf);
    refresh();
}

void drawendscreen(const wchar_t* endmsg) {
    clear();
    box(stdscr, 0, 0);
    mvaddwstr(LINES/2, (COLS-width(endmsg))/2, endmsg);
    // quit message
    wchar_t* quitmsg = L"按任意鍵以退出";
    mvaddwstr(LINES-1, (COLS-width(quitmsg))/2, quitmsg);
    refresh();
}

void drawwaiting() {
    clear();
    box(stdscr, 0, 0);
    wchar_t* waitmsg = L"等待對手加入...";
    mvaddwstr((LINES-1)/2, (COLS-width(waitmsg))/2, waitmsg);
    refresh();
}

void draw2pgame(const char* myid, const char* oppid) {
    clear();
    box(stdscr, 0, 0);

    mvaddwstr(1, 1, L"您的分數");
    mvaddwstr(1, COLS-width(L"對手分數")-1, L"對手分數");
    mvaddch(2, 1, '0');
    mvaddch(2, COLS-2, '0');
    // draw id
    mvaddstr(LINES-2, 1, myid);
    mvaddstr(LINES-2, COLS-1-strlen(oppid), oppid);
    
    refresh();
}

void drawtime(int t) {
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", t);
    mvaddstr(1, (COLS-4)/2, "    ");
    mvaddstr(1, (COLS-strlen(buf))/2, buf);
    refresh();
}

void drawquestion(const struct question* q) {
    // clear();

    // clear last answer
    wchar_t* buf = L"正確答案：[1]";
    move(LINES - 2, (COLS-width(buf))/2);
    for (int i = 0; i < width(buf); i++) {
        addch(' ');
    }

    // clear 3, 4th row
    move(3, 0);
    clrtoeol();
    move(4, 0);
    clrtoeol();

    int top, btm, left, right;
    top = 3;
    btm = LINES - 2 - OPTIONNUM;
    left = COLS / 4;
    right = COLS - left;

    // clear top->btm, left->right
    move(top, left);
    for (int y = top; y < btm; y++) {
        for (int x = left; x < right; x++) {
            mvaddch(y, x, ' ');
        }
    }
    // draw q->q
    move(top, left);
    int x, y;
    for (int i = 0; i < wcslen(q->q); i++) {
        getyx(stdscr, y, x);
        if (x > right) {
            // next line
            move(y+1, left);
        }
        addnwstr(q->q + i, 1);
    }

    // clear options on screen
    for (int i = 0; i < OPTIONNUM; i++) {
        move(btm + i, 0);
        clrtoeol();
    }

    // draw options (assume they are short enough)
    wchar_t wcharbuf[Q_MAXLEN+10];
    move(LINES - 2 - OPTIONNUM, left);
    for (int i = 0; i < OPTIONNUM; i++) {
        swprintf(
            wcharbuf, 
            sizeof(wcharbuf) / sizeof(wchar_t),
            L"[%d] %ls",
            i + 1,
            q->option[i]);
        addwstr(wcharbuf);
        move(getcury(stdscr)+1, left);
    }

    box(stdscr, 0, 0);
    refresh();

}

/*
┌─────────────────────────────────────────────────────────────┐
│您的分數                      time                     對手分數│
│score                                                   score│
│選擇選項n         問題問題問題問題問題問題問題問題問題        已作答│
│答對             問題問題問題問題問題問題問題問題問題          答對│
│                                                             │
│                                                             │
│                                                             │
│                 [1] 選項  　　　　　　                        │
│                 [2] 選項  　　　　　　                        │
│                 [3] 選項  　　　　　　                        │
│                 [4] 選項 　　　　　　                         │
│ID                                                         ID│
└─────────────────────────────────────────────────────────────┘
*/

void updatescore(int isself, int score, char ans, char correct) {

    char scorebuf[32];
    wchar_t wcharbuf[32];
    // move(2, 0);
    // clrtoeol();
    snprintf(scorebuf, sizeof(scorebuf), "%05d", score);

    if (isself) {
        swprintf(
            wcharbuf,
            32,
            L"選擇選項%c",
            ans);
        mvaddstr(2, 1, scorebuf);
        mvaddwstr(3, 1, wcharbuf);
        mvaddwstr(4, 1, (correct == '1') ? L"答對" : L"答錯");
    }
    else {
        mvaddstr(2, COLS-strlen(scorebuf)-1, scorebuf);
        swprintf(
            wcharbuf,
            32,
            L"%ls",
            L"已選擇選項");
        mvaddwstr(3, COLS-width(wcharbuf)-1, wcharbuf);
        swprintf(
            wcharbuf,
            32,
            L"%ls",
            (correct == '1') ? L"答對" : L"答錯");
        mvaddwstr(4, COLS-width(wcharbuf)-1, wcharbuf);
    }
    box(stdscr, 0, 0);
    refresh();
}

void updateans(char myans, char oppans, char trueans) {
    // draw trueans
    wchar_t buf[32];
    swprintf(buf, 32, L"正確答案：[%c]", trueans);
    mvaddwstr(LINES-2, (COLS-width(buf))/2, buf);
    // draw myans
    swprintf(buf, 32, L"選擇選項%c", myans);
    mvaddwstr(3, 1, buf);
    wchar_t* right = (myans == trueans) ? L"答對" : L"答錯";
    mvaddwstr(4, 1, right);
    // draw oppans
    swprintf(buf, 32, L"選擇選項%c", oppans);
    mvaddwstr(3, COLS-width(buf)-1, buf);
    right = (oppans == trueans) ? L"答對" : L"答錯";
    mvaddwstr(4, COLS-width(right)-1, right);
    
    refresh();
}

void drawresult(const struct player_result* res, char assigned) {
    clear();
    box(stdscr, 0, 0);

    int self = assigned - '0';
    wchar_t* str = L"對戰結果";
    mvaddwstr(1, (COLS-width(str))/2 ,str);

    
    wchar_t buf[32];
    swprintf(buf, 32, L"你的分數：%d", res[self].score);
    mvaddwstr(3, (COLS-width(buf))/2, buf);
    swprintf(buf, 32, L"獲得金幣：%d", res[self].coin);
    mvaddwstr(4, (COLS-width(buf))/2, buf);
    self = !self;
    swprintf(buf, 32, L"對手分數：%d", res[self].score);
    mvaddwstr(6, (COLS-width(buf))/2, buf);
    swprintf(buf, 32, L"獲得金幣：%d", res[self].coin);
    mvaddwstr(7, (COLS-width(buf))/2, buf);

    str = L"按任意鍵以繼續";
    mvaddwstr(LINES-1, (COLS-width(str))/2, str);
    refresh();
}

void drawreplay() {
    clear();
    box(stdscr, 0, 0);
    wchar_t* buf = L"[R] 回到主畫面";
    mvaddwstr(LINES/2, (COLS-width(buf))/2, buf);
    buf = L"[Q] 退出";
    mvaddwstr(LINES/2+1, (COLS-width(buf))/2, buf);
    refresh();
}

int width(const wchar_t* s) {
    return wcswidth(s, wcslen(s));
}

void endscreen(const wchar_t* endmsg) {
    drawendscreen(endmsg);
    getch();
    exitwin(0);
}
void exitwin(int status) {
    endwin();
    exit(status);
}