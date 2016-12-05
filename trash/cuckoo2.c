#include "cuckoo.h"
#include "librekv.h"

#define USE_OFFSET  1
#define TABLE_SIZE  2UL*1024*1024

int librekv_cuckoo_create()
    {
      DPRINTF(2, "librekv_cuckoo_create()\n");

        if (hash_table_ || fpf_table_)
            Close();

        /*string hts_string = config_->GetStringValue("child::hash-table-size");
        int hts_int = atoi(hts_string.c_str());
        if (hts_int <= 0) {
            return ERROR;
        }
        uint64_t table_size = (uint64_t)hts_int;
        */

#ifdef SPLIT_HASHTABLE
        max_index_ = (1 << KEYFRAGBITS) * NUMHASH;
#else
        max_index_ = (1 << KEYFRAGBITS);
#endif

        if (max_index_ * ASSOCIATIVITY > TABLE_SIZE)
            max_index_ = TABLE_SIZE / ASSOCIATIVITY;

        max_entries_ = ASSOCIATIVITY * max_index_;
        current_entries_ = 0;

        DPRINTF(2, "librekv_cuckoo_create(): given table_size=%llu\n", (long long unsigned)(max_entries_));


        DPRINTF(2, "CreateCuckoo table information:\n"
                "\t KEYFRAGBITS: %ld\n"
                "\t hashtablesize: %ld\n"
                "\t num_entries: %ld\n"
                "\t Maximum number of entries: %ld\n",
                KEYFRAGBITS, max_index_, current_entires_, max_entries_);

        //if (config_->ExistsNode("child::use-offset") != 0 || atoi(config_->GetStringValue("child::use-offset").c_str()) != 0) {
          if(USE_OFFSET){
            struct TagValStoreEntry hash_table_[max_index_];
            fpf_table_  = NULL;

            // zero out the buffer
            memset(hash_table_, 0, sizeof(TagValStoreEntry) * max_index_);

            DPRINTF(2, "librekv_cuckoo_create(): <result> byte size=%zu\n", sizeof(TagValStoreEntry) * max_index_);
        }
        else {
            hash_table_ = NULL;
            struct TagStoreEntry fpf_table_[max_index_];

            // zero out the buffer
            memset(fpf_table_, 0, sizeof(TagStoreEntry) * max_index_);

            DPRINTF(2, "librekv_cuckoo_create(): <result> byte size=%zu\n", sizeof(TagStoreEntry) * max_index_);
        }

        return 0;
    }

static int librekv_cuckoo_open()
{
	DPRINTF(2, "librekv_cuckoo_open()\n");
        if (hash_table_ || fpf_table_)
            librekv_cuckoo_close();
 
        if (librekv_cuckoo_create() == OK)
            if(0)//ReadFromFile() == 0)
                return OK;
 
        return -1;
}

static int librekv_cuckoo_flush()
{   
        DPRINTF(2, "librekv_cuckoo_flush()\n");
        if (0)//false)//WriteToFile())
            return 0;//ERROR;
        else
            return 1; 
} 

static int librekv_cuckoo_close()
{   
       if (!hash_table_ && !fpf_table_)
            return -1;

       librekv_cuckoo_flush();

       DPRINTF(2, "librekv_cuckoo_close()\n");
       if(hash_table_){
           free(hash_table_);
           DPRINTF(2, "librekv_cuckoo_close():HashTable deleted\n");
       }
       if(fpf_table_){
           free(fpf_table_);
           DPRINTF(2, "librekv_cuckoo_close():FpfTable deleted\n");
       }
       hash_table_ = NULL;
       fpf_table_  = NULL;
       return 0; 
}

