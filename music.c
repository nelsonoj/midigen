//
//  music.c
//
//
//  Created by Nelson Odins-Jones on 14/05/2016.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Event.h"
#include "music.h"

//  int main (int argc, char *argv[]) {
//
//     srand(time(NULL));
//
//     runSomeTests();
//     FILE *output = initialiseFile();
//
//     int tracks = 0;
//     int tempo = 60;
//
//     int numPlays = 0;
//     printf("Enter number of notes or rests\n");
//     scanf("%d", &numPlays);
//
//     int numEvents = numPlays * 2;
//
//     int range = 2;
//     int root = MIDDLE_C;
//     int scaleID = 0;
//
//     printf("Major: 0, minor: 1, harmonic minor: 2\n");
//     scanf("%d", &scaleID);
//
//     scale allScales[NUM_SCALES] = SCALES;
//     //    chord chords[NUM_CHORDS] = CHORDS;
//
//     // generate the random notes
//
//
//     unsigned int stepChance = 0;
//
//     printf("Chance of stepwise motion (%%)?\n");
//     scanf("%d", &stepChance);
//
//     if (stepChance > 100) {
//         stepChance = 100;
//     }
//
//     int *base = generateDegrees (SCALE_DEFAULT_NOTES, numPlays, stepChance);
//     int *rhythms = generateRhythms (tempo, numPlays);
//
//     unsigned int restChance = 0;
//
//     printf("Chance of rest (%%)?\n");
//     scanf("%d", &restChance);
//
//     if (restChance > 100) {
//         restChance = 100;
//     }
//
//     int *pitches = degreesToPitches (base, allScales[scaleID], SCALE_DEFAULT_NOTES,
//                                      numPlays, root, range, 0);
//     track trackOne = createTrack (tempo, numEvents);
//     tracks++;
//     writeNotes (trackOne, pitches, rhythms, numEvents, restChance);
//
//     pitches = degreesToPitches (base, allScales[scaleID], SCALE_DEFAULT_NOTES,
//                                      numPlays, root, range, 2);
//     track trackTwo = createTrack (tempo, numEvents);
//     tracks++;
//     writeNotes (trackTwo, pitches, rhythms, numEvents, restChance);
//
//     free(base);
//     free(rhythms);
//
//     /*
//     int voices = 4;
//     track *poly = malloc (sizeof(struct _track) * voices);
//     assert (poly != NULL);
//     tracks += voices;
//     i = 0;
//     while (i < voices) {
//         poly[i] = createTrack(tempo, numEvents);
//         i++;
//     }
//
//     i = 0;
//     while (i < numPlays) {
//         setChord(poly, voices, i*2, pitches[i], chords[rand()%NUM_CHORDS],
//                  rhythms[rand() % NUM_RHYTHMS]);
//         i++;
//     }
//     */
//     midiHeader(tracks, tempo, output);
//     printTrack(trackOne, output);
//     printTrack(trackTwo, output);
//     /*
//     i = 0;
//     while (i < voices) {
//         printTrack (poly[i], output);
//         i++;
//     }
//     free(poly);
//     */
//     fclose(output);
//     return EXIT_SUCCESS;
// }

// Write pitch and rhythm array to track

void writeNotes (track t, int *pitches, int *rhythms, int numEvents, int restChance) {
    int i = 0;
    while (i < numEvents) {
        if (restChance > 0 && (float)rand()/RAND_MAX > (float)restChance / 100) {
            t->events[i] = startNote(t, pitches[i/2]);
            t->events[i+1] = endNote(t, t->events[i],
                                            rhythms[i/2]);
        } else {
            t->events[i] = startRest(t);
            t->events[i+1] = endNote(t, t->events[i],
                                            rhythms[i/2]);
        }
        i += 2;
    }
    free(pitches);
    pitches = NULL;
}

// passing a scale instead of an ID is necessary
// it allows for custom scales later on

int *degreesToPitches (int *degrees, scale tones, int width, int amount,
                       int root, int range, int interval) {

    int *pitches = malloc (sizeof(int) * amount);
    assert(pitches != NULL);
    int i = 0;
    while (i < amount) {
        pitches[i] = tones[(degrees[i] + interval) % width];
        pitches[i] += (rand() % range) * OCTAVE;
        pitches[i] += root;
        i++;
    }
    return pitches;
}

