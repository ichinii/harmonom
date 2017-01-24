/*
  ...

  Multimedia Belegarbeit 2017

  Bruno Schilling
  Maximilian Gertz
  Sven Erdem

  version 2.1
*/

#define array_size(A) sizeof(A) / sizeof(A[0]);

// constants
const int KEYS_BEGIN = 7;
const int KEYS_END = 14;
const int NUM_KEYS = KEYS_END - KEYS_BEGIN;

const int DISTANCE_MIN = 10;
const int DISTANCE_MAX = 100;
const int DISTANCE_PIN = 0;

const int OCTAVE_BEGIN = 2;
const int OCTAVE_END = 7;
const int NUM_OCTAVES = 4;

const unsigned char MIDI_NOTE_ON = 144;
const unsigned char MIDI_NOTE_OFF = 128;
const int MIDIS[] {
  21,
  23,
  24,
  26,
  28,
  29,
  31,
};

const int TONES[][2] {
  /* a, c */ {0, 2},
  /* g, d */ {3, 6},
  /* f, c */ {2, 5},
  /* e, c */ {2, 4},
  /* d, a */ {0, 3},
  /* c, g */ {2, 6},
  /* g, h */ {1, 6}
};
const int NUM_TONES = array_size(TONES);
const int NUM_TONES_PER_KEY = array_size(TONES[0]);

// utility
float clamp(float value, float min, float max)
{
  return value < min ? min : (value > max ? max : value);
}

float normalize(float value, float min, float max)
{
  return clamp((value - min) / (max - min), 0.f, 1.f);
}

// keys
int key(int index)
{
  return clamp(index + KEYS_BEGIN, KEYS_BEGIN, KEYS_END - 1);
}

bool pressed(int index)
{
  return digitalRead(key(index)) == LOW;
}

bool clicked(int index)
{
  static bool prev[NUM_KEYS] {0};
  if(pressed(index))
  {
    if(!prev[index])
    {
      prev[index] = true;
      return true;  
    }
    else
    {
      prev[index] = true;
      return false;
    }
  }

  prev[index] = false;
  return false;
}

// frequency
float distance()
{
  return analogRead(DISTANCE_PIN);
}

float range()
{
  return normalize(distance(), DISTANCE_MIN, DISTANCE_MAX);
}

int tone(int index, int alter)
{
  return TONES[index][alter];
}

int midi(int tone, int level)
{
  return MIDIS[tone] + level * 12;
}
void midiOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
	Serial.write(MIDI_NOTE_OFF + channel;
	Serial.write(note);
	Serial.write(velocity);
}
void midiOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
	Serial.write(MIDI_NOTE_OFF + channel;
	Serial.write(note);
	Serial.write(velocity);
}
void playNote(unsigned char note)
{
	midiOn(1, note, 100);
}

void setup()
{
  for(int i = 0; i < NUM_KEYS; i++)
    pinMode(key(i), INPUT);

  Serial.begin(31250);
}

void loop()
{
  const float r = range();

  for(int i = 0; i < NUM_KEYS; i++)
  {
    static int prev[NUM_KEYS] {0};
    
    int level = (int)clamp(r * NUM_OCTAVES * NUM_TONES_PER_KEY, 0.f, NUM_OCTAVES * NUM_TONES_PER_KEY - 1);
    //if(level != prev[i] && pressed(i) || clicked(i))
    if(pressed(i))
    {
      int freq = midi(tone(i, level % NUM_TONES_PER_KEY), level / NUM_TONES_PER_KEY + OCTAVE_BEGIN);
      //printf("key: %d, tone: %d, level: %d, freq: %f\n", i, level % NUM_FREQS_PER_TONE, level / NUM_FREQS_PER_TONE, freq);
      playNote((unsigned char)freq);
    }
    prev[i] = level;
  }

  delay(250);
}
