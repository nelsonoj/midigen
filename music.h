//
//  music.h
//
//
//  Created by Nelson Odins-Jones on 14/05/2016.
//
//

#ifndef music_h
#define music_h

// Return an array of random ints that can be indices of any scale
int *generateDegrees (int width, int amount, int stepChance);

int *degreesToPitches (int *degrees, scale tones, int width, int amount,
                       int root, int range, int interval);

int *generateRhythms (int tempo, int amount, int isBoring);

void writeNotes (track t, int *pitches, int *rhythms, int numEvents, int restChance);

void demoOne (FILE *file);
void chordDemo (FILE *file);

#endif /* music_h */
