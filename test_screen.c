#include "screen.h"

int main() {
    initscreen();
    drawmenu();
    getch();
    drawendscreen(L"hi");
    getch();
    endwin();
}