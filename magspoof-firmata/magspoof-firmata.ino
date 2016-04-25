#include <Firmata.h>
#include <vector>

#define PIN_A 0
#define PIN_B 1
#define ENABLE_PIN 3 // also green LED
#define SWAP_PIN 4 // unused
#define BUTTON_PIN 2
#define CLOCK_US 200

#define BETWEEN_ZERO 53 // 53 zeros between track1 & 2

#define TRACKS 2

// consts get stored in flash as we don't adjust them
char* tracks[] = {
"", // Track 1
"" // Track 2
};

char revTrack[41];

const int sublen[] = { 
  32, 48, 48 };
const int bitlen[] = { 
  7, 5, 5 };

unsigned int curTrack = 0;
int dir;

void blink(int pin, int msdelay, int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(pin, HIGH);
    delay(msdelay);
    digitalWrite(pin, LOW);
    delay(msdelay);
  }
}

// send a single bit out
void playBit(int sendBit)
{
  dir ^= 1;
  digitalWrite(PIN_A, dir);
  digitalWrite(PIN_B, !dir);
  delayMicroseconds(CLOCK_US);

  if (sendBit)
  {
    dir ^= 1;
    digitalWrite(PIN_A, dir);
    digitalWrite(PIN_B, !dir);
  }
  delayMicroseconds(CLOCK_US);

}

// when reversing
void reverseTrack(int track)
{
  int i = 0;
  track--; // index 0
  dir = 0;

  while (revTrack[i++] != '\0');
  i--;
  while (i--)
    for (int j = bitlen[track]-1; j >= 0; j--)
      playBit((revTrack[i] >> j) & 1);
}

// plays out a full track, calculating CRCs and LRC
void playTrack(int track)
{
  int tmp, crc, lrc = 0;
  track--; // index 0
  dir = 0;

  // enable H-bridge and LED
  digitalWrite(ENABLE_PIN, HIGH);

  // First put out a bunch of leading zeros.
  for (int i = 0; i < 25; i++)
    playBit(0);

  //
  for (int i = 0; tracks[track][i] != '\0'; i++)
  {
    crc = 1;
    tmp = tracks[track][i] - sublen[track];

    for (int j = 0; j < bitlen[track]-1; j++)
    {
      crc ^= tmp & 1;
      lrc ^= (tmp & 1) << j;
      playBit(tmp & 1);
      tmp >>= 1;
    }
    playBit(crc);
  } 

  // finish calculating and send last "byte" (LRC)
  tmp = lrc;
  crc = 1;
  for (int j = 0; j < bitlen[track]-1; j++)
  {
    crc ^= tmp & 1;
    playBit(tmp & 1);
    tmp >>= 1;
  }
  playBit(crc);

  // if track 1, play 2nd track in reverse (like swiping back?)
  if (track == 0)
  {
    // if track 1, also play track 2 in reverse
    // zeros in between
    for (int i = 0; i < BETWEEN_ZERO; i++)
      playBit(0);

    // send second track in reverse
    reverseTrack(2);
  }

  // finish with 0's
  for (int i = 0; i < 5 * 5; i++)
    playBit(0);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(ENABLE_PIN, LOW);

}



// stores track for reverse usage later
void storeRevTrack(int track)
{
  int i, tmp, crc, lrc = 0;
  track--; // index 0
  dir = 0;

  for (i = 0; tracks[track][i] != '\0'; i++)
  {
    crc = 1;
    tmp = tracks[track][i] - sublen[track];

    for (int j = 0; j < bitlen[track]-1; j++)
    {
      crc ^= tmp & 1;
      lrc ^= (tmp & 1) << j;
      tmp & 1 ?
        (revTrack[i] |= 1 << j) :
        (revTrack[i] &= ~(1 << j));
      tmp >>= 1;
    }
    crc ?
      (revTrack[i] |= 1 << 4) :
      (revTrack[i] &= ~(1 << 4));
  } 

  // finish calculating and send last "byte" (LRC)
  tmp = lrc;
  crc = 1;
  for (int j = 0; j < bitlen[track]-1; j++)
  {
    crc ^= tmp & 1;
    tmp & 1 ?
      (revTrack[i] |= 1 << j) :
      (revTrack[i] &= ~(1 << j));
    tmp >>= 1;
  }
  crc ?
    (revTrack[i] |= 1 << 4) :
    (revTrack[i] &= ~(1 << 4));

  i++;
  revTrack[i] = '\0';
}

void stringCallback(char *track) {
  tracks[1] = track;
  noInterrupts();
  playTrack(2);
  delay(400);

  interrupts();
  Firmata.sendString(track);
}

void sysexCallback(byte command, byte argc, byte *argv) {
  stringCallback((char*) argv);
  Firmata.sendSysex(command, argc, argv);
}

void setup() {
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // blink to show we started up
  blink(ENABLE_PIN, 200, 3);
  
  // store reverse track 2 to play later
  storeRevTrack(2);
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.begin(57600);
}

void loop() {
  while (Firmata.available()) {
    Firmata.processInput();
  }
}


