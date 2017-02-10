/*
 * Implementation of a tag correlation based prefetcher  
 * 
 *
 */

#include "interface.hh"
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;


#define NUM_SETS_L1 1024
#define K_entries   2
#define SIZE_TAG    12  // bits


#define M_TAGBITS 7
#define NUM_SETS_PHT (2*pow(2,7))
#define NUM_ENTRIES_PHT 2

// THT - Tag History Table
vector< vector<int> > THT(NUM_SETS_L1,vector<int>(K_entries)); 


// PHT - Pattern History Table
vector< vector<int> > PHT(NUM_SETS_PHT,vector<int>(NUM_ENTRIES_PHT));

/*
Address bus width is 32 bits. Calculating the tags and indexes:


total: 28 bits
64 bytes of data in block = 6 bits offset // 22 remaining
1024 blocks ( =64kByts/(64 bytes/block) ) = 10 bits index, 12 bits remaining
tag = 12 bits

Unsure whether or not this is correct. The assumption of 28 bits is derived from the 
MAX_PHYS_MEM_SIZE variable, and could potentially be wrong.

*/

#define OFFSET_SIZE     6
#define INDEX_SIZE      10
#define TAG_SIZE        12

#define OFFSET_MASK      0x3F
#define INDEX_MASK      0x3FF
#define TAG_MASK        0xFFF

void prefetch_init(void)
{

    // init THT to 0
    for ( int i = 0; i < NUM_SETS_L1 ; i++ ) {
        for ( int j = 0 ; j < K_entries ; j++) {
            THT[i][j]=0;
           // cout <<  "THT element i: " << i << ", j: " << j << " = " << THT[i][j] <<" \n"; 
        }
    }


    // init PHT to 0
    for (int i = 0 ; i < NUM_SETS_PHT ; i ++ ) {
        for ( int j = 0; j < NUM_ENTRIES_PHT; j++) {
            PHT[i][j] = 0; 
        }
    }

    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
   
//   cout << "Acces requested, memory adress: " << hex <<stat.mem_addr << dec << " \n"; 
    //  Phase 1 - Update
    if ( stat.miss) {
        // 1. calculate the miss index, update corresponding THT sequence
        int missIndex = ( stat.mem_addr >> OFFSET_SIZE  ) & ( INDEX_MASK );   
        THT[missIndex][0]=THT[missIndex][1];
        THT[missIndex[1] = missIndex;

   
   }





    // Phase 2 - Lookup
 





}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
