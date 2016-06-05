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
// interface.c: the second level of abstraction for my SOMETHING AWESOME
//
// Abstraction of curses library functions into custom functions
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

static int getIntWithMenu (WINDOW* window, char **options, int numOptions);
static int getIntWithInput (WINDOW *window, char *prompt, int min, int max);
static void setHeading (WINDOW *header, char *message);

static int isValidName (char *name);
static int isValidChar (char c);

// Display the splash screen

void welcome (int screenHeight, int screenWidth) {

    WINDOW *splash = newwin(screenHeight-SPLASH_PADDING,
        screenWidth-SPLASH_PADDING, 4, 4);
    wbkgd(splash, COLOR_PAIR(1));

    // char *splashBanner =
    // "               o     |  o\n"
    // "     _  _  _       __|      __,  _   _  _\n"
    // "    / |/ |/ |  |  /  |  |  /  | |/  / |/ |\n"
    // "      |  |  |_/|_/\\_/|_/|_/\\_/|/|__/  |  |_/\n"
    // "                             /|\n"
    // "                             \\|\n";

    char *splashBanner =
    "                   _|        _|  _|\n"
    "   _|_|_|  _|_|          _|_|_|        _|_|_|    _|_|    _|_|_|\n"
    "   _|    _|    _|  _|  _|    _|  _|  _|    _|  _|_|_|_|  _|    _|\n"
    "   _|    _|    _|  _|  _|    _|  _|  _|    _|  _|        _|    _|\n"
    "   _|    _|    _|  _|    _|_|_|  _|    _|_|_|    _|_|_|  _|    _|\n"
    "                                           _|\n"
    "                                         _|_|\n";

    wattron(splash, COLOR_PAIR(2));
    mvwprintw(splash, (screenHeight-SPLASH_PADDING)/2 - 3, 0, "%s",
              splashBanner);
    wattron(splash, COLOR_PAIR(1));
    mvwprintw(splash, screenHeight-SPLASH_PADDING-2, 2,
              "\tWelcome! Press any key to continue");
    wgetch(splash);
    delwin(splash);
    refresh();
}

// Get a filename from the user and open a midi file with that name

FILE *initialiseFile (WINDOW *header) {

    char filename[MAX_FILENAME + FILE_EXTENSION_SIZE] = { 0 };
    setHeading(header, "Enter file name: ");
    echo();
    wattron(header, COLOR_PAIR(2));
    wgetstr(header, filename);
    wattron(header, COLOR_PAIR(1));
    noecho();

    while (!isValidName(filename)) {
        setHeading(header, "Invalid name! Please try again: ");
        echo();
        wattron(header, COLOR_PAIR(2));
        wgetstr(header, filename);
        wattron(header, COLOR_PAIR(1));
        noecho();
    }

    strncat(filename, FILE_EXTENSION, FILE_EXTENSION_SIZE);

    FILE *file;
    file = fopen(filename, "wb");
    assert ((file!=NULL) && "Cannot open file :(");

    return file;
}

// Boot up the graphical system

void initialiseCurses (void) {
    initscr();              // start curses
    cbreak();               // allow uncooked key input and also ctrl-c
    noecho();               // stop the terminal from printing input
    curs_set(0);            // make the cursor invisible
    keypad(stdscr, TRUE);   // allow useful action keys like arrow keys
    start_color();          // let there be colours!
    init_pair(1, COLOR_YELLOW, COLOR_BLUE);
    init_pair(2, COLOR_RED, COLOR_BLUE);
}

// Display a nice farewell message

void farewell (WINDOW *header, int screenHeight, int screenWidth) {

    setHeading (header, "Your midi file has been fabricated. Enjoy!");

    WINDOW *splash = newwin(screenHeight-SPLASH_PADDING,
                            screenWidth-SPLASH_PADDING, 4, 4);
    wbkgd(splash, COLOR_PAIR(1));

    mvwprintw(splash, screenHeight-SPLASH_PADDING-2, 2,
              "\tThank you for using midigen. Press any key to exit...");
    wgetch(splash);
    delwin(splash);
    refresh();
}

// Find out what the user wants to do

int getMode (WINDOW *header, WINDOW *menuWindow) {

    setHeading (header, "What do you want to do?");
    char *modeChoices[NUM_MODES] = MODE_NAMES;
    int mode = getIntWithMenu (menuWindow, modeChoices, NUM_MODES);

    return mode;
}

// Get all the user parameters and return them in one big struct

