#include "interface.hh"

#define DELTACOUNT 10
#define LOGSIZE 128

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

void prefetch_init(void)
{
	for(int i=0;i<LOGSIZE;i++)
	{
		resetLogItem(i);
	}
}

void updateLog(AccessStat stat)
{
	int found = 0;
	for(int i=0;i<LOGSIZE;i++)
	{
		if(log[i].pc == stat.pc)
		{
			log[i].deltas[log[i].deltaPointer] = stat.mem_addr - log[i].lastAddr;
			log[i].lastAddr = stat.mem_addr;
			
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
		log[nextReplace].lastAddr = stat.mem_addr;
		nextReplace ++;
		if(nextReplace >= LOGSIZE)
			nextReplace = 0;
	}
}

Addr getDelta(AccessStat stat)
{
	Addr predAddr = 0;
	for(int i=0;i<LOGSIZE;i++)
	{
		if(log[i].pc == stat.pc && log[i].deltaCount > 0)
		{
			int prevDelta = log[i].deltaPointer - 1;
			if(prevDelta >= DELTACOUNT)
				prevDelta = DELTACOUNT - 1;
			predAddr = log[i].lastAddr + log[i].deltas[prevDelta];
		}
	}
	
	return predAddr;
}

void doPrefetch(Addr pf_addr)
{
	if(pf_addr > 0 && pf_addr < MAX_PHYS_MEM_ADDR && !in_cache(pf_addr))
	{
		issue_prefetch(pf_addr);
	}
}

void prefetch_access(AccessStat stat)
{
	updateLog(stat);
	
	Addr pf_addr = getDelta(stat);
	
	doPrefetch(pf_addr);
}

void prefetch_complete(Addr addr)
{

}
