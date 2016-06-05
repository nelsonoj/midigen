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
// interface.h: the second level of abstraction for my SOMETHING AWESOME
//
// Abstraction of curses library functions into custom functions
//

#ifndef interface_h
#define interface_h

#define MAX_EVENTS 999
#define MAX_CHORDS 16
#define MAX_VOICES 4
#define MAX_OCTAVE 9
#define MAX_RANGE (HIGHEST_NOTE - OCTAVE*baseOctave) / OCTAVE
#define DEFAULT_TEMPO 60

#define MAX_FILENAME 14
#define FILE_EXTENSION ".mid"
#define FILE_EXTENSION_SIZE 4

#define HEADER_HEIGHT 3
#define SPLASH_PADDING 8
#define QUIT 'q'

// Modes
#define NUM_MODES 4
#define MODE_MONO 0
#define MODE_HARM 1
#define MODE_CHOR 2
#define MODE_DEMO 3
#define MODE_NAMES {"Generate a monophonic melody", \
                    "Generate a melody with a harmony", \
                    "Generate a chord progression", \
                    "Run the demo" }

#define HARMONY_NAMES { "Seconds", "Thirds (classic)", "Fourths", \
                        "Fifths", "Sixths", "Sevenths", "Octaves" }

// Display the splash screen
void welcome (int screenHeight, int screenWidth);

// Get a filename from the user and open a midi file with that name
FILE *initialiseFile (WINDOW *header);

// Boot up the graphical system
void initialiseCurses (void);

// Display a nice farewell message
void farewell (WINDOW *header, int screenHeight, int screenWidth);

// Find out what the user wants to do
int getMode (WINDOW *header, WINDOW *menuWindow);

// Get all the user parameters and return them in one big struct
songdata getParameters (WINDOW* header, WINDOW *menuWindow, int mode);

#endif /* interface_h */
