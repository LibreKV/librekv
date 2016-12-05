#ifndef _CUCKOO_H_
#define _CUCKOO_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "util.h"    

// Incrementing keyfragbits above 15 requires
// more modifications to code (e.g. hashkey is 16 bits in (Insert())
#define KEYFRAGBITS (15)
#define KEYFRAGMASK ((1 << KEYFRAGBITS) - 1) 
#define INDEXBITS (16)
#define INDEXMASK ((1 << INDEXBITS) - 1)
#define VALIDBITMASK (KEYFRAGMASK+1)
#define EXCESS_BUCKET_FACTOR (1.1)
#define MAX_DELETED_RATIO (0.8)
#define MAX_LOAD_FACTOR (0.9)
#define PROBES_BEFORE_REHASH (8)
/*
 * parameters for cuckoo
 */
#define NUMHASH (2)
#define ASSOCIATIVITY (4)

#define NUMVICTIM (2) // size of victim table
#define MAX_CUCKOO_COUNT (128)
/*
 * make sure KEYFRAGBITS + VALIDBITS <= 16
 */

#define KEYPRESENTMASK (VALIDBITMASK | KEYFRAGMASK)
#define BUCKET_SIZE (sizeof(struct HashEntry) * ASSOCIATIVITY)
#define TABLE_SIZE  ((BUCKET_SIZE)*(1 >> KEYFRAGBITS))
#define BUCKETS_PER_TABLE (1 << KEYFRAGBITS)

/*
  Hash Entry Format
  D = Is slot deleted: 1 means deleted, 0 means not deleted.  Needed for lazy deletion
  V = Is slot empty: 0 means empty, 1 means taken
  K = Key fragment
  O = Offset bits
  ________________________________________________
  |DVKKKKKKKKKKKKKKOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO|
  ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
*/
    struct HashEntry {
	 uint16_t keyTag;
	 uint64_t keyHash;
	 uint64_t offset;
     }__attribute__((__packed__));
 
    struct Cuckoo_Bucket {
//	uint64_t item_vetor[ASSOCIATIVITY];
	struct HashEntry entryArray[ASSOCIATIVITY];
    }__attribute__((__packed__));

    struct CuckooHashTable {
	struct Cuckoo_Bucket bucketArray[BUCKETS_PER_TABLE];		
	int32_t num_buckets;
   	bool fixFlag;
     }__attribute__((__packed__));


    extern  struct CuckooHashTable* librekv_cuckoo_create();

    extern int librekv_cuckoo_fix(struct CuckooHashTable* targetTable);
    
    extern int librekv_cuckoo_drop(struct CuckooHashTable* targetTable);

    extern int librekv_cuckoo_dumpToNvm();

    extern int librekv_cuckoo_put(struct CuckooHashTable* cuckoo_hash_table, const char* key, const char* value);

    extern int librekv_cuckoo_immutable(struct CuckooHashTable* targetTable);

    extern int librekv_cuckoo_get(const uint8_t *key);

    extern int librekv_cuckoo_delete(const uint8_t *key);

#endif
