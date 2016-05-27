//
//  Event.c
//
//
//  Created by Nelson Odins-Jones on 10/05/2016.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "Event.h"

// Values for midi header
#define MIDI_HEADER_START "MThd"
#define MIDI_HEADER_SIZE 6
#define MIDI_TYPE 1

// Sizes in bytes for parts of midi header
#define BYTES_MIDISTART 4
#define BYTES_HEADERSIZE 4
#define BYTES_TYPE 2
#define BYTES_TRACKCOUNT 2
#define BYTES_DURATION 2

// Sizes in bytes for parts of event
#define BYTES_TIMESTAMP_SMALL 1
#define BYTES_TIMESTAMP_BIG 2
#define BYTES_EVENT_SMALL 3 + BYTES_TIMESTAMP_SMALL
#define BYTES_EVENT_BIG 3 + BYTES_TIMESTAMP_BIG

// Values for track header and track out
#define TRACK_HEADER_START "MTrk"
#define TRACK_HEADER_SIZE 4
#define TRACK_END 0x00ff2f00

// Sizes in bytes for parts of track header
#define BYTES_TRACKSTART 4
#define BYTES_TRACKSIZE 4
#define BYTES_TRACKEND 4

typedef unsigned char byte;

struct _event{
    byte number;

    int duration;

    byte type;
    byte channel;

    byte pitch;

    byte volume;
};

// Local function prototypes

static void trackHeader (track t, FILE *file);
static void printEvent (track t, Event event, FILE *file);
static void trackOut (FILE *file);

static void destroyEvent (Event event);
static void destroyTrack (track t);

static void printTimestamp (track t, Event event, FILE *file);
static int durationToTimestamp (track t, Event event);
static byte *hexToByteArray (int hex, int numBytes);
static void printAsByteArray (int number, int size, FILE *file);

// Return a pointer to a new track struct with specified values

track createTrack (int tempo, int numEvents) {

    track newTrack = malloc(sizeof(struct _track));
    assert(newTrack != NULL);

    newTrack->events = malloc(sizeof(struct _event) * numEvents);
    assert(newTrack->events != NULL);

    newTrack->counter = 0;
    newTrack->tempo = tempo;
    newTrack->numEvents = numEvents;

    return newTrack;
}

// Return a pointer to a new event struct with specified values

Event createEvent (int *number, int duration, int type, int pitch, int volume) {

    Event newEvent = malloc(sizeof(struct _event));
    assert(newEvent != NULL);

    newEvent->number = ++*number;
    newEvent->duration = duration;
    newEvent->type = (byte)type;
    newEvent->channel = CHANNEL;
    newEvent->pitch = (byte)pitch;
    newEvent->volume = volume;

    return newEvent;
}

// Return a new event with a given pitch playing

Event startNote (track t, int pitch) {

    Event note = createEvent (&t->counter, 0, NOTE_ON, pitch,
                              VOLUME_PLAY);
    return note;
}

// Return a new event functioning as a pause

Event startRest (track t) {

    Event rest = createEvent(&t->counter, 0, NOTE_ON, MIDDLE_C,
                             VOLUME_SILENT);
    return rest;
}

// Given a playing note or rest, return a new event silencing it

Event endNote (track t, Event noteon, int noteLength) {

    assert(noteon != NULL);

    Event noteoff = createEvent (&t->counter, noteLength, NOTE_OFF,
                                 noteon->pitch, VOLUME_SILENT);
    return noteoff;
}

// Play and end a chord across an array of tracks

void setChord (track *tracks, int voices, int pos,
               int root, chord chord, int noteLength) {

    int i = 0;
    while (i < voices && i < CHORD_NOTES) {
        tracks[i]->events[pos] = startNote(tracks[i], root + chord[i]);
        tracks[i]->events[pos+1] = endNote(tracks[i], tracks[i]->events[pos],
                                           noteLength);
        i++;
    }
}


// Free a given event struct

static void destroyEvent (Event event) {

    assert(event != NULL);
    free(event);
    event = NULL;
}

// Free a given track struct

static void destroyTrack (track t) {

    assert(t != NULL);
    free(t->events);
    t->events = NULL;
    free(t);
    t = NULL;
}

// Write the midi header to standard out. This is done once per file.

void midiHeader (int trackCount, int tempo, FILE *file) {

    char *midiStart = MIDI_HEADER_START;
    fwrite (midiStart, BYTES_MIDISTART, 1, file);
    printAsByteArray (MIDI_HEADER_SIZE, BYTES_HEADERSIZE, file);
    printAsByteArray (MIDI_TYPE, BYTES_TYPE, file);
    printAsByteArray (trackCount, BYTES_TRACKCOUNT, file);
    printAsByteArray (tempo, BYTES_DURATION, file);

}

