#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//int framesFull(unsigned int frameArr[], int numFrames);
//int isPageLoaded(unsigned int pageNum, unsigned int frameArr[], int numFrames);

#define PINDEX(x)  (((x) >> 12) & 0xfffff)
#define FINDEX(x) ( (x) & 0xfff)

int OPT();
int clock();
int aging();
int WSClock();
int checkParams(int argc, char *argv[]);
struct frame *fault(unsigned int address);
int printClock();
struct page
{
	unsigned int *frameAddress;
	unsigned int dirty;
	unsigned int referenced;
	unsigned int inRAM;
};
struct frame
{
	unsigned int frameIndex;
	unsigned int *address;
	unsigned int virtAddr;
	struct page *pagePointer;
	struct frame *next;
};
struct nodeClock
{
	struct frame *frameStruct;
	struct nodeClock *next;
};