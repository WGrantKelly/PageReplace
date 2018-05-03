#include "vmsim.h"
//input validation and mode declaration
int tau, refresh, memAccesses, faults, writes, numFrames;
char alg;

unsigned int *pages = NULL;
unsigned int *frames = NULL;

struct frame *frameStruct = NULL;

struct nodeClock *firstNode = NULL;
struct nodeClock *handNode = NULL;

struct page *newPage = NULL;

struct frame *head;
struct frame *this;

int main(int argc, char *argv[])
{


	//Accepting a string in the following format: ./vmsim –n numframes -a opt|clock|aging|work [-r refresh] [-t tau] tracefile;
	//4 kibibyte paging is 2^12 offset.

	if(checkParams(argc, argv))
	{
		printf("fml 0\n");
		return 1;
	}
	printf("ALG : %c\n", alg);
	if(argc>6)
	{
		//After proper input is confirmed, get values and place them into variable	
		tau = atoi(argv[6]);
		printf("tau: %d\n", tau); 
		refresh = atoi(argv[8]);
		printf("refresh: %d\n", refresh);
	}


	//save filename into tracefile
	char *tracefile = argv[argc-1];
	printf("tracefile: %s\n", tracefile);

	//save number of frames denoted by user into numFrames
	numFrames = atoi(argv[2]);
	printf("numFrames: %d\n", numFrames);

	
	FILE *file;
	memAccesses = 0;
	unsigned int addr;
	char mode;


	//open the file
	if( (file = fopen(tracefile, "r+")) == NULL)
	{
	    printf("No such file\n");
	    return(1);
	}  
	//scan file for number of accesses
	while(fscanf(file, "%x %c", &addr, &mode))
	{
		//OPT specific code here to handle every line one at a time
		memAccesses++;
		if (feof(file)) 
		{
		  break;
		}
	}
	rewind(file);
	printf("WHAT THE HECK\n");


	//Create array structures for page table and frame table so that file can be closed and stored in stack
	unsigned int pageArr[memAccesses];
	unsigned char modeArr[memAccesses];
	int i = 0;

	//enter file data into arrays for use in stack

	while(fscanf(file, "%x %c", &addr, &mode))
	{
		pageArr[i] = addr;
		modeArr[i] = mode;
		if (feof(file)) 
		{
		  break;
		}
		i++;
	}


	//we don't need the file anymore
	if(fclose(file))
	{
		printf("There was an error closing the file \n");
	    return(1);
	}
	printf("fml 1\n");

	
	//create frames data structure representing physical memory in RAM to be occupied by virtual pages

	frames = malloc(4096*numFrames);
	memset(frames, 0, 4096*numFrames);

	struct frame *frameStruct = malloc(sizeof(struct frame));
	memset(frameStruct, 0, sizeof(struct frame));

	head = frameStruct;

	int j;
	for(j=0; j<numFrames; j++)
	{
		frameStruct->frameIndex=j;
		frameStruct->address = frames+((j*4096)/4);
		frameStruct->virtAddr=0;
		frameStruct->pagePointer=NULL;
		frameStruct->next = malloc(sizeof(struct frame));
		printf("new frame made\n");
		frameStruct = frameStruct->next;
		memset(frameStruct, 0, sizeof(struct frame));
	}
	
	//populate the linked list section for the clock algorithm.

			//first node is 12 o'clock, or the first node of the clock
	firstNode = (struct nodeClock *)malloc(sizeof(struct nodeClock));
	firstNode->frameStruct = head;

	//handNode is the dial around the clock that is currently being referenced
	handNode = firstNode;
	this = head->next;
	int k;

	for(k = 1; k<numFrames; k++)
	{
		handNode->next = (struct nodeClock *)malloc(sizeof(struct nodeClock));
		handNode = handNode->next;
		//for every new node in the clock, update the corresponding frame for that node
		handNode->frameStruct = this;
		this = this->next;
	}
	//tie it all back together to the front and set the hand node as the starting node again
	handNode->next = firstNode;
	handNode = firstNode;

	//create virtual page data structure representing virtual memory 
	printf("fml 2\n");
	pages = malloc(4*1048576);
	memset(pages, 0, 4*1048576);
	
	//create the frame array structure that represents the number of frames in memory
	//unsigned int frameArr[numFrames];

	
	//right now we have 3 arrays. 
	//The pageArr array holds all of the page indeces accessed in order from oldest to most recent
	//The modeArr array holds all of the read/write data accessed in order from oldest to most recent
	//The frameArr represents which page indeces are currently in use in RAM
	


	//int ramFull = 0;
	//int ramIndex = 0;
	//int pageIndex = 0;

/*
	//fill frames until RAM is full
	while (ramFull == 0)
	{
		if(framesFull(frameArr, numFrames))
		{
			ramFull = 1;
			break;
		}

		//check and see if a page is already loaded in RAM, if its not loaded, load it. If it is, move on to the next mem access
		if(!isPageLoaded(pageArr[pageIndex], frameArr, numFrames))
		{
			frameArr[ramIndex] = pageArr[pageIndex];
			ramIndex++;
		}
		pageIndex++;
		
	}

*/
	


	/*

	switch(alg)
	{
		//int i;
		//If we are using optimal page replacing, we are only going to need which memory access were on, and our page array and frame array
		case 'o':
		
			for(i = pageIndex; i<memAccesses/10;i++)
			{
				if(!isPageLoaded(pageArr[i], frameArr, numFrames))
				{
					//If the page we need is not loaded in memory, this is a PAGE FAULT. Increment page fault count.
					OPT(pageArr, memAccesses, i, frameArr, numFrames);
					faults++;
				}
				
			}
		
			break;
		case 'c':
			

			break;
		case 'a':
			
			break;
		case 'w':
			
			break;
	}
*/
	printf("fml 3\n");
	//At this point, we have algorithm specific code ready to go, we now need to begin iterating through the memory accesses
	
	unsigned int address = 0;
	unsigned char mode2;
	unsigned int lastEviction = 0;
	int found = 0;
	faults = 0;
	writes = 0;
	struct page *newerPage = NULL;
	int z;
	int evict = 0;
	for(z = 0; z<memAccesses; z++)
	{
		printClock();
		address = pageArr[z];
		mode2 = modeArr[z];
		found = 0;

		
		newerPage = (struct page *) fault(address);
		//check if page exists in RAM, if not, throw a page fault and start an algorithm
		this=head;
		
		while(this->next)
		{
			if(this->address==newerPage->frameAddress)
			{
				if(newerPage->inRAM)
				{
					printf("NO EVICTION NEEDED\n");
					this->virtAddr = address;
					found = 1;
				}
				break;
			}
			else
			{

				this = this->next;
			}
		}
		
		if(!found)
		{
			printf("page isnt in RAM\n");
			//call the algorithm specified by the user to pick a frame to replace.
			switch(alg)
			{
				int i;
				case 'o':
					break;
				case 'c':
					printf("clock called\n");
					evict = clock();
					break;
				case 'a':
					
					break;
				case 'w':
					
					break;
			}
			//find frame with the right page id to evict
			this=head;
			
			while(this->next)
			{
				printf("current frame: %d\n",this->frameIndex);
				printf("need to evict frame: %d\n", evict);
				if(this->frameIndex == evict)
				{
					printf("found the frame we need to evict\n");
					faults++;
					lastEviction=this->virtAddr;
					if(this->pagePointer)
					{
						printf("pagePointer removed from ram and dereferenced\n");
						this->pagePointer->inRAM = 0;
						this->pagePointer->referenced = 0;
						if(this->pagePointer->dirty)
						{
							printf("write mode encountered\n");
							this->pagePointer->dirty = 0;
							writes++;
							
						}
						
					}
					//move page into frame that a page just got evicted from
				
					this->pagePointer = (struct page *) newerPage;
					newerPage->frameAddress=this->address;
					newerPage->inRAM=1;
					this->virtAddr=address;
					if(mode2 == 'W'){
						newerPage->dirty = 1;
					}
					break;
				}
				this = this->next;
			}
		}
	}
	

	printf("Number of Frames:\t\t %d \n", numFrames);
	printf("Total memory accesses:\t\t %d \n", memAccesses);
	printf("Total page faults:\t\t %d \n", faults);
	printf("Total writes to disk:\t\t %d \n", writes);

	return 0;

}
struct frame * fault(unsigned int address)
{
	//check if page exists in page table (pages)