// Write an entire track to standard out, including all headers and events

void printTrack (track t,  FILE *file) {
    int i = 0;
    trackHeader(t, file);
    while (i < t->numEvents) {
        printEvent(t, t->events[i], file);
        i++;
    }
    trackOut(file);
    destroyTrack(t);
}

// Write a track header to standard out. This is done once per track.

static void trackHeader (track t, FILE *file) {
    char *trackStart = TRACK_HEADER_START;
    fwrite(trackStart, BYTES_TRACKSTART, 1, file);

    int trackSize = BYTES_EVENT_SMALL + (t->tempo > 0x7f);

    int i = 1;
    while (i < t->numEvents) {
        if (t->events[i]->duration > 0x7f) {
            trackSize += BYTES_EVENT_BIG;
        } else {
            trackSize += BYTES_EVENT_SMALL;
        }
        i++;
    }

    trackSize += BYTES_TRACKEND;

    printAsByteArray (trackSize, BYTES_TRACKSIZE, file);

}

// Write the constant end of track bytes to standard out

static void trackOut (FILE *file) {

    printAsByteArray (TRACK_END, BYTES_TRACKEND, file);
}

// Write a midi event to standard out

static void printEvent (track t, Event event, FILE *file) {

    assert(event != NULL);

    byte statusByte = event->type * 0x10 + event->channel;

    printTimestamp(t, event, file);
    fwrite(&statusByte,    sizeof statusByte,    1, file);
    fwrite(&event->pitch,  sizeof event->pitch,  1, file);
    fwrite(&event->volume, sizeof event->volume, 1, file);

    destroyEvent(event);
}

/*
 *  Rules governing next three functions:
 *
 *      First event is always either 00 or 80 00 (if tempo is over 7f)
 *      All noteons are 00 (regardless of duration)
 *      All noteoffs use complex rules: values up to 7f are printed as
 *      they are, but 80 becomes 81 00, 81 -> 81 01, and so forth.
 */

// Print the timestamp of a given event

static void printTimestamp (track t, Event event, FILE *file) {

    assert(event != NULL);

    int timestamp = durationToTimestamp(t, event);
    if (timestamp < 0x80) {
        fwrite(&timestamp, BYTES_TIMESTAMP_SMALL, 1, file);
    } else {
        printAsByteArray (timestamp, BYTES_TIMESTAMP_BIG, file);
    }
}

// Convert a duration to an integer, obeying the rules above

static int durationToTimestamp (track t, Event event) {

    assert(event != NULL);

    int time;
    if (event->number == 1) {
        time = 0x8000 * (t->tempo > 0x7f);
    } else if (event->volume == VOLUME_PLAY) {
        time = 0;
    } else if (event->duration > 0x7f) {
        time = 0x8000 + (event->duration/0x80 * 0x100) +
                        (event->duration - event->duration/0x80 * 0x80);
    } else {
        time = event->duration;
    }

    return time;
}

// Given a number, split it into bytes like this:
// e.g. 123456 -> 12 , 34 , 56

static byte *hexToByteArray (int hex, int numBytes) {

    byte *array = malloc(sizeof(byte) * numBytes);
    assert (array != NULL);

    int i = numBytes;
    while (i > 0) {
        array[i-1] = hex % 0x100;
        hex /= 0x100;
        i--;
    }

    return array;
}

// Print a number as constituent bytes

static void printAsByteArray (int number, int size, FILE *file) {

    byte *byteArray = hexToByteArray(number, size);
    int i = 0;
    while (i < size) {
        fwrite(&byteArray[i], sizeof byteArray[i], 1, file);
        i++;
    }
    free(byteArray);
    byteArray = NULL;
}

