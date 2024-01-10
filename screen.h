#define _XOPEN_SOURCE 600
#include <ncursesw/curses.h>
#include <locale.h>
#include "common.h"

int width(const wchar_t* s);

// Inits the screen.
void initscreen();

void drawloginmenu();

// draws the menu.
void drawmenu();

// draws waiting screen (waiting for another player)
void drawwaiting();

void drawreplay();

void draw2pgame(const char* myid, const char* oppid);

void drawtime(int t);

void drawquestion(const struct question* q);

// update score and show correct
// if not self, `ans` won't be used
void updatescore(int isself, int score, char ans, char correct);

void updateans(char myans, char oppans, char trueans);

void drawresult(const struct player_result* res);

// draws endscreen with endmsg
void drawendscreen(const wchar_t* endmsg);

void endscreen(const wchar_t* endmsg);

void exitwin(int status);