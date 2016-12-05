#include <stdio.h>
#include "cuckoo.h"
#include "librekv.h"
#include "hash.h"
#include "log.h"

struct Cuckoo_Bucket* bucket;
struct CuckooHashTable* cuckoo_hash_table;
struct HashEntry* hashentry;

//char log_buffer[100];

static uint16_t max_index;
static uint32_t max_entries;
static uint32_t current_entries;

struct CuckooHashTable* librekv_cuckoo_create(){

	printf("librekv_cuckoo_create()\n");

//	struct CuckooHashTable* cuckoo_hash_table  = NULL;

	if (cuckoo_hash_table != NULL)
		return -1;

	max_index = BUCKETS_PER_TABLE;

	max_entries = ASSOCIATIVITY * max_index;

	current_entries = 0;

	printf("Create cuckoo table information:\n"
			"\t KEYFRAGBITS: %I16u\n"
			"\t hashtablesize: %I32u\n"
			"\t num_entries: %Iu32\n"
			"\t Maximum number of entries: %I32u\n",
			KEYFRAGBITS, max_index, current_entries, max_entries);

	uint64_t hash_table_size = 0;
	hash_table_size += sizeof(cuckoo_hash_table->bucketArray);
	hash_table_size += sizeof(cuckoo_hash_table->fixFlag);
	hash_table_size += sizeof(cuckoo_hash_table->num_buckets);

	cuckoo_hash_table = (struct CuckooHashTable*)malloc(hash_table_size);
	cuckoo_hash_table->num_buckets = max_index;
	cuckoo_hash_table->fixFlag = false;

	// zero out the buffer
	memset(cuckoo_hash_table, 0, hash_table_size);

	printf("librekv_cuckoo_create(): <result> byte size=%ld\n", hash_table_size);

	return cuckoo_hash_table;//hash_table_size;
	//return 0;
}

int librekv_cuckoo_fix(struct CuckooHashTable* targetTable){

	printf("librekv_cuckoo_fix()\n");

	if(targetTable->fixFlag == false){

		targetTable->fixFlag = true;
	}

	printf("librekv_cuckoo_fix():source hash table is fixed!\n");
	return 0;
}


int librekv_cuckoo_drop(struct CuckooHashTable* targetTable){

	printf("librekv_cuckoo_drop()\n");

	if(targetTable == NULL){

		printf("target hash table is already null!\n");

	} else{

		free(targetTable);

	}

	printf("librekv_cuckoo_drop():source hash table is dropped!\n");

	return 0;
}

/* read the keyfragment from the key */
extern uint32_t keyfrag(uint64_t*  key, uint16_t keyfrag_id) {
	
	assert(key >= sizeof(uint64_t));

	uint32_t tmp = (key);
        tmp = (tmp >> (keyfrag_id * KEYFRAGBITS)) & KEYFRAGMASK;

        return tmp;
}    

bool valid(struct CuckooHashTable* cuckoo_hash_table, uint16_t index, uint16_t way){

        uint32_t pos = way * (KEYFRAGBITS + 1); 
        uint32_t offset = pos >> 3;
        uint32_t tag;
        assert(KEYFRAGBITS == 15);
        if (cuckoo_hash_table){
            struct Cuckoo_Bucket* current_bucket = (struct Cuckoo_Bucket* )(&(cuckoo_hash_table->bucketArray[index]));
            struct HashEntry* current_entry = (struct HashEntry*)(&(current_bucket->entryArray[way]));
            tag = current_entry->keyTag;
        }
        return (tag & VALIDBITMASK);

}

/* check if the given row has a free slot, return its way */
uint32_t freeslot(struct CuckooHashTable* cuckoo_hash_table, uint32_t index)  {
        uint32_t way;
        for (way = 0; way < ASSOCIATIVITY; way++) {
            printf("check ... (%d, %d)\t", index, way);
            if (!valid(cuckoo_hash_table, index, way)) {
                printf("not used!\n");
                return way;
            }
            printf("used...\n");
        }
        return way;
    }  
 
