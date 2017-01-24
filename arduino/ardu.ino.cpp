/*
	...

	Multimedia Belegarbeit 2017

	Bruno Schilling
	Maximilian Gertz
	Sven Erdem
*/

#define array_size(A) sizeof(A) / sizeof(A[0]);

// constants
const int KEYS_BEGIN = 6;
const int KEYS_END = 14;
const int NUM_KEYS = KEYS_END - KEYS_BEGIN - 1;

const int DISTANCE_MIN = 10;
const int DISTANCE_MAX = 100;
const int DISTANCE_PIN = 0;

const int NUM_FREQUENCIES_STEPS = 10;
const float FREQUENCIES[] {
	/* a */ 27.5000f,
	/* h */ 58.2705f,
	/* c */ 32.7032f,
	/* d */ 36.7081f,
	/* e */ 41.2035f,
	/* f */ 43.6536f,
	/* g */ 48.9995f,
};
const int NUM_FREQUENCIES = array_size(FREQUENCIES);

const unsigned char MIDI_NOTE_ON = 144;
const unsigned char MIDI_NOTE_OFF = 128;
const int MIDIS[] {
	22,
	23,
	24,
	26,
	28,
	29,
	31,
};

const int TONES[][2] {
	/* a, c */ {0, 2},
	/* g, d */ {6, 3},
	/* f, c */ {5, 2},
	/* e, c */ {4, 2},
	/* d, a */ {3, 0},
	/* c, g */ {2, 6},
	/* g, h */ {6, 1}
};
const int NUM_TONES = array_size(TONES);
const int NUM_FREQS_PER_TONE = array_size(TONES[0]);

// utility
template <typename T>
T clamp(T value, T min, T max)
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
	return digitalRead(key(index)) == HIGH;
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

float frequency(int index, int level)
{
	return FREQUENCIES[index] * level;
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

void loop(float r)
{
	const float r = range();

	for(int i = 0; i < NUM_KEYS; i++)
	{
		if(pressed(i))
		{
			int level = (int)clamp<float>(r * NUM_FREQUENCIES_STEPS * NUM_FREQS_PER_TONE, 0.f, NUM_FREQUENCIES_STEPS * NUM_FREQS_PER_TONE - 1);
			int freq = midi(tone(i, level % NUM_FREQS_PER_TONE), level / NUM_FREQS_PER_TONE);
			//printf("key: %d, tone: %d, level: %d, freq: %f\n", i, level % NUM_FREQS_PER_TONE, level / NUM_FREQS_PER_TONE, freq);
			playNote((unsigned char)freq);
		}
	}

	delayTime(200);
}
