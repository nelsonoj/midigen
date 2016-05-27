//
//  Event.h
//
//
//  Created by Nelson Odins-Jones on 10/05/2016.
//
//

#ifndef Event_h
#define Event_h

#define TRUE  1
#define FALSE 0

// Rhythms
#define SEMIBRIEF  (tempo*4)
#define MINIM      (tempo*2)
#define CROTCHET   (tempo)
#define QUAVER     (tempo/2)
#define TRIPLET    (tempo/3)
#define SEMIQUAVER (tempo/4)
#define RHYTHMS { SEMIBRIEF, MINIM, CROTCHET, TRIPLET, QUAVER, SEMIQUAVER }
#define NUM_RHYTHMS 6

// Event types
#define NOTE_OFF 9
#define NOTE_ON  9
#define CC      11

// Control changes
#define CC_SILENCE 123

// Channel to use (global setting)
#define CHANNEL 0

// Pitches
#define OCTAVE 12
#define MIDDLE_C    60
#define C_SHARP     61
#define D           62
#define D_SHARP     63
#define E           64
#define F           65
#define F_SHARP     66
#define G           67
#define G_SHARP     68
#define A           69
#define A_SHARP     70
#define B           71
#define NOTE_NAMES { "C", "C#", "D", "D#", "E", "F", \
                     "F#", "G", "G#", "A", "A#", "B" };
#define HIGHEST_NOTE 127

// Volumes
#define VOLUME_PLAY  96
#define VOLUME_SILENT 0

// Chords
#define CHORD_NOTES 4
#define CHORD_MAJOR    { 0, 4, 7, -OCTAVE }
#define CHORD_MINOR    { 0, 3, 7, -OCTAVE }
#define CHORD_DOMSEVEN { 0, 4, 7, 10 }
#define CHORD_MAJSEVEN { 0, 4, 7, 11 }
#define CHORD_MINSEVEN { 0, 3, 7, 10 }
#define CHORDS { CHORD_MAJOR, CHORD_MINOR, CHORD_DOMSEVEN, \
                 CHORD_MAJSEVEN, CHORD_MINSEVEN }
#define NUM_CHORDS 5

// Scales
#define SCALE_DEFAULT_NOTES 7
#define SCALE_MAJOR     { 0, 2, 4, 5, 7, 9, 11 }
#define SCALE_MINOR     { 0, 2, 3, 5, 7, 8, 10 }
#define SCALE_HARMMINOR { 0, 2, 3, 5, 7, 8, 11 }
#define CHROMATIC -1
#define MAJOR      0
#define MINOR      1
#define HARMMINOR  2
#define SCALES { SCALE_MAJOR, SCALE_MINOR, SCALE_HARMMINOR }
#define NUM_SCALES 3
#define SCALE_NAMES { "Major", "Minor", "Harmonic minor" };

typedef struct _event *Event;

typedef int chord[CHORD_NOTES];
typedef int scale[OCTAVE];

struct _track{
    Event *events;
    int counter;
    int tempo;
    int numEvents;
};

typedef struct _track *track;

/*
 *  Construction and Destruction
 */

// Return a pointer to a new track struct with specified values
track createTrack (int tempo, int numEvents);

// Return a pointer to a new event struct with specified values
Event createEvent (int *number, int duration, int type, int pitch, int volume);

// Return a new event with a given pitch playing
Event startNote (track t, int pitch);

// Return a new event functioning as a pause
Event startRest (track t);

// Given a playing note, return a new event silencing it
Event endNote (track t, Event noteon, int noteLength);

// Play a chord across an array of tracks
void setChord (track *tracks, int voices, int pos,
               int root, chord chord, int noteLength);

/*
 *  Writing the midi file
 */

// Write the midi header to standard out. This is done once per file.
void midiHeader (int trackCount, int tempo, FILE* file);

// Write an entire track to standard out, including all headers and events
void printTrack (track t, FILE* file);



void runSomeTests (void);

#endif /* Event_h */