// Return an array of random rhythms

int *generateRhythms (int tempo, int amount, int isBoring) {

    int *rhythms = malloc (sizeof(int) * amount);
    assert(rhythms != NULL);

    int possibleRhythms[NUM_RHYTHMS] = RHYTHMS;
    if (!isBoring) {
        int i = 0;
        while (i < amount) {
            rhythms[i] = possibleRhythms[rand() % NUM_RHYTHMS];
            i++;
        }
    } else {
        int i = 0;
        while (i < amount) {
            rhythms[i] = possibleRhythms[rand() % NUM_RHYTHMS];
            int j = 1;
            if (rhythms[i] < tempo) {
                while (j < tempo/rhythms[i]) {
                    rhythms[i+j] =  rhythms[i];
                    j++;
                }
            }
            i+=j;
        }
    }

    return rhythms;
}

// Return an array of random ints that can be indices of any scale

int *generateDegrees (int width, int amount, int stepChance) {

    int *degrees = malloc (sizeof(int) * amount);
    assert(degrees != NULL);

    degrees[0] = rand() % width;

    int i = 1;
    while (i < amount) {
        if (stepChance > 0 && (float)rand()/RAND_MAX > (float)stepChance / 100) {
            degrees[i] = rand() % width;
        } else {
            degrees[i] = degrees[i-1] + -1 * ((rand()%3) - 1);
            if (degrees[i] < 0) {
                degrees[i] = 0;
            }
        }
        i++;
    }
    return degrees;
}

