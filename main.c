//
// Nelson Odins-Jones, z5122504, nelsonoj
//
//                    _|        _|  _|
//    _|_|_|  _|_|          _|_|_|        _|_|_|    _|_|    _|_|_|
//    _|    _|    _|  _|  _|    _|  _|  _|    _|  _|_|_|_|  _|    _|
//    _|    _|    _|  _|  _|    _|  _|  _|    _|  _|        _|    _|
//    _|    _|    _|  _|    _|_|_|  _|    _|_|_|    _|_|_|  _|    _|
//                                            _|
//                                          _|_|
//
// main.c: the highest level of abstraction for my SOMETHING AWESOME
//
// Generate valid midi files with C.
//

#include <stdlib.h>
#include <curses.h>
#include <menu.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "Event.h"
#include "music.h"
#include "interface.h"

int main (int argc, char *argv[]) {

    srand(time(NULL));

    initialiseCurses();
    int screenWidth = 0;
    int screenHeight = 0;
    getmaxyx(stdscr, screenHeight, screenWidth);

    welcome(screenHeight, screenWidth);

    WINDOW *header = newwin(HEADER_HEIGHT, screenWidth, 0, 0);
    wbkgd(header, COLOR_PAIR(1));

    WINDOW *menuWindow = newwin(OCTAVE, screenWidth, HEADER_HEIGHT + 2, 0);
    keypad(menuWindow, TRUE);

    int mode = getMode(header, menuWindow);

    FILE *output = initialiseFile(header);

    if (mode == MODE_DEMO) {
        demoOne (output);
    } else {
        songdata newsong = getParameters(header, menuWindow, mode);
        if (mode == MODE_MONO) {
            singleNoteMelody (output, newsong);
        } else if (mode == MODE_HARM) {
            melodyWithHarmony (output, newsong);
        } else if (mode == MODE_CHOR) {
            chords (output, newsong);
        }
    }

    delwin(menuWindow);
    farewell(header, screenHeight, screenWidth);
    delwin(header);

    endwin();

    return EXIT_SUCCESS;
}
