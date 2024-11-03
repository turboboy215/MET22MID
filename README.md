# MET22MID
Metroid 2/Wario Land 1 (GB) to MIDI converter

This tool converts music from Game Boy games using Ryohji Yoshitomi's sound engine, used in two popular first-party games, to MIDI format. This sound engine appears to partially be based on Hirokazu Tanaka's sound engine.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex). For both games, the bank number is 5.

Examples:
* MET22MID "Metroid II - Return of Samus (W) [!].gb" 5
* MET22MID "Wario Land - Super Mario Land 3 (W) [!].gb" 5

This tool was based on information from the following disassembly: https://github.com/alex-west/M2RoS

Also included is another program, MET22TXT, which prints out information about the song data from each game. This is essentially a prototype of MET22MID.

Supported games:
  * Metroid II: Return of Samus
  * Wario Land: Super Mario Land 3

## To do:
  * Panning support
  * GBS file support
