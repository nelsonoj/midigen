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
// music.h: the third level of abstraction for my SOMETHING AWESOME
//
// Use the Event functions to create musical things
//

#ifndef music_h
#define music_h

// Struct containing all the data a music generation needs
typedef struct _songData {
        unsigned int tempo;
        unsigned int numPlays;
        unsigned int stepChance;
        unsigned int restChance;
        unsigned int isBoring;
        unsigned int root;
        unsigned int range;
        unsigned int scaleID;
        // Only present in the relevant modes
        int harmony;
        int voices;
}*songdata;

/*
 *  Top level music generation
 */

// Generate and write a monophonic melody with the given parameters
void singleNoteMelody (FILE* output, songdata newsong);

// Generate and write a melody and harmony with the given parameters
void melodyWithHarmony (FILE* output, songdata newsong);

// Generate a random chord progression
void chords (FILE* output, songdata newsong);

// A demo to demonstrate the capabilities of Event
void demoOne (FILE *file);

// Unused but here for the heck of it
void chordDemo (FILE *file);

/*
 *  A level down, working with pitches and rhythms
 */

// Return an array of random ints that can be indices of any scale
int *generateDegrees (int width, int amount, int stepChance);

// Convert an array of degrees to midi pitches in a given scale
int *degreesToPitches (int *degrees, scale tones, int width, int amount,
                       int root, int range, int interval);

// Return an array of random musical divisions of the tempo
// If isBoring is true, rhythms will conform to a more listenable grid
int *generateRhythms (int tempo, int amount, int isBoring);

// Write an array of pitches and rhythms into midi data
void writeNotes (track t, int *pitches, int *rhythms, int numEvents,
    int restChance);

#endif /* music_h */
