//
//  user.c
//
//
//  Created by Nelson Odins-Jones on 23/05/2016.
//
// TODO: modes (single, harmony, chords, demo)
// TODO: clean up everything
// TODO: put limits on everything DONE 27/5
// TODO: add tame rhythm mode DONE 27/5

#include <stdlib.h>
#include <curses.h>
#include <menu.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "Event.h"
#include "music.h"

#define MAX_EVENTS 9999
#define MAX_OCTAVE 9
#define MAX_RANGE (HIGHEST_NOTE - OCTAVE*baseOctave) / OCTAVE

#define MAX_FILENAME 14
#define FILE_EXTENSION ".mid"
#define FILE_EXTENSION_SIZE 4

#define HEADER_HEIGHT 3
#define QUIT 'q'

// Modes
#define NUM_MODES 3
#define MODE_MONO 0
#define MODE_HARM 1
#define MODE_DEMO 2
#define MODE_NAMES {"Generate a monophonic melody", \
                    "Generate a melody with a harmony", \
                    "Run the demo" }

#define HARMONY_NAMES { "Seconds", "Thirds (classic)", "Fourths", \
                        "Fifths", "Sixths", "Sevenths", "Octaves" }

typedef struct _songData {
        unsigned int tempo;
        unsigned int numPlays;
        unsigned int stepChance;
        unsigned int restChance;
        unsigned int isBoring;
        unsigned int root;
        unsigned int range;
        unsigned int scaleID;
}*songdata;

static void welcome (int screenHeight, int screenWidth);
static int getMode (WINDOW *header, WINDOW *menuWindow);
static FILE *initialiseFile (WINDOW *header);
static int isValidName (char *name);
static int isValidChar (char c);

static void initialiseCurses (void);
static songdata getParameters (WINDOW* header, WINDOW *menuWindow);
static void singleNoteMelody (FILE* output, songdata newsong);
static void melodyWithHarmony (FILE* output, songdata newsong, int interval);

static int getIntWithMenu (WINDOW* window, char **options, int numOptions);
static int getIntWithInput (WINDOW *window, char *prompt, int min, int max);
static void setHeading (WINDOW *header, char *message);
static void farewell (WINDOW *header, int screenHeight, int screenWidth);

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

    if (mode == MODE_MONO) {
        songdata newsong = getParameters(header, menuWindow);
        singleNoteMelody (output, newsong);
    } else if (mode == MODE_HARM) {
        songdata newsong = getParameters(header, menuWindow);

        setHeading (header, "What kind of harmony would you like?");
        char *harmChoices[SCALE_DEFAULT_NOTES] = HARMONY_NAMES;
        int interval = 1 + getIntWithMenu (menuWindow, harmChoices, SCALE_DEFAULT_NOTES);

        melodyWithHarmony (output, newsong, interval);
    } else if (mode == MODE_DEMO){
        demoOne (output);
    }

    delwin(menuWindow);
    farewell(header, screenHeight, screenWidth);
    delwin(header);

    endwin();

    return EXIT_SUCCESS;
}

static void welcome (int screenHeight, int screenWidth) {

    WINDOW *splash = newwin(screenHeight-8, screenWidth-8, 4, 4);
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
    mvwprintw(splash, (screenHeight-8)/2 - 3, 0, "%s", splashBanner);
    wattron(splash, COLOR_PAIR(1));
    mvwprintw(splash, screenHeight-8-2, 2, "\tWelcome! Press any key to continue");
    wgetch(splash);
    delwin(splash);
    refresh();
}

static void farewell (WINDOW *header, int screenHeight, int screenWidth) {

    setHeading (header, "Your midi file has been fabricated. Enjoy!");

    WINDOW *splash = newwin(screenHeight-8, screenWidth-8, 4, 4);
    wbkgd(splash, COLOR_PAIR(1));

    mvwprintw(splash, screenHeight-8-2, 2, "\tThank you for using midigen. Press any key to exit...");
    wgetch(splash);
    delwin(splash);
    refresh();
}

static int getMode (WINDOW *header, WINDOW *menuWindow) {

    setHeading (header, "What do you want to do?");
    char *modeChoices[NUM_MODES] = MODE_NAMES;
    int mode = getIntWithMenu (menuWindow, modeChoices, NUM_MODES);

    return mode;
}