static int  librekv_cuckoo_put(const uint8_t *key, const uint8_t *value)
{   
#ifdef DEBUG
        if (debug_level & 2) {
            DPRINTF(2, "librekv_cuckoo_put(): key=\n");
           // print_payload((const u_char*)key.data(), key.size(), 4); 
            DPRINTF(2, "librekv_cuckoo_put(): data=%llu\n", (long long unsigned)(value);
        }
#endif

        DPRINTF(2, "librekv_cuckoo_put(): key=\n");
        //print_payload((const u_char*)key.data(), key.size(), 4);
        DPRINTF(2, "librekv_cuckoo_put(): value=%llu\n", (long long unsigned)(value));
    
        // for undo correctness checking
        //uint32_t init_checksum = Hashes::h1(hash_table_, sizeof(TagValStoreEntry) * max_index_);

        uint32_t current_index, current_tag, current_val;
        uint32_t victim_way, victim_index, victim_tag, victim_val;

        uint32_t way;

        uint32_t undo_index[MAX_CUCKOO_COUNT];
        uint8_t undo_way[MAX_CUCKOO_COUNT];

        uint32_t fn = rand() % NUMHASH;
#ifdef SPLIT_HASHTABLE
        current_index = (keyfrag(key, fn) + fn * (1 << KEYFRAGBITS)) % max_index_;
#else
        current_index = keyfrag(key, fn) % max_index_;
#endif
        current_tag = keyfrag(key, (fn + 1) % NUMHASH) % max_index_;

        DPRINTF(2, "key: (index %d sig %d)\n", current_index, current_tag);

        current_val = (uint32_t *)(value);
        if ((way = freeslot(current_index)) < ASSOCIATIVITY) {
            /* an empty slot @ (index, way) of hash table */
            store(current_index, way, current_tag | VALIDBITMASK, current_val);
            current_entries_++;
            return OK;
	}

        fn = (fn + 1) % NUMHASH;
#ifdef SPLIT_HASHTABLE
        current_index = (keyfrag(key, fn) + fn * (1 << KEYFRAGBITS))  % max_index_;
#else
        current_index = keyfrag(key, fn)  % max_index_;
#endif
        current_tag = keyfrag(key, (fn + 1) % NUMHASH)  % max_index_;
	uint32_t n;
        for (n = 0; n < MAX_CUCKOO_COUNT; n++) {
            DPRINTF(2, "%d th try in loop!\n", n);
            DPRINTF(2, "key: (index %d sig %d)\n", current_index, current_tag);

            if ((way = freeslot(current_index)) < ASSOCIATIVITY) {
                /* an empty slot @ (current_index, current_way) of hash table */
                store(current_index, way, current_tag | VALIDBITMASK, current_val);
                current_entries_++;
                return OK;
            }

            victim_index = current_index;
            victim_way   = rand() % ASSOCIATIVITY;
            victim_tag   = tag(victim_index, victim_way);
            victim_val   = val(victim_index, victim_way);

            undo_index[n] = victim_index;
            undo_way[n] = victim_way;

            DPRINTF(2, "this bucket (%d) is full, take %d th as victim!\n", victim_index, victim_way);
            //DPRINTF(2, "victim: (index %d sig %d)\n", victim_index, victim_tag);
            store(victim_index, victim_way, current_tag | VALIDBITMASK, current_val);
#ifdef SPLIT_HASHTABLE
            current_index = (victim_tag + (1 << KEYFRAGBITS)) % max_index_;
            current_tag   = victim_index %  max_index_;
#else
            current_index = victim_tag  % max_index_;
            current_tag   = victim_index  % max_index_;
#endif
            current_val   = victim_val;
        }

DPRINTF(2, "librekv_cuckoo_put(): no more space to put new key -- undoing cuckooing\n");

        // undo cuckooing
        // restore the last state (find the previous victim_tag when calling last store())
#ifdef SPLIT_HASHTABLE
        current_tag   = (current_index + (1 << KEYFRAGBITS)) % max_index_;
#else
        current_tag   = current_index  % max_index_;
#endif
	uint32_t m;
        for (m = 0; m < MAX_CUCKOO_COUNT; m++) {
            victim_index  = undo_index[MAX_CUCKOO_COUNT - 1 - m];
            victim_way    = undo_way[MAX_CUCKOO_COUNT - 1 - m];
            victim_tag    = tag(victim_index, victim_way);
            victim_val    = val(victim_index, victim_way);

            DPRINTF(2, "restoring victim (index %d sig %d)\n", victim_index, current_tag);

            store(victim_index, victim_way, current_tag | VALIDBITMASK, current_val);

#ifdef SPLIT_HASHTABLE
            current_tag   = (victim_index + (1 << KEYFRAGBITS))  % max_index_;
#else
            current_tag   = victim_index  % max_index_;
#endif
            current_val   = victim_val;
        }

        assert(current_val == (uint32_t)(value));

        //uint32_t final_checksum = Hashes::h1(hash_table_, sizeof(TagValStoreEntry) * max_index_);
        //DPRINTF(2, "HashTableCuckoo::Put(): initial checksum=%u, final checksum=%u\n", init_checksum, final_checksum);

        DPRINTF(2, "librekv_cuckoo_put(): <result> undoing done\n");

        return INSUFFICIENT_SPACE;
    } // librekv_cuckoo_put()

/*  librekv_cuckoo::IteratorElem::Replace(const ConstValue& data)
    {   
#ifdef DEBUG
        if (debug_level & 2) {
            DPRINTF(2, "HashTableCuckoo::IteratorElem::Replace(): index %zu, data=%llu\n", current_index, static_cast<long long unsigned>(data.as_number<size_t>()));
        }
#endif
       // HashTableCuckoo* table = static_cast<HashTableCuckoo*>(const_cast<FawnDS*>(fawnds));

        uint32_t new_id = data.as_number<uint32_t>(-1);
        if (new_id == static_cast<uint32_t>(-1)) {
            DPRINTF(2, "HashTableCuckoo::IteratorElem::Replace(): could not parse data as ID\n");
            return INVALID_DATA;
        }

        DPRINTF(2, "HashTableCuckoo::IteratorElem::Replace(): <result> updated\n");
        table->hash_table_[current_index].val_vector[current_way] = new_id;
        return OK; 
    } // IteratorElem::Replace
*/