extern  uint32_t tag(struct CuckooHashTable* cuckoo_hash_table, uint16_t index, uint16_t way){

        uint16_t tag;
	uint64_t keyhash;
	uint64_t value;
        assert(KEYFRAGBITS == 15);
        
	if (cuckoo_hash_table){
            struct Cuckoo_Bucket* current_bucket = (struct Cuckoo_Bucket* )(&(cuckoo_hash_table->bucketArray[index]));





	    struct HashEntry* current_entry = (struct HashEntry* )(&(current_bucket->entryArray[way]));
	    tag = current_entry->keyTag;
	    keyhash =current_entry->keyHash;
	    value = current_entry->offset;
	}

	return (tag & KEYFRAGMASK);
}


void store(struct CuckooHashTable* cuckoo_hash_table, uint16_t index, uint16_t way, struct HashEntry* hashEntry){
        assert(cuckoo_hash_table);
        assert(way < ASSOCIATIVITY);
        assert(KEYFRAGBITS == 15);

        struct Cuckoo_Bucket* current_bucket = (struct Cuckoo_Bucket* )(&(cuckoo_hash_table->bucketArray[index]));
	struct HashEntry* current_entry = (struct HashEntry* )(&(current_bucket->entryArray[way]));
	current_entry->keyTag = hashEntry->keyTag;
	current_entry->keyHash = hashEntry->keyHash;
	current_entry->offset = hashEntry->offset;

}

int librekv_cuckoo_put(struct CuckooHashTable* cuckoo_hash_table, const char* key, const char* value){

        printf("librekv_cuckoo_put():\n");

        if(key == NULL || value == NULL)
        	return -1;

        uint16_t current_index, current_tag;
	uint64_t current_hash;
	uint64_t current_offset;
	struct   HashEntry current_entry;

        uint16_t victim_way, victim_index, victim_tag;
	uint64_t victim_hash;
	uint64_t victim_offset;
	struct   HashEntry victim_entry;
        
	uint16_t way;
        uint32_t undo_index[MAX_CUCKOO_COUNT];
        uint8_t  undo_way[MAX_CUCKOO_COUNT];

	//the keyhash and the offset going to be inserted.
        current_hash   = hash((const uint8_t *)&key, sizeof(key));
	current_offset = (uint64_t *)value;	//*** Here need to be modify to the value offset in NVM

        uint16_t fn = rand() % NUMHASH;
        current_index  = keyfrag(key, fn) % max_index;
        current_tag    = keyfrag(key, (fn + 1) % NUMHASH) % max_index;
        
	if ((way = freeslot(cuckoo_hash_table, current_index)) < ASSOCIATIVITY) {
        	 current_entry.keyTag  = current_tag | VALIDBITMASK;
	         current_entry.keyHash = current_hash;
        	 current_entry.offset  = current_offset;
	   	 store(cuckoo_hash_table, current_index, way, &(current_entry));
           	 current_entries++;
        	 printf("librekv_cuckoo_put(): is successed.\n");
	         return 1;
        }

        fn = (fn + 1) % NUMHASH;
        current_index = keyfrag(key, fn)  % max_index;
        current_tag = keyfrag(key, (fn + 1) % NUMHASH)  % max_index;

	for (uint32_t n = 0; n < MAX_CUCKOO_COUNT; n++) {
            printf("%d th try in loop!\n", n);
            printf("key: (index %16d, tag %16d)\n", current_index, current_tag);
            if ((way = freeslot(cuckoo_hash_table, current_index)) < ASSOCIATIVITY) {
        	 current_entry.keyTag  = current_tag | VALIDBITMASK;
	         current_entry.keyHash = current_hash;
        	 current_entry.offset  = current_offset;
	       	 store(cuckoo_hash_table, current_index, way, &(current_entry));
                 current_entries++;
        	 printf("librekv_cuckoo_put(): is successed.\n");
		 return 1;
       	    }
        
	victim_index = current_index;
        victim_way   = rand() % ASSOCIATIVITY;
        struct HashEntry victimEntry;
        victimEntry = (struct HashEntry)(cuckoo_hash_table->bucketArray[victim_index].entryArray[victim_way]);
        victim_tag   = victimEntry.keyTag;
        victim_hash = victimEntry.keyHash;
        victim_offset   = victimEntry.offset; //**** here is not the real value but the value offset in NVM.
        undo_index[n] = victim_index;
        undo_way[n] = victim_way;

	printf("this bucket (%d) is full, take %d th as victim!\n", victim_index, victim_way);
        current_entry.keyTag  = current_tag | VALIDBITMASK;
	
        memset(log_buffer, 0, sizeof(log_buffer));
        sprintf(log_buffer, "current_tag is %16d, current_tag with valid bit is %16d, masked current_tag is %16d\n",current_tag, current_entry.keyTag,current_tag & KEYFRAGMASK);
        librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));

        current_entry.keyHash = current_hash;
        current_entry.offset  = current_offset;
        store(cuckoo_hash_table, victim_index, victim_way, &(current_entry));

        current_index = victim_tag & KEYFRAGMASK;// % max_index;
        current_tag   = victim_index  % max_index;
        // next need to be stored entry is the previous victim_entry
        current_hash  = victim_hash;
        current_offset = victim_offset;

  }

       memset(log_buffer, 0, sizeof(log_buffer));
       sprintf(log_buffer, "librekv_cuckoo_put(): no more space to put new key -- undoing cuckooing\n");
       librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));
       printf("librekv_cuckoo_put(): no more space to put new key -- undoing cuckooing\n");
       // undo cuckooing
       // restore the last state (find the previous victim_tag when calling last store())
       current_tag  = current_index  % max_index;
       for (uint32_t n = 0; n < MAX_CUCKOO_COUNT; n++) {
           victim_index  = undo_index[MAX_CUCKOO_COUNT - 1 - n];
           victim_way    = undo_way[MAX_CUCKOO_COUNT - 1 - n];
           struct HashEntry victimEntry;
           victimEntry = (struct HashEntry)(cuckoo_hash_table->bucketArray[victim_index].entryArray[victim_way]);
           victim_tag = victimEntry.keyTag;
	   victim_hash = victimEntry.keyHash;
	   victim_offset = victimEntry.offset;
	   
           printf("restoring victim (index %d tag %d)\n", victim_index, victim_tag);
           memset(log_buffer, 0, sizeof(log_buffer));
           sprintf(log_buffer, "restoring victim (index %d tag %d)\n", victim_index, victim_tag);
           librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));

           current_entry.keyTag  = current_tag | VALIDBITMASK;
           current_entry.keyHash = current_hash;
           current_entry.offset  = current_offset;
	   store(cuckoo_hash_table, victim_index, victim_way, &(current_entry));
            
	   current_tag   = victim_index  % max_index;
	   current_hash  = victim_hash;
           current_offset  = victim_offset;
      }

       memset(log_buffer, 0, sizeof(log_buffer));
       sprintf(log_buffer, "librekv_cuckoo_put(): <result> undoing done\n");
       librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));
       printf("librekv_cuckoo_put(): <result> undoing done\n");
       return -1;//INSUFFICIENT_SPACE;

}