		newPage = (struct page *) pages[PINDEX(address)];

		//if it doesn't, add it
		if(!newPage){
			printf("page isnt in page table\n");
			
			newPage = (struct page *)malloc(sizeof(struct page));
			memset(newPage, 0, sizeof(struct page));
			
			newPage->inRAM = 0;
			newPage->frameAddress = NULL;
			pages[PINDEX(address)] = (unsigned int) newPage;
		}	
		//set referenced bit to 1, and add it to the page table

		newPage->referenced = 1;
		return (struct frame *)newPage;
}
int checkParams(int argc, char *argv[])
{
	
	/*if(strcmp(argv[1], "-n") != 0)
	{
		printf("malformed parameters .1\n");
		return 1;
	}*/
	if(strcmp(argv[3], "-a") != 0)
	{
		printf("malformed parameters .2\n");
		return 1;
	}
	printf("algorithm: %s\n", argv[4]);
	if(strcmp(argv[4], "opt") == 0)
	{
		alg = 'o';
	}
	else if(strcmp(argv[4], "clock") == 0)
	{
		printf("alg set to c\n");
		alg = 'c';
	}
	else if(strcmp(argv[4], "aging") == 0)
	{
		alg = 'a';
	}
	else if(strcmp(argv[4], "work") == 0)
	{
		alg = 'w';
	}
	else
	{
		printf("malformed parameters .3\n");
	}
	if(argc>6)
	{
		if(strcmp(argv[5], "-r") != 0)
		{
			printf("malformed parameters .4\n");
			return 1;
		}
		if(strcmp(argv[7], "-t") != 0)
		{
			printf("malformed parameters .5\n");
			return 1;
		}
	}
	return 0;
}
/*
int checkFault(unsigned int pageAddr)
{

}
//return 0 if there is space in the frame array, return 1 if RAM is full
int framesFull(unsigned int frameArr[], int numFrames)
{
	int i = 0;
	for(i=0;i<numFrames;i++)
	{
		if (frameArr[i]==0)
		{
			return 0;
		}
	}
	return 1;
}
//return 1 if page already exists in RAM, return 0 if not
int isPageLoaded(unsigned int pageNum, unsigned int frameArr[], int numFrames)
{
	int i = 0;
	for(i=0;i<numFrames;i++)
	{
		if (frameArr[i]==pageNum)
		{
			return 1;
		}
	}
	return 0;
}
int distanceToNextAccess(int frameVal, unsigned int pageArr[], int pageIndex, int memAccesses)
{
	int i;
	int dist = 0;
	for(i = pageIndex; i<memAccesses; i++)
	{
		if(pageArr[i]==frameVal)
		{
			return dist;
		}
		else
		{
			dist++;
		}
	}
	return i;
}
*/
/*
compulsory miss: when a block of main memory is trying to occupy fresh empty line of cache, it is called compulsory miss
conflict miss: when still there are empty lines in the cache, block of main memory is conflicting with the already filled line of cache, ie., even when empty place is available, block is trying to occupy already filled line. its called conflict miss
capacity miss: miss occured when all lines of cache are filled.
conflict miss occurs only in direct mapped cache and set-associative cache. Because in associative mapping, no block of main memory tries to occupy already filled line. 
*/




