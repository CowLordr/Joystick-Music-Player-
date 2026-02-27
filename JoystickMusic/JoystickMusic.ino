#include <SoftwareSerial.h>
#include "DYSVAudio5W.h"

// Set up SoftwareSerial pins for communication with the DY-SV5W audio module
int RX_PIN = 10;
int TX_PIN = 11;
SoftwareSerial audioSerial(RX_PIN, TX_PIN);

// Initialize the audio player object
DYSVAudio5W player(audioSerial, 9600, Serial);

// Define digital pins for the arcade joystick directions
const int pinUp = 4;
const int pinDown = 5;
const int pinLeft = 6;
const int pinRight = 7;

// Define the different states the system can be in
enum State { IDLE, PLAYING, WAITING, FADING };
State state = IDLE;

// Variables to handle timing and volume control without using delay()
unsigned long releaseTime = 0;   // Stores the time when the joystick was released
unsigned long lastFadeTime = 0;  // Stores the time of the last volume decrease
int currentVolume = 30;          // Tracks the current volume (max is 30)
bool firstPlay = true;           // Flag to trigger random mode on the very first movement

// Raw hexadecimal commands used to control the audio module
uint8_t nextTrack[] = {0xAA, 0x06, 0x00, 0xB0};         // Plays the next track
uint8_t resumeMusic[] = {0xAA, 0x02, 0x00, 0xAC};       // Resumes playback from pause
uint8_t pauseMusic[] = {0xAA, 0x03, 0x00, 0xAD};        // Pauses playback
uint8_t randomMode[] = {0xAA, 0x18, 0x01, 0x03, 0xC6};  // Sets the playback mode to Random

// Function to dynamically set the volume using a raw hex command
void setVolume(uint8_t vol) {
  if (vol > 30) vol = 30; // Prevent volume from exceeding the maximum limit of 30
  
  // Calculate the required checksum for the volume command
  uint8_t checksum = (0xAA + 0x13 + 0x01 + vol) & 0xFF;
  uint8_t volCmd[] = {0xAA, 0x13, 0x01, vol, checksum};
  
  // Send the volume command to the module
  audioSerial.write(volCmd, sizeof(volCmd));
}

void setup() {
  Serial.begin(9600);
  audioSerial.begin(9600);
  
  // Give the module and SD card time to initialize upon power-up
  delay(1000);
  player.begin();
  delay(1000);

  // Set the joystick pins to use internal pull-up resistors
  // This means they read HIGH normally, and LOW when the switch connects them to Ground
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);
  pinMode(pinLeft, INPUT_PULLUP);
  pinMode(pinRight, INPUT_PULLUP);

  // Set the initial starting volume to max
  setVolume(30);
}

void loop() {
  // Check if any joystick direction is currently being pushed (reading LOW)
  bool isMoved = (digitalRead(pinUp) == LOW) || 
                 (digitalRead(pinDown) == LOW) || 
                 (digitalRead(pinLeft) == LOW) || 
                 (digitalRead(pinRight) == LOW);

  // Get the current running time in milliseconds
  unsigned long currentMillis = millis();

  // --- JOYSTICK IS MOVED ---
  if (isMoved) {
    // If the system wasn't already actively playing music
    if (state != PLAYING) {
      setVolume(30); // Instantly restore full volume
      delay(10);     // Brief pause to allow volume command to process
      
      // If waking up from a fully idle/paused state
      if (state == IDLE) {
        if (firstPlay) {
          // On the very first movement, send the Random Mode command, then play
          audioSerial.write(randomMode, sizeof(randomMode));
          delay(20);
          audioSerial.write(nextTrack, sizeof(nextTrack));
          firstPlay = false; // Ensure this block only runs once
        } else {
          // If it's not the first play, simply resume the paused track
          audioSerial.write(resumeMusic, sizeof(resumeMusic));
        }
      }
      state = PLAYING; // Update state to prevent spamming play commands
    }
  } 
  // --- JOYSTICK IS RELEASED ---
  else {
    if (state == PLAYING) {
      // The moment the joystick is let go, switch to WAITING state
      state = WAITING;
      releaseTime = currentMillis; // Record the exact time of release
    } 
    else if (state == WAITING) {
      // If it has been waiting for 10 seconds (10000 ms) without movement
      if (currentMillis - releaseTime >= 10000) {
        state = FADING;                 // Switch to the FADING state
        currentVolume = 30;             // Prepare to start fading from max volume
        lastFadeTime = currentMillis;   // Record the start time of the fade
      }
    } 
    else if (state == FADING) {
      // Drop the volume by 1 step every 66 milliseconds (takes about 2 seconds to reach 0)
      if (currentMillis - lastFadeTime >= 66) {
        lastFadeTime = currentMillis; // Reset the fade timer for the next step
        currentVolume--;              // Decrease volume by 1
        
        if (currentVolume > 0) {
          setVolume(currentVolume);   // Apply the newly lowered volume
        } else {
          // Once volume hits 0, pause the music and reset to IDLE state
          setVolume(0);
          delay(10);
          audioSerial.write(pauseMusic, sizeof(pauseMusic));
          state = IDLE;
        }
      }
    }
  }

  // A tiny loop delay for overall stability and switch debouncing
  delay(10);
}