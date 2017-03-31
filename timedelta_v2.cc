#include "interface.hh"
#include <limits.h>

#define LOGSIZE 256
#define WINDOWSIZE 10
#define BLOCKSIZE 1

typedef struct struct_log_item
{
	Addr pc;
	Addr lastAddr;
	Addr offsetPrediction;
	int loaded;
}LogItem;

LogItem windowTable [WINDOWSIZE];
LogItem table [LOGSIZE];

int windowSize = 0;
int windowPointer = 0;
int tablePointer = 0;

//Function will issue a prefetch if the address is valid and if the adress is not in the cache.
void doPrefetch(Addr pf_addr)
{
	if(pf_addr >= 0 && pf_addr < MAX_PHYS_MEM_ADDR && !in_cache(pf_addr))
	{
		issue_prefetch(pf_addr);
	}
}

void resetTableItem(int index)
{
	table[index].pc = 0;
	table[index].lastAddr = 0;
	table[index].offsetPrediction = 0;
	table[index].loaded = 0;
}

void prefetch_init(void)
{
	for(int i=0;i<LOGSIZE;i++)
		resetTableItem(i);
}

int findEntry(Addr pc)
{
	for(int i=0;i<LOGSIZE;i++)
	{
		if(table[i].pc == pc)
			return i;
	}
	return -1;
}

void updateLog(AccessStat stat)
{
	if(windowSize >= WINDOWSIZE)
	{
		LogItem windowItem = windowTable[windowPointer];
		Addr stride = stat.mem_addr - windowItem.lastAddr;
		int previousIndex = findEntry(windowItem.pc);
		if(previousIndex >= 0)
		{
			table[previousIndex].lastAddr = windowItem.lastAddr;
			table[previousIndex].offsetPrediction = stride;
		}
		else
		{
			table[tablePointer].pc = windowItem.pc;
			table[tablePointer].lastAddr = windowItem.lastAddr;
			table[tablePointer].offsetPrediction = stride;
			tablePointer = (tablePointer + 1) % LOGSIZE;
		}
	}
	
	windowTable[windowPointer].pc = stat.pc;
	windowTable[windowPointer].lastAddr = stat.mem_addr;
	windowTable[windowPointer].offsetPrediction = 0;

	windowPointer = (windowPointer + 1) % WINDOWSIZE;
	if(windowPointer == 0)
		windowSize = WINDOWSIZE;
}

void predictOffset(AccessStat stat)
{
	int previousIndex = findEntry(stat.pc);
	if(previousIndex >= 0)
	{
		Addr stride = table[previousIndex].offsetPrediction;
		Addr pf_addr = ((stat.mem_addr / BLOCKSIZE) + stride) * BLOCKSIZE;
		doPrefetch(pf_addr);
	}
}

void prefetch_access(AccessStat stat)
{
	updateLog(stat);
	
	predictOffset(stat);
}

void prefetch_complete(Addr addr)
{

}
