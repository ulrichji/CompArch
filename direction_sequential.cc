
#include "interface.hh"

#define NBLOCKS 1

Addr previous_address;


void prefetch_init(void)
{
	previous_address = 0;
    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
	Addr addr = stat.mem_addr;
	Addr dir = addr - previous_address;
	if(dir >= 0)
		dir = 1;
	else
		dir = -1;
	
	for(int i=0;i<NBLOCKS;i++)
	{
		/* pf_addr is now an address within the _next_ cache block */
		Addr pf_addr = stat.mem_addr + (BLOCK_SIZE * dir * (i+1));
		
		/*
		 * Issue a prefetch request if a demand miss occured,
		 * and the block is not already in cache.
		 */
		if (addr > 0 && stat.miss && !in_cache(pf_addr)) {
			issue_prefetch(pf_addr);
		}
	}
	previous_address = addr;
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