int librekv_cuckoo_immutable(struct CuckooHashTable* targetTable){

	struct CuckooHashTable* immutable_cuckoo_table = NULL;
	immutable_cuckoo_table = librekv_cuckoo_create();

	if (targetTable == NULL) {
            printf( "librekv_cuckoo_immutable(): target cuckoo table is null!\n");
            return -1;
        }

	printf("fixFlag is %d\n",(uint16_t)targetTable->fixFlag);
        if (targetTable->fixFlag == true) {
            printf( "librekv_cuckoo_immutable(): target cuckoo table is already fixed!\n");
            return -1;
        }

	if(immutable_cuckoo_table == NULL){
	 	printf("failed to create immutable cuckoo hash table!\n");
		return -1;
	}

	if (max_index != immutable_cuckoo_table->num_buckets) {
            printf("librekv_cuckoo_immutable(): hash table size mismatch!\n");
	    printf("here4\n");
	    return -1;
        }

        printf("librekv_cuckoo_immutable():staring to fix the target cuckoo table.\n");
        if (targetTable != NULL && immutable_cuckoo_table != NULL){
	    memcpy(immutable_cuckoo_table, targetTable,sizeof(struct CuckooHashTable));
	}else{
            for (size_t i = 0; i < max_index; i++) {
                memcpy(&(immutable_cuckoo_table->bucketArray[i]), &(targetTable->bucketArray[i]), sizeof(struct Cuckoo_Bucket));
            }
	}
	immutable_cuckoo_table->fixFlag = true;
        printf("librekv_cuckoo_immutable():convert the target cuckoo table successfully.\n");
        return 1;

}

/*void dectobin(uint32_t n)
 {
     if(n/2>0)
     {  
         dectobin(n/2);
         printf("%d",n%2);
     } 
     else  
     {
         printf("%d",n);
     }  
 } */ 