void demoOne (FILE *file) {

    int tempo = 120;
    int tracks = 0;

    assert(tempo % 12 == 0);

    track melody = createTrack(tempo, 34);
    tracks++;
    {
        melody->events[0] = startRest(melody);
        melody->events[1] = endNote(melody, melody->events[0], SEMIBRIEF);

        melody->events[2] = startRest(melody);
        melody->events[3] = endNote(melody, melody->events[2], SEMIBRIEF);

        melody->events[4] = startRest(melody);
        melody->events[5] = endNote(melody, melody->events[4], SEMIBRIEF);

        melody->events[6] = startRest(melody);
        melody->events[7] = endNote(melody, melody->events[6], SEMIBRIEF);

        melody->events[8] = startRest(melody);
        melody->events[9] = endNote(melody, melody->events[8], 3*CROTCHET);

        melody->events[10] = startNote(melody, A);
        melody->events[11] = endNote(melody, melody->events[10], QUAVER+SEMIQUAVER);
        melody->events[12] = startNote(melody, A);
        melody->events[13] = endNote(melody, melody->events[12], SEMIQUAVER);

        melody->events[14] = startNote(melody, A);
        melody->events[15] = endNote(melody, melody->events[14], 3*CROTCHET);

        melody->events[16] = startNote(melody, A);
        melody->events[17] = endNote(melody, melody->events[16], QUAVER+SEMIQUAVER);
        melody->events[18] = startNote(melody, A);
        melody->events[19] = endNote(melody, melody->events[18], SEMIQUAVER);

        melody->events[20] = startNote(melody, A);
        melody->events[21] = endNote(melody, melody->events[20], MINIM);
        melody->events[22] = startNote(melody, A_SHARP);
        melody->events[23] = endNote(melody, melody->events[22], MINIM);

        melody->events[24] = startNote(melody, A);
        melody->events[25] = endNote(melody, melody->events[24], MINIM);
        melody->events[26] = startNote(melody, G);
        melody->events[27] = endNote(melody, melody->events[26], CROTCHET);
        melody->events[28] = startNote(melody, MIDDLE_C + OCTAVE);
        melody->events[29] = endNote(melody, melody->events[28], CROTCHET);

        melody->events[30] = startRest(melody);
        melody->events[31] = endNote(melody, melody->events[30], SEMIQUAVER/2*3);
        melody->events[32] = startNote(melody, F);
        melody->events[33] = endNote(melody, melody->events[32], MINIM);
    }

    track trackOne = createTrack(tempo, 196);
    tracks++;
    {
        int i = 0;
        while (i < 48) {

            trackOne->events[i] = startNote(trackOne, A - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, D);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, F);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        while (i < 60) {

            trackOne->events[i] = startNote(trackOne, A_SHARP - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, D);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, F);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        while (i < 72) {

            trackOne->events[i] = startNote(trackOne, A_SHARP - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, D_SHARP);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, G);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        {
            trackOne->events[72] = startNote(trackOne, A - OCTAVE);
            trackOne->events[73] = endNote(trackOne, trackOne->events[72], TRIPLET);
            trackOne->events[74] = startNote(trackOne, C_SHARP);
            trackOne->events[75] = endNote(trackOne, trackOne->events[74], TRIPLET);
            trackOne->events[76] = startNote(trackOne, G);
            trackOne->events[77] = endNote(trackOne, trackOne->events[76], TRIPLET);

            trackOne->events[78] = startNote(trackOne, A - OCTAVE);
            trackOne->events[79] = endNote(trackOne, trackOne->events[78], TRIPLET);
            trackOne->events[80] = startNote(trackOne, D);
            trackOne->events[81] = endNote(trackOne, trackOne->events[80], TRIPLET);
            trackOne->events[82] = startNote(trackOne, F);
            trackOne->events[83] = endNote(trackOne, trackOne->events[82], TRIPLET);

            trackOne->events[84] = startNote(trackOne, A - OCTAVE);
            trackOne->events[85] = endNote(trackOne, trackOne->events[84], TRIPLET);
            trackOne->events[86] = startNote(trackOne, D);
            trackOne->events[87] = endNote(trackOne, trackOne->events[86], TRIPLET);
            trackOne->events[88] = startNote(trackOne, E);
            trackOne->events[89] = endNote(trackOne, trackOne->events[88], TRIPLET);

            trackOne->events[90] = startNote(trackOne, G - OCTAVE);
            trackOne->events[91] = endNote(trackOne, trackOne->events[90], TRIPLET);
            trackOne->events[92] = startNote(trackOne, C_SHARP);
            trackOne->events[93] = endNote(trackOne, trackOne->events[92], TRIPLET);
            trackOne->events[94] = startNote(trackOne, E);
            trackOne->events[95] = endNote(trackOne, trackOne->events[94], TRIPLET);
        }


        trackOne->events[96] = startNote(trackOne, F - OCTAVE);
        trackOne->events[97] = endNote(trackOne, trackOne->events[96], TRIPLET);
        trackOne->events[98] = startNote(trackOne, A - OCTAVE);
        trackOne->events[99] = endNote(trackOne, trackOne->events[98], TRIPLET);
        trackOne->events[100] = startNote(trackOne, D);
        trackOne->events[101] = endNote(trackOne, trackOne->events[100], TRIPLET);

        trackOne->events[102] = startNote(trackOne, A - OCTAVE);
        trackOne->events[103] = endNote(trackOne, trackOne->events[102], TRIPLET);
        trackOne->events[104] = startNote(trackOne, D);
        trackOne->events[105] = endNote(trackOne, trackOne->events[104], TRIPLET);
        trackOne->events[106] = startNote(trackOne, F);
        trackOne->events[107] = endNote(trackOne, trackOne->events[106], TRIPLET);

        trackOne->events[108] = startNote(trackOne, A - OCTAVE);
        trackOne->events[109] = endNote(trackOne, trackOne->events[108], TRIPLET);
        trackOne->events[110] = startNote(trackOne, D);
        trackOne->events[111] = endNote(trackOne, trackOne->events[110], TRIPLET);
        trackOne->events[112] = startNote(trackOne, F);
        trackOne->events[113] = endNote(trackOne, trackOne->events[112], TRIPLET);

        trackOne->events[114] = startNote(trackOne, A - OCTAVE);
        trackOne->events[115] = endNote(trackOne, trackOne->events[114], TRIPLET);
        trackOne->events[116] = startNote(trackOne, D);
        trackOne->events[117] = endNote(trackOne, trackOne->events[116], TRIPLET);
        trackOne->events[118] = startNote(trackOne, F);
        trackOne->events[119] = endNote(trackOne, trackOne->events[118], TRIPLET);

        i = 120;
        while (i < 144) {

            trackOne->events[i] = startNote(trackOne, A - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, E);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, G);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        while (i < 156) {

            trackOne->events[i] = startNote(trackOne, A - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, D);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, F);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        while (i < 168) {

            trackOne->events[i] = startNote(trackOne, A_SHARP - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, D);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, G);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }

        while (i < 180) {

            trackOne->events[i] = startNote(trackOne, A - OCTAVE);
            trackOne->events[i+1] = endNote(trackOne, trackOne->events[i], TRIPLET);
            trackOne->events[i+2] = startNote(trackOne, MIDDLE_C);
            trackOne->events[i+3] = endNote(trackOne, trackOne->events[i+2], TRIPLET);
            trackOne->events[i+4] = startNote(trackOne, F);
            trackOne->events[i+5] = endNote(trackOne, trackOne->events[i+4], TRIPLET);
            i += 6;
        }
        trackOne->events[180] = startNote(trackOne, A_SHARP - OCTAVE);
        trackOne->events[181] = endNote(trackOne, trackOne->events[180], TRIPLET);
        trackOne->events[182] = startNote(trackOne, MIDDLE_C);
        trackOne->events[183] = endNote(trackOne, trackOne->events[182], TRIPLET);
        trackOne->events[184] = startNote(trackOne, E);
        trackOne->events[185] = endNote(trackOne, trackOne->events[184], TRIPLET);

        trackOne->events[186] = startNote(trackOne, A_SHARP - 3*OCTAVE);
        trackOne->events[187] = endNote(trackOne, trackOne->events[186], TRIPLET);
        trackOne->events[188] = startNote(trackOne, MIDDLE_C);
        trackOne->events[189] = endNote(trackOne, trackOne->events[188], TRIPLET);
        trackOne->events[190] = startNote(trackOne, E);
        trackOne->events[191] = endNote(trackOne, trackOne->events[190], TRIPLET);

        trackOne->events[192] = startRest(trackOne);
        trackOne->events[193] = endNote(trackOne, trackOne->events[192], SEMIQUAVER);
        trackOne->events[194] = startNote(trackOne, MIDDLE_C);
        trackOne->events[195] = endNote(trackOne, trackOne->events[194], MINIM);
    }

    track bass = createTrack(tempo, 28);
    tracks++;
    {
        bass->events[0] = startNote(bass, D - 2*OCTAVE);
        bass->events[1] = endNote(bass, bass->events[0], SEMIBRIEF);

        bass->events[2] = startNote(bass, MIDDLE_C - 2*OCTAVE);
        bass->events[3] = endNote(bass, bass->events[2], SEMIBRIEF);

        bass->events[4] = startNote(bass, A_SHARP - 2*OCTAVE);
        bass->events[5] = endNote(bass, bass->events[4], MINIM);
        bass->events[6] = startNote(bass, G - 2*OCTAVE);
        bass->events[7] = endNote(bass, bass->events[6], MINIM);

        bass->events[8] = startNote(bass, A - 2*OCTAVE);
        bass->events[9] = endNote(bass, bass->events[8], MINIM);
        bass->events[10] = startNote(bass, A - 2*OCTAVE);
        bass->events[11] = endNote(bass, bass->events[10], MINIM);

        bass->events[12] = startNote(bass, D - 2*OCTAVE);
        bass->events[13] = endNote(bass, bass->events[12], SEMIBRIEF);

        bass->events[14] = startNote(bass, C_SHARP - 2*OCTAVE);
        bass->events[15] = endNote(bass, bass->events[14], SEMIBRIEF);

        bass->events[16] = startNote(bass, D - 2*OCTAVE);
        bass->events[17] = endNote(bass, bass->events[16], MINIM);
        bass->events[18] = startNote(bass, G - 2*OCTAVE);
        bass->events[19] = endNote(bass, bass->events[18], MINIM);

        bass->events[20] = startNote(bass, MIDDLE_C - 2*OCTAVE);
        bass->events[21] = endNote(bass, bass->events[20], MINIM);
        bass->events[22] = startNote(bass, MIDDLE_C - 2*OCTAVE);
        bass->events[23] = endNote(bass, bass->events[22], MINIM);

        bass->events[24] = startRest(bass);
        bass->events[25] = endNote(bass, bass->events[24], SEMIQUAVER/2);
        bass->events[26] = startNote(bass, A - OCTAVE);
        bass->events[27] = endNote(bass, bass->events[26], MINIM);
    }

    track bassTwo = createTrack(tempo, 18);
    tracks++;
    {
        bassTwo->events[0] = startRest(bassTwo);
        bassTwo->events[1] = endNote(bassTwo, bassTwo->events[0], SEMIBRIEF);

        bassTwo->events[2] = startRest(bassTwo);
        bassTwo->events[3] = endNote(bassTwo, bassTwo->events[2], SEMIBRIEF);

        bassTwo->events[4] = startRest(bassTwo);
        bassTwo->events[5] = endNote(bassTwo, bassTwo->events[4], SEMIBRIEF);

        bassTwo->events[6] = startRest(bassTwo);
        bassTwo->events[7] = endNote(bassTwo, bassTwo->events[6], SEMIBRIEF);

        bassTwo->events[8] = startNote(bassTwo, A - 2*OCTAVE);
        bassTwo->events[9] = endNote(bassTwo, bassTwo->events[8], SEMIBRIEF);

        bassTwo->events[10] = startNote(bassTwo, A - 2*OCTAVE);
        bassTwo->events[11] = endNote(bassTwo, bassTwo->events[10], SEMIBRIEF);

        bassTwo->events[12] = startRest(bassTwo);
        bassTwo->events[13] = endNote(bassTwo, bassTwo->events[12], SEMIBRIEF);

        bassTwo->events[14] = startRest(bassTwo);
        bassTwo->events[15] = endNote(bassTwo, bassTwo->events[14], SEMIBRIEF);

        bassTwo->events[16] = startNote(bassTwo, F - 2*OCTAVE);
        bassTwo->events[17] = endNote(bassTwo, bassTwo->events[16], SEMIBRIEF);

    }

    midiHeader(tracks, tempo, file);
    printTrack(melody, file);
    printTrack(trackOne, file);
    printTrack(bass, file);
    printTrack(bassTwo, file);
}

