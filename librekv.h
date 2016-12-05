#ifndef _LIBREKV_H_
#define _LIBREKV_H_

#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <stdint.h> 
#include <unistd.h> 

enum LibreKV_Return {
        OK = 0,
      //  ERROR,
        UNSUPPORTED,
        KEY_DELETED,
        KEY_NOT_FOUND,
        INVALID_KEY,
        INVALID_DATA,
        INVALID_LENGTH,
        INSUFFICIENT_SPACE,
        END,
};  

enum LibreKV_StatusType {
        NUM_DATA = 0,       // returns # of total data (including any dummy entries & holes)
        NUM_ACTIVE_DATA,    // returns # of total active (valid) data
        CAPACITY,           // returns # of total data the data store can possibly hold
        MEMORY_USE,         // returns memory byte size
        DISK_USE,           // returns disk byte size
};  

struct super_block {
        uint32_t magic;           /* Magic Number */
        uint64_t huge_page_count;   /* total # of segments */
        uint64_t init_total_size;       /* total # of Bytes */
        uint64_t dynamic_hash_start_addr; /*Dynamic hash start address */
        uint64_t page_count_dynamic_hash;  /*Dynamic hash page count*/
        uint64_t log_start_addr; /*Log area start address */
        uint64_t page_count_log;  /*Log area page count*/ 
        uint64_t data_start_addr; /*Data area start address */
        uint64_t page_count_data;  /*Data area page count*/
        uint16_t checksum;
} __attribute__ ((packed));



#endif
