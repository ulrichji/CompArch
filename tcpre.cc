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


#define M_TAGBITS       11 
#define NUM_SETS_PHT   (pow(2,M_TAGBITS))
#define NUM_WAYS_PHT    8
#define NUM_ENTRIES_PHT 2

// THT - Tag History Table
vector< vector<int> > THT(NUM_SETS_L1,vector<int>(K_entries)); 


// PHT - Pattern History Table
vector<vector<vector<uint32_t> > > PHT;

/*
Address bus width is 32 bits. Calculating the tags and indexes:


total: 28 bits
64 bytes of data in block = 6 bits offset // 22 remaining
1024 blocks ( =64kByts/(64 bytes/block) ) = 10 bits index, 12 bits remaining
tag = remainder (12 bits, based on that the size of accesses are <=28 bits)

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

static void printPHT ( void ){


    for ( int i = 0; i < NUM_SETS_PHT ; i++ ) {
        cout << "PHT LINE: " << std::dec << i << std::hex << " ";
        for ( int j = 0 ; j < NUM_WAYS_PHT ; j++) {
            cout << " | [ ";
            
            cout << std::hex << PHT[i][j][0] << ", "; 
            cout << PHT[i][j][1] << ", ";
            cout << dec << PHT[i][j][2] << hex;
            cout << "]";
        
        }

        cout << " | " << endl;
    }   

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
        for ( int j = 0; j < (NUM_WAYS_PHT); j++) {
            PHT[i][j].resize(3);
            
            PHT[i][j][0] = 0; // tag marked
            PHT[i][j][1] = 0; // tag
            PHT[i][j][2] = 0; // numberOfTimesStored
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
    if ( counter == 50000) {
       

        printPHT();
       
    }
    if (stat.miss) {

        Addr missIndex   = ( stat.mem_addr >> OFFSET_SIZE  ) & ( INDEX_MASK );   
        Addr missTag     = ( stat.mem_addr >> (OFFSET_SIZE + INDEX_SIZE) ) & ( TAG_MASK);
      
        Addr prev_oldestTag = THT[missIndex][OLDEST_TAG_POS];
        Addr prev_newestTag = THT[missIndex][NEWEST_TAG_POS]; 

        if ( counter%100 == 0 ){
            cout << "----------------------------------------" << endl;
            cout << dec << "Print number " << counter << endl; 
            cout << "| Address: "     << hex << stat.mem_addr;
            cout << "| missIndex: "   << hex << missIndex;
            cout << "| missTag : "    << hex << missTag << "\n";
        }
        // 1. update THT sequence
        THT[missIndex][OLDEST_TAG_POS] = prev_newestTag;
        THT[missIndex][NEWEST_TAG_POS] = missTag;

        
          
        // 2. use tag sequence and missIndex to select a set  in PHT
        int tag2 = prev_newestTag;
        int tag1 = prev_oldestTag;

        int PHT_set =(tag1+tag2) >>  (SIZE_TAG-M_TAGBITS+1); 
        
        if ( counter%100 == 0){
            cout << "TAG1: " << tag1 << ", TAG2: " << tag2 << endl; 
            cout << " PHT_set non-truncated: "<< hex <<tag1+tag2 << "PHT_set truncated: "<<PHT_set <<"\n";
        }
        // 3. search the set for newestTag. Also located least used 
        // position in case tag not found. Ifnot found, insert into last replaced position
         
        uint32_t    tagPos          = NUM_WAYS_PHT;
        uint32_t    leastUsedVal    = 0xffffffff;
        uint32_t    leastUsedPos    = 0;
        
        for ( int i = 0; i < NUM_WAYS_PHT; i++) {   
            if ( PHT[PHT_set][i][0] == tag2 ){
                tagPos = i;
            }
            if ( PHT[PHT_set][i][2] < leastUsedVal ){
                leastUsedVal = PHT[PHT_set][i][2];
                leastUsedPos = i;
            }

        }

        // 4 if tag available:
        if ( tagPos < NUM_WAYS_PHT ){
            PHT[PHT_set][tagPos][1] = missTag;    
            PHT[PHT_set][tagPos][2]++;
        } else {
            PHT[PHT_set][leastUsedPos][0] = tag2;
            PHT[PHT_set][leastUsedPos][1] = missTag;
            PHT[PHT_set][leastUsedPos][2] = 1;
        }

   




        // Phase 2 - Lookup
 

        int PHT_lookup = ( tag2 + missTag ) >> ( SIZE_TAG-M_TAGBITS+1);
        bool predictionAvailable = false;
        Addr predictedAddress;
        int ways_pos = 0;
        for ( int i = 0; i < NUM_WAYS_PHT; i++ ) {
            if ( PHT[PHT_lookup][i][0] == missTag ) {    
         
                predictedAddress = PHT[PHT_lookup][i][1] << (INDEX_SIZE + OFFSET_SIZE);
                predictedAddress = predictedAddress | ( missIndex << OFFSET_SIZE);
        
                predictionAvailable = true;
                ways_pos = i;
                break;
        
            }
        }

        if ( counter % 100 == 0 ){
            cout << "calculating next adress based on: " << endl;
            cout << "Tag 2: " << tag2 << ", Miss tag: " << missTag << ", PHT lookup: " << dec << PHT_lookup << hex;
            cout << "Lookup result: " << PHT[PHT_lookup][ways_pos][1] << "Ways pos = " << ways_pos; 
            cout << ", MissIndex: " << missIndex << endl;
            cout << "Current address: " << stat.mem_addr << ", predicted next address: " << predictedAddress <<endl;
        }   

        if ( stat.miss && predictionAvailable && (predictedAddress > 0) && !in_cache(predictedAddress)) {
            issue_prefetch(predictedAddress);

        }

    }


    counter++;

}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
