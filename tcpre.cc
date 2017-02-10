/*
 * Implementation of a tag correlation based prefetcher  
 * 
 *
 */

#include "interface.hh"
#include <cmath>
#include <vector>

using namespace std;


#define NUM_SETS_L1 1024
#define K_entries   2
#define SIZE_TAG    64 // bits, expressed in the usage of "uint64_t" in the vectors


#define M_TAGBITS 7
#define NUM_SETS_PHT (2*pow(2,7))
#define NUM_ENTRIES_PHT 2

// THT - Tag History Table
vector< vector<uint64_t> > THT(NUM_SETS_L1,vector<uint64_t>(K_entries)); 


// PHT - Pattern History Table
vector< vector<uint64_t> > PHT(NUM_SETS_PHT,vector<uint64_t>(NUM_ENTRIES_PHT));



void prefetch_init(void)
{

    // init THT to 0
    for ( int i = 0; i < NUM_SETS_L1 ; i++ ) {
        for ( int j = 0 ; j < K_entries ; j++) {
            THT[i][j]=0;
            DPRINTF(HWPrefetch, "THT element ");//%i, %i = %i\n ",i,j,THT[i][j]);
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
   
    
    //  Phase 1 - Update







    // Phase 2 - Lookup
 





}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