int OPT(unsigned int pageArr[], int memAccesses, int pageIndex, unsigned int frameArr[], int numFrames)
{
/*
	//Simulate what the optimal page replacement algorithm would choose if it had perfect knowledge
	//We can simulate OPT because we already know the memory accesses that will happen in advance
	//Every time a page fault occurs, we need to replace the frame in memory that will be accessed last out of all of the current frames, and put in the page that is trying to be accessed

	int i = 0, distRet = 0;
	int currMaxDist = 0, currMaxFrameIndex = 0;

	for(i=0;i<numFrames;i++)
	{
		distRet = distanceToNextAccess(frameArr[i], pageArr, pageIndex, memAccesses);
		if(distRet>currMaxDist)
		{
			currMaxFrameIndex=i;
		}
	}

	//Optimal page replacement algorithm says that if page fault occurs then that page should be removed that will not be used for maximum time in future.

	//remove the frame and replace it with the memory that needs to be accessed.
	frameArr[currMaxFrameIndex] = pageArr[pageIndex];
	return 0;
*/
	return 0;
}
int clock(){
	int evict;
	//Implement a circular queue improvement to the second chance algorithm as described in lecture
	//if the handNode page does not exist in memory
	if(handNode->frameStruct->pagePointer==NULL)
	{
		evict = handNode->frameStruct->frameIndex;
		handNode = handNode->next;
		printf("EMPTY SLOT FOUND IN THE CLOCK\n");
	}
	else
	{
		while(handNode->frameStruct->pagePointer->referenced)
		{
			handNode->frameStruct->pagePointer->referenced = 0;
			handNode = handNode->next;
		}
		evict = handNode->frameStruct->frameIndex;
		printf("a page was evicted from a frame\n");
	}
	return evict;
}
int aging(){
	//Implement the use of an additional Referenced byte (8-bits) to implement aging
	return 0;
}
int WSClock(){
	/*
		Discussed in lecture, specifically:

	    Use the line number of the file as your virtual time
	    Use a tau value specified via a command line argument to determine that a page is out of the working set
	    On a page fault with no free frames available:
	        Scan the page table looking at valid pages, each time continuing from where you left off previously
	        If you find a page that is referenced, record the current virtual time as the last used time for that page
	        If you find a page that is unreferenced, older than tau, and clean, evict it and stop
	        If you find a page that is unreferenced and dirty, write it out to disk and mark it as clean
	        If you make it through the whole page table, evict the page with the oldest timestamp
	*/
	return 0;
}
int printClock()
{
	int i;
	handNode=firstNode;
	for(i=0;i<8;i++)
	{
		printf("frameIndex:\t%d\naddress:\t%d\nvirtAddr:\t%d\npagePointer:\t%p\n"
			,handNode->frameStruct->frameIndex, 
			handNode->frameStruct->address,
			handNode->frameStruct->virtAddr,
			handNode->frameStruct->pagePointer);
		handNode=handNode->next;
	}
	return 0;
}