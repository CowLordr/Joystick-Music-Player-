# Joystick Music Player 

This project uses an Arduino Uno, an arcade joystick, and a DY-SV5W audio module to create a movement-triggered music player that shuffles and fades.

## **Features**

* **Movement Triggered:** Music starts instantly when the joystick is moved.  
* **Random Shuffle:** Shuffles to a random song on the SD card when first activated.  
* **Resume Playback:** Pauses when inactive and resumes from the exact spot when moved again.  
* **Smart Timer:** Music continues for 10 seconds after releasing the joystick.  
* **Smooth Fade-Out:** 2-second volume fade-out before pausing.

## **Hardware Components**

* Arduino Uno  
* Arcade Joystick (5-pin/4-way)  
* DY-SV5W 5W Audio Module  
* 4Ω/8Ω Speaker  
* Micro SD Card (formatted FAT32)

## **Wiring**

| Component | Pin | Arduino Pin |
| :---- | :---- | :---- |
| Joystick | GND | GND |
| Joystick | Up | D4 |
| Joystick | Down | D5 |
| Joystick | Left | D6 |
| Joystick | Right | D7 |
| Audio | TX | D10 (RX) |
| Audio | RX | D11 (TX) |

## **Installation**

1. Install the DYSVAudio5W library in the Arduino IDE.  
2. Upload the ArcadeJoystickMusic.ino code to your Arduino Uno.  
3. Place your .mp3 files on a FAT32 formatted SD card and insert it into the module.  
4. Move the joystick to start the music.
