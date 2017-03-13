#include "interface.hh"

#define LOGSIZE 128

typedef struct struct_log_item
{
	Addr pc;
	Addr lastAddr;
}LogItem;

LogItem log[LOGSIZE];
int logPointer = 0;

void doPrefetch(Addr pf_addr)
{
	if(pf_addr >= 0 && pf_addr < MAX_PHYS_MEM_ADDR && !in_cache(pf_addr))
	{
		issue_prefetch(pf_addr);
	}
}

void prefetch_init(void)
{
	for(int i=0;i<LOGSIZE;i++)
	{
		log[i].pc = 0;
		log[i].lastAddr = 0;
	}
}

void prefetch_access(AccessStat stat)
{
	Addr delta = 0;
	int found = 0;
	for(int i=0;i<LOGSIZE;i++)
	{
		if(stat.pc == log[i].pc)
		{
			delta = stat.mem_addr - log[i].lastAddr;
			log[i].lastAddr = stat.mem_addr;
			found = 1;
			break;
		}
	}
	
	if(found == 0)
	{
		log[logPointer].pc = stat.pc;
		log[logPointer].lastAddr = stat.mem_addr;
		logPointer ++;
		if(logPointer >= LOGSIZE)
			logPointer = 0;
	}
	
	if(delta != 0)
		doPrefetch(stat.mem_addr + delta);
}

void prefetch_complete(Addr addr)
{

}
