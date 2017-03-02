#include "interface.hh"

#define DELTACOUNT 16
#define LOGSIZE 128

#define PREDICTIONLIMIT 4
#define OUTSTANDINGSIZE 32
#define BLOCKSIZE 64

#define USEOUTSTANDINGBUFFER 0
#define USELASTPREFETCH 1

typedef struct struct_log_item
{
	Addr pc;
	Addr lastAddr;
	Addr lastPrefetch;
	Addr deltas[DELTACOUNT];
	Addr deltaPointer;
	int deltaCount;
}LogItem;

LogItem log[LOGSIZE];
int nextReplace = 0;


Addr outstandingPrefetches[OUTSTANDINGSIZE];

//Function will issue a prefetch if the address is valid and if the adress is not in the cache.
void doPrefetch(Addr pf_addr)
{
	for(int i=0;i<OUTSTANDINGSIZE;i++)
	{
		if(USEOUTSTANDINGBUFFER == 0 || outstandingPrefetches[i] == 0)
		{
			if(pf_addr > 0 && pf_addr < MAX_PHYS_MEM_ADDR && !in_cache(pf_addr))
			{
				outstandingPrefetches[i] = pf_addr;
				issue_prefetch(pf_addr);
				break;
			}
		}
	}
}

//Function will reset the log item from the log at the specified index.
void resetLogItem(int index)
{
	log[index].pc = 0;
	log[index].lastAddr = 0;
	log[index].lastPrefetch = 0;
	for(int u=0;u<DELTACOUNT;u++)
		log[index].deltas[u] = 0;
	log[index].deltaPointer = 0;
	log[index].deltaCount = 0;
}

//Function is called before the prefetcher is used. It will reset all log items.
void prefetch_init(void)
{
	for(int i=0;i<LOGSIZE;i++)
	{
		resetLogItem(i);
	}
}

//Function will look through the log for a previous memory access from the same instruction.
//If it is found, it will update the log item with a new delta.
void updateLog(AccessStat stat)
{
	int found = 0;
	for(int i=0;i<LOGSIZE;i++)
	{
		if(log[i].pc == stat.pc)
		{
			log[i].deltas[log[i].deltaPointer] = (stat.mem_addr / BLOCKSIZE) - log[i].lastAddr;
			log[i].lastAddr = stat.mem_addr / BLOCKSIZE;
			
			//increment delta pointer and check if it wraps around.
			log[i].deltaPointer = log[i].deltaPointer + 1;
			if(log[i].deltaPointer >= DELTACOUNT)
				log[i].deltaPointer = 0;
				
			//Increment delta count and check if it overflows.
			log[i].deltaCount = log[i].deltaCount + 1;
			if(log[i].deltaCount > DELTACOUNT)
				log[i].deltaCount = DELTACOUNT;
			
			//Indicate that the log element is found so there is no need to add a new entry
			found = 1;
		}
	}
	
	//If the pc was not found, it must be added as a new entry in the log
	if(found == 0)
	{
		resetLogItem(nextReplace);
		log[nextReplace].pc = stat.pc;
		log[nextReplace].lastAddr = stat.mem_addr / BLOCKSIZE;
		nextReplace ++;
		if(nextReplace >= LOGSIZE)
			nextReplace = 0;
	}
}

//Function will search through the log and find a previous memory access of this instruction.
//It will issue a prefetch with the current memory address plus the last delta.
void predictLastDelta(AccessStat stat)
{
	Addr predAddr = 0;
	for(int i=0;i<LOGSIZE;i++)
	{
		if(log[i].pc == stat.pc && log[i].deltaCount > 0)
		{
			int prevDelta = log[i].deltaPointer - 1;
			if(prevDelta >= DELTACOUNT)
				prevDelta = 0;
			predAddr = log[i].lastAddr + log[i].deltas[prevDelta];
		}
	}
	
	doPrefetch(predAddr * BLOCKSIZE);
}

Addr predictPatternFromLogItem(LogItem predictionItem, AccessStat stat)
{
	Addr predictionBuffer [DELTACOUNT];

	int deltaPtr = predictionItem.deltaPointer;
	int deltaSize = predictionItem.deltaCount;

	int patternStart = deltaPtr;

	//Test all pattern sizes
	for(int patternSize = 1; patternSize <= predictionItem.deltaCount / 2; patternSize++)
	{
		//Copy the next deltas into the predictionBuffer to make it easiser to manipulate later.
		for(int i=0;i<patternSize;i++)
			predictionBuffer[patternSize - i - 1] = predictionItem.deltas[(deltaPtr - i - 1) % DELTACOUNT];
		
		//Count the amount of deltas that match the pattern.
		int lastPattern = -1;
		
		for(int i=0;i<predictionItem.deltaCount - patternSize;i++)
		{
			if(predictionBuffer[i % patternSize] == predictionItem.deltas[(deltaPtr - deltaSize + i) % DELTACOUNT])
				lastPattern = (deltaPtr - deltaSize + i) % DELTACOUNT;
		}
		
		//If it is the currently best match. Copy it to the largest prediction such that it can be predicted.
		if(lastPattern >= 0)
			patternStart = lastPattern;
	}
	
	//This part of the code is used to check if this pattern is predicted previously
	Addr prev_addr = stat.mem_addr / BLOCKSIZE;
	int drop = 0;
	int tempPatternStart = patternStart;
	
	while(USELASTPREFETCH && tempPatternStart != deltaPtr)
	{
		Addr pf_addr = prev_addr + predictionItem.deltas[tempPatternStart];
		if(pf_addr == predictionItem.lastPrefetch)
			drop = 1;
		tempPatternStart = (tempPatternStart + 1) % DELTACOUNT;
		prev_addr = pf_addr;
	}
	
	prev_addr = stat.mem_addr / BLOCKSIZE;
	while(drop == 0 && patternStart != deltaPtr)
	{
		Addr pf_addr = prev_addr + predictionItem.deltas[patternStart];
		doPrefetch(pf_addr * BLOCKSIZE);
		patternStart = (patternStart + 1) % DELTACOUNT;
		prev_addr = pf_addr;
	}
	return prev_addr;
}

void predictPattern(AccessStat stat)
{
	//The log item that is found at the stat's pc. It is it's deltas to use
	LogItem predictionItem;
	int predictionFound = 0;
	int predictionIndex = 0;
	
	//Search the list for a log item with the same pc as this item
	for(int i=0;i<LOGSIZE;i++)
	{
		if(log[i].pc == stat.pc && log[i].deltaCount > 0)
		{
			predictionFound = 1;
			predictionItem = log[i];
			predictionIndex = i;
			//We can break since there will only be one item in the log with this adress.
			break;
		}
	}
	
	//If it is in the list, the program will try to predict the pattern.
	if(predictionFound)
	{
		Addr lastPrefetch = predictPatternFromLogItem(predictionItem,stat);
		log[predictionIndex].lastPrefetch = lastPrefetch;
	}
}

void prefetch_access(AccessStat stat)
{
	updateLog(stat);
	
	predictPattern(stat);
}

void prefetch_complete(Addr addr)
{
	if(USEOUTSTANDINGBUFFER != 0)
	{
		for(int i=0;i<OUTSTANDINGSIZE;i++)
		{
			if(outstandingPrefetches[i] == addr)
				outstandingPrefetches[i] = 0;
		}
	}
}
