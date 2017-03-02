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


#define M_TAGBITS       8 
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

uint64_t missCounter = 0;
uint64_t hitCounter = 0;


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

static int countBits( uint64_t input ) {

    for ( int i = 0; i < 64 ; i++ ){
        if (!(input >> i)){
            return 64-i;
        }
    }
    return 64;
}

static uint32_t createLowerBitsMask ( int numberOfLowbits ) {
    
    uint32_t mask = 0;
    for ( int i = 0; i < numberOfLowbits ; i++ ) {
        mask |= ( 1 << i );
    }

    return mask;
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

    if ( stat.miss) {
        missCounter++;
    }else{ 
        hitCounter++;
    }
    if (stat.miss) {

        

        Addr missIndex   = ( stat.mem_addr >> OFFSET_SIZE  ) & ( INDEX_MASK );   
        Addr missTag     = ( stat.mem_addr >> (OFFSET_SIZE + INDEX_SIZE) ) & ( TAG_MASK);
      
        Addr prev_oldestTag = THT[missIndex][OLDEST_TAG_POS];
        Addr prev_newestTag = THT[missIndex][NEWEST_TAG_POS]; 

        /*if ( counter%100 == 0 ){
            cout << "----------------------------------------" << endl;
            cout << dec << "Print number " << counter << endl; 
            cout << "| Address: "     << hex << stat.mem_addr;
            cout << "| missIndex: "   << hex << missIndex;
            cout << "| missTag : "    << hex << missTag << "\n";
        }*/

        // 1.& 2. update THT sequence and calculate old PHT hash
        int PHT_set_pos = 0;
        for ( int i = 0 ; i < (K_entries) ; i++ ){
            PHT_set_pos += THT[missIndex][i];
            THT[missIndex][i] = THT[missIndex][i+1];
        }
        PHT_set_pos += THT[missIndex][K_entries-1];
        THT[missIndex][K_entries-1] = missTag;

        PHT_set_pos = PHT_set_pos & createLowerBitsMask(M_TAGBITS);

        // 3. search the set for newestTag. Also locate least used 
        // position in case tag not found. If not found, insert into last replaced position
         
        uint32_t    tagPos          = NUM_WAYS_PHT;
        uint32_t    leastUsedVal    = 0xffffffff;
        uint32_t    leastUsedPos    = 0;
        uint32_t    tagk            = THT[missIndex][K_entries-2]; 
        
        for ( int i = 0; i < NUM_WAYS_PHT; i++) {   
            if ( PHT[PHT_set_pos][i][0] == tagk ){
                tagPos = i;
            }
            if ( PHT[PHT_set_pos][i][2] < leastUsedVal ){
                leastUsedVal = PHT[PHT_set_pos][i][2];
                leastUsedPos = i;
            }

        }

        // 4 if tag available:
        if ( tagPos < NUM_WAYS_PHT ){
            PHT[PHT_set_pos][tagPos][1] = missTag;    
            PHT[PHT_set_pos][tagPos][2]++;
        } else {
            PHT[PHT_set_pos][leastUsedPos][0] = tagk;
            PHT[PHT_set_pos][leastUsedPos][1] = missTag;
            PHT[PHT_set_pos][leastUsedPos][2] = 1;
        }
    }
        // Phase 2 - Lookup
        Addr missIndex   = ( stat.mem_addr >> OFFSET_SIZE  ) & ( INDEX_MASK );   
        Addr missTag     = ( stat.mem_addr >> (OFFSET_SIZE + INDEX_SIZE) ) & ( TAG_MASK);
        
        int PHT_lookup = 0;
        for ( int i = 0; i <  (K_entries) ; i++ ){
            PHT_lookup += THT[missIndex][i];
        }
        
        PHT_lookup = PHT_lookup & createLowerBitsMask(M_TAGBITS);
       
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

    if ( (counter > 10000 ) && ( counter < 12000) ){
        cout << "---------------------------------"<< endl;
        cout << "calculating next adress based on: " << endl;
        for ( int i = 0; i < (K_entries) ; i++ ){
            cout << " Tag"<< dec << i+1 << hex << ", " << THT[missIndex][i];
        } 
        cout << ", PHT lookup: " << dec << PHT_lookup << hex;
        cout << ", Lookup result: " << PHT[PHT_lookup][ways_pos][1] << ", Ways pos = " << ways_pos; 
        cout << ", MissIndex: " << missIndex << endl;
        cout << "Current address: " << stat.mem_addr << ", predicted next address: " << predictedAddress <<endl;
        cout << "Current queue size is: " << current_queue_size() << endl;
        cout << "Misses: " << dec << missCounter << endl;
        cout << "Hits  : " << dec << hitCounter << endl;
        cout << endl;
    }   

    if (/* stat.miss &&*/ predictionAvailable && (predictedAddress > 0) && !in_cache(predictedAddress)) {
        issue_prefetch(predictedAddress);

    }


    counter++;

}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
