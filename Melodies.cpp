#ifndef MELODIES_HPP
#define MELODIES_HPP

/*
 * This library will define pre-set melodies for frequency and duration.
*/

/*
* Jingle Bell
*/
const int JINGLE_BELL_NUM_NOTES = 27;

int jingleBellFrequencies[JINGLE_BELL_NUM_NOTES] = {
  330, 330, 330, 330, 330, 330, 330, 392, 
  262, 294, 330, 350, 350, 350, 350, 350, 
  350, 330, 330, 330, 330, 330, 294, 294, 
  330, 294, 392
};

int jingleBellDurations[JINGLE_BELL_NUM_NOTES] = {
  250, 250, 500, 250, 250, 500, 250, 250, 
  375, 125, 1000, 250, 250, 500, 250, 250, 
  500, 250, 250, 250, 250, 250, 250, 250, 
  250, 500, 500
};

int getJingleBell(int** frequencies, int** durations){
  *frequencies = jingleBellFrequencies;
  *durations = jingleBellDurations;   
  return JINGLE_BELL_NUM_NOTES;
}

/*
 * For Elisa
*/

const int FOR_ELISA_NUM_NOTES = 37;

int forElisaFrequencies[FOR_ELISA_NUM_NOTES] = {
660, 622, 660, 622, 494, 
587, 523, 440, 0, 262, 
330, 440, 493, 0, 330,
415, 494, 523,
0,
660, 622, 660, 622, 494, 
587, 523, 440, 0, 262, 
330, 440, 493, 0, 330,
415, 494, 440
};	

int forElisaDurations[FOR_ELISA_NUM_NOTES] = {
125, 125, 125, 125, 125, 
125, 125, 250, 125, 125,
125, 125, 250, 125, 125,
125, 125, 250,
300,
125, 125, 125, 125, 125, 
125, 125, 250, 125, 125,
125, 125, 250, 125, 125,
125, 125, 350
};

int getForElisa(int** frequencies, int** durations){
  *frequencies = forElisaFrequencies;
  *durations = forElisaDurations;
  return FOR_ELISA_NUM_NOTES;
}

#endif