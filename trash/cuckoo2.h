#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
    
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
         uint16_t present_key;
         uint32_t id;
     } __attribute__((__packed__));
  
struct TagValStoreEntry {
        char     tag_vector[ASSOCIATIVITY * ( KEYFRAGBITS + 1) / 8];
        uint32_t val_vector[ASSOCIATIVITY];
    } __attribute__((__packed__));

struct TagStoreEntry {
        char     tag_vector[ASSOCIATIVITY * ( KEYFRAGBITS + 1) / 8];
    } __attribute__((__packed__));


struct  TagValStoreEntry* hash_table_;
struct  TagStoreEntry*    fpf_table_;

    //HashEntry* hash_table_;
    size_t hash_table_size_;
    size_t current_entries_;
  
    // probe increment function coefficients
    double c_1_;
    double c_2_;

    static int librekv_cuckoo_create();
    static int librekv_cuckoo_open();

    static int librekv_cuckoo_flush();
    static int librekv_cuckoo_close();

    static int librekv_cuckoo_destroy();

 //   static int Status(const FawnDS_StatusType& type, Value& status) const;

    static int librekv_cuckoo_put(const uint8_t *key, const uint8_t *data);

 //   static int ConvertTo(FawnDS* new_store) const;

/*
    static FawnDS_ConstIterator Enumerate() const;
    static FawnDS_Iterator Enumerate();

    static FawnDS_ConstIterator Find(const ConstValue& key) const;
    static FawnDS_Iterator Find(const ConstValue& key);
*/
/*
    struct IteratorElem : public FawnDS_IteratorElem {
        FawnDS_IteratorElem* Clone() const;
        void Next();
        FawnDS_Return Replace(const ConstValue& data);
        uint32_t keyfrag[NUMHASH];
        uint32_t current_keyfrag_id;
        uint32_t current_index;
        uint32_t current_way;
    };
*/
    static int WriteToFile();
    static int ReadFromFile();

  //  TagValStoreEntry* hash_table_;
  //  TagStoreEntry*    fpf_table_;

    uint32_t  max_index_;
    uint32_t  max_entries_;
   // uint32_t  current_entries_;

    //inline int valid(int32_t index){
    //    return 0;
	// return (hash_table_[index]->present_key & VALIDBITMASK);
    //}

    inline uint16_t verifykey(int32_t index){
	return 0;
	//return (hash_table_[index]->present_key & KEYFRAGMASK);
    }
    
    inline uint16_t keyfragment_from_key(const uint8_t *key){
	if (strlen(&key) < sizeof(uint16_t)) {
           // return (uint16_t) (*(key) & KEYFRAGMASK);
        }
       // return (uint16_t) (((strlen(key))[key->length-2]<<8) + (key->data()[key->length-1])) & KEYFRAGMASK);
    }

    inline int valid(uint32_t index, uint32_t way){
        uint32_t pos = way * (KEYFRAGBITS + 1);
        uint32_t offset = pos >> 3;
        uint32_t tmp;
        // this is not big-endian safe, and it accesses beyond the end of the table
        /*
        if (hash_table_)
           tmp = *((uint32_t *) (hash_table_[index].tag_vector + offset));
        else
           tmp = *((uint32_t *) (fpf_table_[index].tag_vector + offset));
        tmp = tmp >> (pos & 7);
        */
        // specialized code
        assert(KEYFRAGBITS == 15);
        if (hash_table_)
            tmp = *((uint16_t *) (hash_table_[index].tag_vector + offset));
        else
            tmp = *((uint16_t *) (fpf_table_[index].tag_vector + offset));
        return (tmp & VALIDBITMASK);
    }

    // read from tagvector in the hashtable
    inline uint32_t tag(uint32_t index, uint32_t way){
        uint32_t pos = way * (KEYFRAGBITS + 1);
        uint32_t offset = pos >> 3;
        uint32_t tmp;
        // this is not big-endian safe, and it accesses beyond the end of the table
        /*
        if (hash_table_)
            tmp = *((uint32_t *) (hash_table_[index].tag_vector + offset));
        else 
            tmp = *((uint32_t *) (fpf_table_[index].tag_vector + offset));
        tmp = tmp >> (pos & 7);
        */
        // specialized code
        assert(KEYFRAGBITS == 15);
        if (hash_table_)
            tmp = *((uint16_t *) (hash_table_[index].tag_vector + offset));
        else
            tmp = *((uint16_t *) (fpf_table_[index].tag_vector + offset));
        return (tmp & KEYFRAGMASK);
    }

    inline uint32_t val(uint32_t index, uint32_t way){
        if (hash_table_)
            return hash_table_[index].val_vector[way];
        else
            return index * ASSOCIATIVITY + way;
    }

    // store keyfrag + validbit to  tagvector in the hashtable
    void store(uint32_t index, uint32_t way, uint32_t keypresent, uint32_t id) {
        assert(hash_table_);
        assert(way < ASSOCIATIVITY);
        uint32_t pos = way * (KEYFRAGBITS + 1);
        uint32_t offset = pos >> 3;
        uint32_t shift = pos & 7;
        // this is not big-endian safe, and it accesses beyond the end of the table
        /*
        uint32_t *p= (uint32_t *) (hash_table_[index].tag_vector + offset);
        uint32_t tmp = *p;
        */
        // specialized code
        assert(KEYFRAGBITS == 15);
        uint16_t *p= (uint16_t *) (hash_table_[index].tag_vector + offset);
        uint16_t tmp = *p;
        tmp &= ~( KEYPRESENTMASK << shift);
        *p = tmp | ( keypresent << shift);
        hash_table_[index].val_vector[way] = id;
    }


    /* read the keyfragment from the key */
    inline uint32_t keyfrag(const uint8_t *key, uint32_t keyfrag_id){
        assert(strlen(&key) == sizeof(uint8_t));
        // take the last 4 bytes
       // uint32_t tmp = *((uint32_t *) (key->data() + key->length - 4));
        tmp = (tmp >> (keyfrag_id * KEYFRAGBITS)) & KEYFRAGMASK;

#ifdef DEBUG
        // DPRINTF(2, "\t\t    key=\t");
        // print_payload((const u_char*) key.data(), key.size(), 4);
        // DPRINTF(2, "\t\t%dth tag=\t", keyfrag_index);
        // print_payload((const u_char*) &tmp, 2, 4);
#endif
        return tmp;
    }

    /* check if the given row has a free slot, return its way */
    uint32_t freeslot(uint32_t index)  {
        uint32_t way;
        for (way = 0; way < ASSOCIATIVITY; way++) {
            DPRINTF(4, "check ... (%d, %d)\t", index, way);
            if (!valid(index, way)) {
                DPRINTF(4, "not used!\n");
                return way;
            }
            DPRINTF(4, "used...\n");
        }
        return way;
    }
