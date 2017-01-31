/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

#define TABLE_SIZE 24

Addr pc_table [TABLE_SIZE];
Addr last_addr [TABLE_SIZE];

int table_pointer = 0;

void prefetch_init(void)
{
	table_pointer = 0;
	for(int i=0;i<TABLE_SIZE;i++)
	{
		pc_table[i] = 0;
		last_addr[i] = 0;
	}
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
	Addr pc = stat.pc;
	Addr mem_addr = stat.mem_addr;

	//values to search for in the table
	Addr prev_pc = 0;
	Addr prev_mem = 0;

	//Search the table to see if this instruction has been used before
	for(int i=0;i<TABLE_SIZE;i++)
	{
		//We need to calculate the index because we want to search for PC in a chronological manner from the oldest entry to the newest.
		int index = (i + table_pointer) % TABLE_SIZE;
		if(pc_table[index] == pc)
		{
			prev_pc = pc_table[index];
			prev_mem = last_addr[index];
		}
	}
	
	//Simply assume that 0 means that it didn't find the address.
	//This might be wrong, but it basically means that there are some strided access at the first instruction, which is mostly related to restarting the program.
	if(prev_pc != 0 || prev_mem != 0)
	{
		Addr stride = mem_addr - prev_mem;
		Addr pf_addr = mem_addr + stride;
		if(pf_addr >= 0 && stat.miss && !in_cache(pf_addr))
		{
			issue_prefetch(pf_addr);
		}
	}

    /* pf_addr is now an address within the _next_ cache block */
    Addr pf_addr = stat.mem_addr + BLOCK_SIZE;

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */
    if (stat.miss && !in_cache(pf_addr)) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
