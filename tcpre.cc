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


#define NUM_SETS_L1     1024
#define K_entries       2
#define SIZE_TAG        12  // bits


#define M_TAGBITS       9 
#define NUM_SETS_PHT   (2*pow(2,M_TAGBITS))
#define NUM_WAYS_PHT    8
#define NUM_ENTRIES_PHT 2

// THT - Tag History Table
vector< vector<int> > THT(NUM_SETS_L1,vector<int>(K_entries)); 


// PHT - Pattern History Table
vector<vector<vector<int> > > PHT;

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

#define OLDEST_TAG_POS 0
#define NEWEST_TAG_POS 1

static int maskBits(int number, int mask){

    return number & mask;

}


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
    PHT.resize(NUM_SETS_PHT);
    for (int i = 0 ; i < NUM_SETS_PHT ; i++ ) {
        PHT[i].resize(NUM_WAYS_PHT);
        for ( int j = 0; j < NUM_WAYS_PHT; j++) {
            PHT[i][j].resize(2);
            
            PHT[i][j][0] = 0;
            PHT[i][j][1] = 0;
        }
    }

    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
   
    //   cout << "Acces requested, memory adress: " << hex <<stat.mem_addr << dec << " \n"; 
    
    
    static long long int counter = 0;
    //  Phase 1 - Update. Note that THT[index][0] is oldest netry, THT[index][1] is newest
    if ( stat.miss && counter == 1) {
        
        int missIndex   = ( stat.mem_addr >> OFFSET_SIZE  ) & ( INDEX_MASK );   
        int missTag     = ( stat.mem_addr >> (OFFSET_SIZE + INDEX_SIZE) ) & ( TAG_MASK);
      
        int prev_oldestTag = THT[missIndex][OLDEST_TAG_POS];
        int prev_newestTag = THT[missIndex][NEWEST_TAG_POS]; 

        cout << "Address: "     << hex << stat.mem_addr << "\n";
        cout << "missIndex: "   << hex << missIndex << "\n";
        cout << "missTag : "    << hex << missTag << "\n";

        // 1. update THT sequence
        THT[missIndex][OLDEST_TAG_POS] = prev_newestTag;
        THT[missIndex][NEWEST_TAG_POS] = missTag;



          
        // 2. use tag sequence and missIndex to select a set  in PHT
        int tag0 = prev_newestTag;
        int tag1 = prev_oldestTag;

        int PHT_set =(tag0+tag1) >>  SIZE_TAG-M_TAGBITS; 
        

        // 3. search the set for newestTag
        




   }


    counter++;




    // Phase 2 - Lookup
 





}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
