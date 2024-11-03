/*Metroid 2/Wario Land 1 (SML3) (Ryohji Yoshitomi) (GB) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long offset;
long tablePtrLoc;
long tableOffset;
int i, j;
char outfile[1000000];
int songNum;
long seqPtrs[4];
long songPtr;
int chanMask;
long bankAmt;
int foundTable = 0;
long firstPtr = 0;
int songTranspose = 0;

unsigned static char* romData;

const char MagicBytesM2[6] = { 0xE0, 0x25, 0xFA, 0xDC, 0xCE, 0x21 };
const char MagicBytesWL[6] = { 0xE0, 0x25, 0xFA, 0x1C, 0xA6, 0x21 };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptr);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Metroid 2/Wario Land 1 (SML3) (GB) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: MET22TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Try to search the bank for song table loader - Metroid 2*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesM2, 6)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i + 6;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
				}
			}

			/*Try to search the bank for song table loader - Wario Land*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesWL, 6)) && foundTable != 1)
				{
					tablePtrLoc = bankAmt + i + 6;
					printf("Found pointer to song table at address 0x%04x!\n", tablePtrLoc);
					tableOffset = ReadLE16(&romData[tablePtrLoc - bankAmt]);
					printf("Song table starts at 0x%04x...\n", tableOffset);
					foundTable = 1;
				}
			}

			if (foundTable == 1)
			{
				i = tableOffset - bankAmt;
				songNum = 1;
				while (ReadLE16(&romData[i]) < (bankSize * 2))
				{
					songPtr = ReadLE16(&romData[i]);
					printf("Song %i: 0x%04X\n", songNum, songPtr);
					if (songPtr != 0)
					{
						song2txt(songNum, songPtr);
					}
					i += 2;
					songNum++;
				}
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				exit(-1);
			}
		}

		printf("The operation was completed successfully!\n");
		return 0;
	}
}

/*Convert the song data to TXT*/
void song2txt(int songNum, long ptr)
{
	long romPos = 0;
	long seqPos = 0;
	long speedPtr = 0;
	int curTrack = 0;
	long chanPtrs[4];
	long curSeq = 0;
	int seqEnd = 0;
	int chanEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int repeat = 0;
	int transpose = 0;
	int tempo = 0;
	unsigned char command[4];
	long wavePos = 0;
	long loopPt = 0;
	int seqCtr = 0;
	unsigned long seqList[500];
	int k = 0;


	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		romPos = ptr - bankAmt;
		songTranspose = (signed char)romData[romPos] / 2;
		fprintf(txt, "Song transpose: %i\n", songTranspose);
		romPos++;
		speedPtr = ReadLE16(&romData[romPos]);
		fprintf(txt, "Speed table pointer: 0x%04X\n", speedPtr);
		romPos += 2;
		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			chanPtrs[curTrack] = ReadLE16(&romData[romPos]);
			fprintf(txt, "Channel %i: 0x%04X\n", (curTrack + 1), chanPtrs[curTrack]);
			romPos += 2;
		}
		fprintf(txt, "\n");

		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			fprintf(txt, "Channel %i:\n", (curTrack + 1));
			if (chanPtrs[curTrack] != 0 && chanPtrs[curTrack] < (bankSize * 2))
			{
				romPos = chanPtrs[curTrack] - bankAmt;

				while (ReadLE16(&romData[romPos]) != 0x00F0 && ReadLE16(&romData[romPos]) != 0x0000)
				{
					curSeq = ReadLE16(&romData[romPos]);
					seqList[seqCtr] = curSeq;
					fprintf(txt, "Sequence 0x%04X\n", curSeq);
					romPos += 2;
					seqCtr++;
				}

				if (romData[romPos] == 0xF0)
				{
					loopPt = ReadLE16(&romData[romPos + 2]);
					fprintf(txt, "Loop point: 0x%04X\n", loopPt);
				}
				else
				{
					fprintf(txt, "End song without loop\n");
				}
			}

			fprintf(txt, "\n");
		}

		for (k = 0; k < seqCtr; k++)
		{
			seqPos = seqList[k] - bankAmt;
			fprintf(txt, "Sequence 0x%04X:\n", seqList[k]);
			seqEnd = 0;

			while (seqEnd == 0)
			{
				command[0] = romData[seqPos];
				command[1] = romData[seqPos + 1];
				command[2] = romData[seqPos + 2];
				command[3] = romData[seqPos + 3];

				if (command[0] == 0x00)
				{
					fprintf(txt, "End of sequence\n\n");
					seqEnd = 1;
				}

				else if (command[0] == 0x01)
				{
					fprintf(txt, "Rest\n");
					seqPos++;
				}

				else if (command[0] == 0x03)
				{
					fprintf(txt, "Echo (v1)\n");
					seqPos++;
				}

				else if (command[0] == 0x05)
				{
					fprintf(txt, "Echo (v2)\n");
					seqPos++;
				}

				else if (command[0] < 0xA0)
				{
					curNote = command[0];
					fprintf(txt, "Play note: %01X\n", curNote);
					seqPos++;
				}

				else if (command[0] >= 0xA0 && command[0] <= 0xAE)
				{
					curNoteLen = command[0];
					fprintf(txt, "Change note length: %01X\n", curNoteLen);
					seqPos++;
				}

				else if (command[0] == 0xF1)
				{
					fprintf(txt, "Set channel parameters: %01X, %01X, %01X\n", command[1], command[2], command[3]);
					seqPos += 4;
				}

				else if (command[0] == 0xF2)
				{
					speedPtr = ReadLE16(&romData[seqPos + 1]);
					fprintf(txt, "Change speed table: %01X\n", speedPtr);
					seqPos += 3;
				}

				else if (command[0] == 0xF3)
				{
					transpose = (signed char)command[1];
					fprintf(txt, "Set transpose: %i\n", transpose);
					seqPos += 2;
				}
				
				else if (command[0] == 0xF4)
				{
					repeat = command[1];
					fprintf(txt, "Repeat section %i times\n", repeat);
					seqPos += 2;
				}

				else if (command[0] == 0xF5)
				{
					fprintf(txt, "End of repeat\n");
					seqPos++;
				}

				else
				{
					fprintf(txt, "Unknown command: %01X\n", command[0]);
					seqPos++;
				}
			}
		}
		fclose(txt);
	}

}