void chordDemo (FILE *file) {
    int tempo = 256;
    int numEvents = 16;
    int numTracks = 4;

    //    chord major = CHORD_MAJOR;
    //    chord minor = CHORD_MINOR;
    chord dom7 = CHORD_DOMSEVEN;
    chord major7 = CHORD_MAJSEVEN;
    chord minor7 = CHORD_MINSEVEN;

    midiHeader (numTracks, tempo, file);

    track *chord = malloc(sizeof(struct _track) * numTracks);
    int i = 0;
    while (i < numTracks) {
        chord[i] = createTrack(tempo, numEvents);
        i++;
    }

    //    setChord (chord, numTracks, 0, MIDDLE_C, major, SEMIBRIEF);
    //    setChord (chord, numTracks, 2, E - OCTAVE, minor, SEMIBRIEF);
    //    setChord (chord, numTracks, 4, A - OCTAVE, minor, SEMIBRIEF);
    //    setChord (chord, numTracks, 6, G_SHARP - OCTAVE, major, SEMIBRIEF);

    setChord (chord, numTracks, 0, A, minor7, MINIM);
    setChord (chord, numTracks, 2, D, dom7, MINIM);
    setChord (chord, numTracks, 4, G, major7, MINIM);
    setChord (chord, numTracks, 6, MIDDLE_C, major7, MINIM);
    setChord (chord, numTracks, 8, F_SHARP, minor7, MINIM);
    setChord (chord, numTracks, 10, B, dom7, MINIM);
    setChord (chord, numTracks, 12, E, minor7, MINIM);
    setChord (chord, numTracks, 14, E - OCTAVE, minor7, MINIM);

    i = 0;
    while (i < numTracks) {
        printTrack (chord[i], file);
        i++;
    }
    free(chord);
    chord = NULL;
}