static void singleNoteMelody (FILE* output, songdata newsong) {

    unsigned int tempo = newsong->tempo;
    unsigned int numPlays = newsong->numPlays;
    unsigned int stepChance = newsong->stepChance;
    unsigned int restChance = newsong->restChance;
    unsigned int root = newsong->root;
    unsigned int range = newsong->range;
    unsigned int scaleID = newsong->scaleID;
    unsigned int isBoring = newsong->isBoring;

    free(newsong);
    newsong = NULL;

    int *base = generateDegrees (SCALE_DEFAULT_NOTES, numPlays, stepChance);
    int *rhythms = generateRhythms (tempo, numPlays, isBoring);

    scale allScales[NUM_SCALES] = SCALES;
    int *pitches = degreesToPitches (base, allScales[scaleID], SCALE_DEFAULT_NOTES,
                                     numPlays, root, range, 0);

    // mvprintw(15, 20, "tempo is %d, numPlays is %d and stepChance is %d", tempo, numPlays, stepChance);
    // mvprintw(17, 20, "restChance is %d, scaleID is %d", restChance, scaleID);
    // getch();

    int numEvents = numPlays * 2;
    int tracks = 0;
    track trackOne = createTrack (tempo, numEvents);
    tracks++;
    writeNotes (trackOne, pitches, rhythms, numEvents, restChance);

    midiHeader(tracks, tempo, output);
    printTrack(trackOne, output);

    free(base);
    base = NULL;
    free(rhythms);
    rhythms = NULL;
}

static void melodyWithHarmony (FILE* output, songdata newsong, int interval) {

    unsigned int tempo = newsong->tempo;
    unsigned int numPlays = newsong->numPlays;
    unsigned int stepChance = newsong->stepChance;
    unsigned int restChance = newsong->restChance;
    unsigned int root = newsong->root;
    unsigned int range = newsong->range;
    unsigned int scaleID = newsong->scaleID;
    unsigned int isBoring = newsong->isBoring;

    free(newsong);
    newsong = NULL;

    int *base = generateDegrees (SCALE_DEFAULT_NOTES, numPlays, stepChance);
    int *rhythms = generateRhythms (tempo, numPlays, isBoring);

    scale allScales[NUM_SCALES] = SCALES;
    int *pitches = degreesToPitches (base, allScales[scaleID], SCALE_DEFAULT_NOTES,
                                     numPlays, root, range, 0);

    int numEvents = numPlays * 2;
    int tracks = 0;
    track trackOne = createTrack (tempo, numEvents);
    tracks++;
    writeNotes (trackOne, pitches, rhythms, numEvents, restChance);

    pitches = degreesToPitches (base, allScales[scaleID], SCALE_DEFAULT_NOTES,
                                numPlays, root, range, interval);

    track trackTwo = createTrack (tempo, numEvents);
    tracks++;
    writeNotes (trackTwo, pitches, rhythms, numEvents, restChance);

    midiHeader(tracks, tempo, output);
    printTrack(trackOne, output);
    printTrack(trackTwo, output);

    free(base);
    base = NULL;
    free(rhythms);
    rhythms = NULL;
}

static songdata getParameters (WINDOW* header, WINDOW *menuWindow) {

    songdata newsong = malloc (sizeof(struct _songData));

    newsong->tempo = 60; //getIntWithInput (header, "Enter the tempo: ", 0, 500);
    newsong->numPlays = getIntWithInput (header, "Enter number of notes or rests: ", 1, MAX_EVENTS);
    newsong->stepChance = getIntWithInput (header, "Enter chance of stepwise motion (%): ", 0, 100);
    newsong->restChance = getIntWithInput (header, "Enter chance of rest (%): ", 0, 100);

    char *rhythmChoices[2] = { "Avant Garde", "Boring"};
    setHeading(header, "How do you like your rhythms?");
    newsong->isBoring = getIntWithMenu(menuWindow, rhythmChoices, 2);

    char *rootChoices[OCTAVE] = NOTE_NAMES;
    setHeading(header, "Choose a root note");
    newsong->root = getIntWithMenu(menuWindow, rootChoices, OCTAVE);

    unsigned int baseOctave = getIntWithInput (header, "How many octaves high should that be? ", 0, MAX_OCTAVE);
    newsong->root += baseOctave * OCTAVE;

    char *scaleChoices[NUM_SCALES] = SCALE_NAMES;
    setHeading(header, "Choose a scale");
    newsong->scaleID = getIntWithMenu(menuWindow, scaleChoices, NUM_SCALES);

    newsong->range = getIntWithInput (header, "Enter range in octaves: ", 1, MAX_RANGE);

    werase(header);
    werase(menuWindow);
    wrefresh(header);
    wrefresh(menuWindow);
    return newsong;
}

static void initialiseCurses (void) {
    initscr();              // start curses
    cbreak();               // allow uncooked key input and also ctrl-c
    noecho();               // stop the terminal from printing input
    curs_set(0);            // make the cursor invisible
    keypad(stdscr, TRUE);   // allow useful action keys like arrow keys
    start_color();          // let there be colours
    init_pair(1, COLOR_YELLOW, COLOR_BLUE);
    init_pair(2, COLOR_RED, COLOR_BLUE);
}

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

static void setHeading (WINDOW *header, char *message) {
    werase(header);
    mvwprintw(header, 1, 2, "%s", message);
    wrefresh(header);
}

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

// TODO: limits

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
        snprintf(errorMessage, 60, "This value must be between %d and %d. Please try again: ", min, max);
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

static int isValidChar (char c) {

    return (c >= '0' && c <= '9') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= 'a' && c <= 'z') ||
    c == '.' || c == '-' || c == '_';
}