void runSomeTests (void) {

    int tempo = 0x7f;

    track trackOne = createTrack (tempo, 4);
    assert(trackOne->counter == 0);

    trackOne->events[0] = startNote (trackOne, MIDDLE_C);
    assert(trackOne->counter == 1);
    trackOne->events[1] = endNote (trackOne, trackOne->events[0], CROTCHET);
    assert(trackOne->counter == 2);

    tempo = 0x80;

    trackOne->events[3] = startNote (trackOne, MIDDLE_C + OCTAVE);
    trackOne->events[4] = endNote (trackOne, trackOne->events[3], CROTCHET);
    assert(trackOne->counter == 4);

    track trackTwo = createTrack (tempo, 2);
    assert(trackTwo->counter == 0);

    trackTwo->events[0] = startNote (trackTwo, MIDDLE_C);
    assert(trackTwo->counter == 1);
    trackTwo->events[1] = endNote (trackTwo, trackTwo->events[0], CROTCHET);
    assert(trackTwo->counter == 2);


    // Testing constructors
    {
        assert(trackOne->counter == 4);
        assert(trackOne->tempo == 0x7f);
        assert(trackOne->numEvents == 4);

        assert(trackTwo->counter == 2);
        assert(trackTwo->tempo == 0x80);
        assert(trackTwo->numEvents == 2);

        assert(trackOne->events[0] != NULL);
        assert(trackOne->events[0]->number == 1);
        assert(trackOne->events[0]->duration == 0);
        assert(trackOne->events[0]->type == NOTE_ON);
        assert(trackOne->events[0]->channel == CHANNEL);
        assert(trackOne->events[0]->pitch == MIDDLE_C);
        assert(trackOne->events[0]->volume == VOLUME_PLAY);

        assert(trackOne->events[1] != NULL);
        assert(trackOne->events[1]->number == 2);
        assert(trackOne->events[1]->duration == 0x7f);
        assert(trackOne->events[1]->type == NOTE_OFF);
        assert(trackOne->events[1]->channel == CHANNEL);
        assert(trackOne->events[1]->pitch == MIDDLE_C);
        assert(trackOne->events[1]->volume == VOLUME_SILENT);

        assert(trackOne->events[3] != NULL);
        assert(trackOne->events[3]->number == 3);
        assert(trackOne->events[3]->duration == 0);
        assert(trackOne->events[3]->type == NOTE_ON);
        assert(trackOne->events[3]->channel == CHANNEL);
        assert(trackOne->events[3]->pitch == 72);
        assert(trackOne->events[3]->volume == VOLUME_PLAY);

        assert(trackOne->events[4] != NULL);
        assert(trackOne->events[4]->number == 4);
        assert(trackOne->events[4]->duration == 0x80);
        assert(trackOne->events[4]->type == NOTE_OFF);
        assert(trackOne->events[4]->channel == CHANNEL);
        assert(trackOne->events[4]->pitch == 72);
        assert(trackOne->events[4]->volume == VOLUME_SILENT);

        assert(trackTwo->events[0] != NULL);
        assert(trackTwo->events[0]->number == 1);
        assert(trackTwo->events[0]->duration == 0);
        assert(trackTwo->events[0]->type == NOTE_ON);
        assert(trackTwo->events[0]->channel == CHANNEL);
        assert(trackTwo->events[0]->pitch == MIDDLE_C);
        assert(trackTwo->events[0]->volume == VOLUME_PLAY);

        assert(trackTwo->events[1] != NULL);
        assert(trackTwo->events[1]->number == 2);
        assert(trackTwo->events[1]->duration == 0x80);
        assert(trackTwo->events[1]->type == NOTE_OFF);
        assert(trackTwo->events[1]->channel == CHANNEL);
        assert(trackTwo->events[1]->pitch == MIDDLE_C);
        assert(trackTwo->events[1]->volume == VOLUME_SILENT);

    }

    // Testing durationToTimestamp
    {
        // First event in a track, tempo under 128
        assert(durationToTimestamp(trackOne, trackOne->events[0]) == 0);
        // Noteoff event in a track, tempo under 128
        assert(durationToTimestamp(trackOne, trackOne->events[1]) == 0x7f);
        // Noteon event in a track, tempo 128
        assert(durationToTimestamp(trackOne, trackOne->events[3]) == 0);
        // Noteoff event in a track, tempo 128
        assert(durationToTimestamp(trackOne, trackOne->events[4]) == 0x8100);
        // First event in a track, tempo 128
        assert(durationToTimestamp(trackTwo, trackTwo->events[0]) == 0x8000);
        // Noteoff event in a track, tempo 128
        assert(durationToTimestamp(trackTwo, trackTwo->events[1]) == 0x8100);
    }

    /*
     byte *timestamp = hexToByteArray(durationToTimestamp(trackTwo, trackTwo->events[0]), 2);
     int i = 0;
     while (i < 2) {
     printf("%02x ", timestamp[i]);
     i++;
     }


    printEvent(trackTwo, trackTwo->events[0]);

    printf("\n");
    printf("All tests passed! You are awesome!\n");
*/
}