songdata getParameters (WINDOW* header, WINDOW *menuWindow, int mode) {

    songdata newsong = malloc (sizeof(struct _songData));

    newsong->tempo = DEFAULT_TEMPO;
    if (mode == MODE_CHOR) {
        newsong->numPlays = getIntWithInput (header,
            "Enter number of notes or rests: ", 1, MAX_CHORDS);
    } else {
        newsong->numPlays = getIntWithInput (header,
            "Enter number of notes or rests: ", 1, MAX_EVENTS);
    }

    newsong->stepChance = getIntWithInput (header,
        "Enter chance of stepwise motion (%): ", 0, 100);
    newsong->restChance = 0;
    if (mode != MODE_CHOR) {
        newsong->restChance = getIntWithInput (header,
            "Enter chance of rest (%): ", 0, 100);
    }

    char *rhythmChoices[2] = { "Avant Garde", "Boring"};
    setHeading(header, "How do you like your rhythms?");
    newsong->isBoring = getIntWithMenu(menuWindow, rhythmChoices, 2);

    char *rootChoices[OCTAVE] = NOTE_NAMES;
    setHeading(header, "Choose a root note");
    newsong->root = getIntWithMenu(menuWindow, rootChoices, OCTAVE);

    unsigned int baseOctave = getIntWithInput (header,
        "How many octaves high should that be? ", 0, MAX_OCTAVE);
    newsong->root += baseOctave * OCTAVE;

    char *scaleChoices[NUM_SCALES] = SCALE_NAMES;
    setHeading(header, "Choose a scale");
    newsong->scaleID = getIntWithMenu(menuWindow, scaleChoices, NUM_SCALES);

    newsong->range = getIntWithInput (header,
        "Enter range in octaves: ", 1, MAX_RANGE);

    newsong->harmony = -1;
    newsong->voices = -1;
    if (mode == MODE_HARM) {
        setHeading (header, "What kind of harmony would you like?");
        char *harmChoices[SCALE_DEFAULT_NOTES] = HARMONY_NAMES;
        newsong->harmony = 1 + getIntWithMenu (menuWindow, harmChoices,
            SCALE_DEFAULT_NOTES);
    } else if (mode == MODE_CHOR) {
        newsong->voices = getIntWithInput (header,
            "How many voices would you like? ", 1, MAX_VOICES);
    }

    werase(header);
    werase(menuWindow);
    wrefresh(header);
    wrefresh(menuWindow);
    return newsong;
}

// Display a list of selections from a given list of strings, and
// return what the user chooses

static int getIntWithMenu (WINDOW* window, char **options, int numOptions) {

    ITEM **menuItems = malloc (sizeof(ITEM *) * (numOptions + 1));

    int i = 0;
    while (i < numOptions) {
        menuItems[i] = new_item(options[i], 0);
        i++;
    }
    menuItems[i] = NULL;

    MENU *menu= new_menu((ITEM **)menuItems);

    set_menu_win(menu, window);
    set_menu_mark(menu, " > ");
    post_menu(menu);
    wrefresh(window);

    int c = wgetch(window);
    int choice = 0;
    int chosen = FALSE;

    while(c != QUIT && !chosen) {

        if (c == 10) {
            chosen = TRUE;
        } else {
            if (c == KEY_DOWN && choice < numOptions-1) {
                menu_driver(menu, REQ_DOWN_ITEM);
                choice++;
            } else if (c == KEY_UP && choice > 0) {
                menu_driver(menu, REQ_UP_ITEM);
                choice--;
            }
            wrefresh(window);
            c = wgetch(window);
        }
    }
    werase(window);
    wrefresh(window);

    unpost_menu(menu);
    free_menu(menu);
    menu = NULL;
    i = 0;
    while (i < numOptions+1) {
        free_item(menuItems[i]);
        menuItems[i] = NULL;
        i++;
    }
    free(menuItems);
    menuItems = NULL;

    return choice;
}

// Ask the user the prompt, and return the integer of the text they input

static int getIntWithInput (WINDOW *window, char *prompt, int min, int max) {

    char string[MAX_FILENAME] = { 0 };
    setHeading(window, prompt);
    echo();
    wattron(window, COLOR_PAIR(2));
    wgetstr(window, string);
    wattron(window, COLOR_PAIR(1));
    noecho();
    int x = atoi (string);

    while (x < min || x > max) {
        char errorMessage[60];
        snprintf(errorMessage, 60,
            "This value must be between %d and %d. Please try again: ",
            min, max);
        setHeading(window, errorMessage);
        echo();
        wattron(window, COLOR_PAIR(2));
        wgetstr(window, string);
        wattron(window, COLOR_PAIR(1));
        noecho();
        x = atoi (string);
    }

    return x;
}

// Set the text of a given window

static void setHeading (WINDOW *header, char *message) {
    werase(header);
    mvwprintw(header, 1, 2, "%s", message);
    wrefresh(header);
}

// See if a filename is valid (length is ok and contains valid characters)

static int isValidName (char *name) {

    int isValid = strlen(name) > 0 && strlen(name) <= MAX_FILENAME;
    int i = 0;
    while (name[i] != '\0' && isValid) {
        if (!isValidChar(name[i])) {
            isValid = FALSE;
        }
        i++;
    }
    return isValid;
}

// See if a character is valid

static int isValidChar (char c) {

    return (c >= '0' && c <= '9') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= 'a' && c <= 'z') ||
    c == '.' || c == '-' || c == '_';
}
