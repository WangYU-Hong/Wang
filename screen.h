#define _XOPEN_SOURCE 600
#include <ncursesw/curses.h>
#include <locale.h>
#include "common.h"

int width(const wchar_t* s);

// Inits the screen.
void initscreen();

// draws the menu.
void drawmenu();

// draws waiting screen (waiting for another player)
void drawwaiting();

// draws endscreen with endmsg
void drawendscreen(const wchar_t* endmsg);

void draw2pgame();

void drawtime(int t);

void drawquestion(const struct question* q);

// update score and show correct
// `player == '0'` means self
void updatescore(char player, int score, char ans, char correct);

void drawresult(const struct player_result* res);