# 🏎️ hatsu_ino - Play custom sounds when you start

[![](https://img.shields.io/badge/Download-hatsu_ino-blue.svg)](https://github.com/coralynjungly552/hatsu_ino)

Hatsu_ino adds custom sound effects to your car. The device connects to your ignition. When you turn your car key, the system triggers a stored audio file. It uses an Arduino Nano to manage the sound playback. You hear your favorite car melodies or greetings every time you drive. 

## ⚙️ How it works

The system relies on a central controller board called an Arduino Nano. This board reads your car ignition signal. Once it detects power, it talks to a small memory card reader. It finds a specific sound file on the card and sends the signal to an amplifier. This amplifier pushes the sound through your speakers. The hardware uses a PAM8403 chip for clear audio output. It supports standard WAV files which keep the sound quality high.

## 📦 What you need

*   One Arduino Nano controller.
*   One Micro SD card module.
*   One PAM8403 amplifier module.
*   One Micro SD card formatted to FAT32.
*   Basic wiring tools including a soldering iron and wire cutters.
*   A stable 12V to 5V power converter for your car.
*   Your favorite music or sound clips saved as WAV files.

## 📥 Get the files

You need the software files from the project page. Visit the link below to reach the download area.

[Download hatsu_ino software from the page](https://github.com/coralynjungly552/hatsu_ino)

## 🛠️ Setting up the hardware

The wiring stage connects all parts to the Arduino Nano. Each module connects to specific pins on the board. The SD card module handles data transfer. The amplifier board connects to the audio output pins. You must solder these connections to ensure a steady bridge for electricity. Loose wires cause noise or system resets. Use heat shrink tubing to protect your joints from shorts. Secure the entire kit in a small box to prevent damage from car vibrations.

## 💾 Preparing the SD card

The Arduino requires specific file formats to play sounds. Insert your SD card into your computer. Format the drive to FAT32. Create a folder named "sounds" if the software requires it. Rename your WAV files to simple eight-character names without spaces. Place these files on the root of the card. The software looks for these specific files during the startup sequence. Confirm your files work on your computer play software before you put the card into the car unit.

## 🔌 Installing in the vehicle

Identify your ignition power wire. This wire provides electricity only when the key turns to the "on" position. Connect your power converter to this wire. Test the converter with a multimeter to ensure it outputs a clean 5V signal. Connect this power to your Arduino Nano. Connect the speaker wires to the output pins of your PAM8403 amplifier. Mount the speakers in a place with open air to ensure good sound projection. Test the ignition cycle multiple times while the car remains stationary.

## 🔊 Testing the audio

Turn your key. The Arduino Nano receives power. It checks the SD card. It loads the WAV file into the buffer. The sound plays through the speakers. If you hear nothing, check your wiring connections on the amplifier. If you hear static, verify your WAV file sample rate. The Arduino Nano works best with files at 16kHz or 22kHz sample rates. Adjust your WAV export settings if the audio sounds distorted or plays at the wrong speed. 

## ❓ Frequently asked questions

**Does this drain my battery?**
The system uses very little power. It only draws electricity when the ignition turns on. It does not pull power while the car sits parked.

**Can I change the sound later?**
Yes. Remove the SD card from the unit. Put it back into your computer. Replace the WAV file with a new one. Ensure you use the same filename so the software finds it easily.

**Is it difficult to install?**
You need basic skills with a soldering iron. If you have done minor car repairs, you can handle this installation. Follow the wiring diagram to avoid mistakes.

**What happens if I turn the key too fast?**
The Arduino Nano starts instantly. There is a slight delay of half a second while the system initializes the SD card. This ensures the sound plays from the very start of the track.

**Can I add more than one sound?**
The software included plays one file at a time. Advanced users can modify the code to select random files from a list. The current version focuses on a single, reliable startup sound. 

## 🎛️ Troubleshooting tips

*   Check your solder joints. A cold joint causes intermittent sound issues.
*   Verify your SD card format. Windows might default to exFAT; you must choose FAT32 for the Arduino to read it.
*   Look for a red light on the Arduino Nano. If it does not glow, the power supply has no connection.
*   Listen for a light hum. This means the amplifier works but the data signal does not reach it. Check the wires between the Arduino and the SD module.
*   Ensure the sound file ends cleanly. A corrupt file header stops the playback process. Download a tool to verify the health of your WAV files if you encounter skips.