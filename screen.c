#include "screen.h"

void initscreen() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak(); // disable line buffer
    noecho(); // disable echoing of input
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE); // enable arrow key
}

void drawmenu() {
    clear();
    box(stdscr, 0, 0);
    const wchar_t *MENUOPTIONS[] = {
        L"單人遊戲",
        L"雙人對戰",
        L"多人對戰",
        L"排行榜"
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

void draw2pgame() {
    clear();
    box(stdscr, 0, 0);

    mvaddwstr(1, 1, L"您的分數");
    mvaddwstr(1, COLS-width(L"對手分數"), L"對手分數");
    mvaddch(2, 1, '0');
    mvaddch(2, COLS-2, '0');
    
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
    int top, left, right;
    top = 3;
    left = COLS / 4;
    right = COLS - left;

    // draw q->q
    cchar_t buf[Q_MAXLEN];
    setcchar(buf, q->q, A_NORMAL, 0, NULL);
    move(top, left);
    int x, y;
    for (int i = 0; i < wcslen(q->q); i++) {
        getyx(stdscr, y, x);
        if (x > right) {
            // next line
            move(y+1, left);
        }
        add_wch(buf + i);
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

    refresh();

}

/*
┌─────────────────────────────────────────────────────────────┐
│您的分數                      time                     對手分數│
│score                                                   score│
│                 問題問題問題問題問題問題問題問題問題             │
│                 問題問題問題問題問題問題問題問題問題             │
│                                                             │
│                                                             │
│                                                             │
│                 [1] 選項  　　　　　　                        │
│                 [2] 選項  　　　　　　                        │
│                 [3] 選項  　　　　　　                        │
│                 [4] 選項 　　　　　　                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
*/

void updatescore(char player, int score, char ans, char correct) {
    // player = '0' is on left
    char buf[32];
    wchar_t wcharbuf[32];
    move(1, 0);
    clrtoeol();
    snprintf(buf, sizeof(buf), "%d", score);
    swprintf(
        wcharbuf,
        32,
        L"選擇選項%c：%ls",
        ans, 
        (correct == '1') ? L"答對" : L"答錯");

    if (player == '0') {
        mvaddstr(2, 1, buf);
        mvaddwstr(3, 1, wcharbuf);
    }
    else if (player == '1') {
        mvaddstr(2, COLS-strlen(buf), buf);
        mvaddwstr(3, COLS-width(wcharbuf), wcharbuf);
    }
    box(stdscr, 0, 0);
    refresh();
}

void drawresult(const struct player_result* res) {
    clear();
    box(stdscr, 0, 0);

    wchar_t* str = L"對戰結果";
    mvaddwstr(1, (COLS-width(str))/2 ,str);

    wchar_t buf[32];
    swprintf(buf, 32, L"你的分數：%d", res[0].score);
    mvaddwstr(3, (COLS-width(buf))/2, buf);
    swprintf(buf, 32, L"獲得金幣：%d", res[0].coin);
    mvaddwstr(4, (COLS-width(buf))/2, buf);
    swprintf(buf, 32, L"對手分數：%d", res[1].score);
    mvaddwstr(6, (COLS-width(buf))/2, buf);
    swprintf(buf, 32, L"獲得金幣：%d", res[1].coin);
    mvaddwstr(7, (COLS-width(buf))/2, buf);

    str = L"按任意鍵以繼續";
    mvaddwstr(LINES-1, (COLS-width(str))/2, str);
    refresh();
}

int width(const wchar_t* s) {
    return wcswidth(s, wcslen(s));
}