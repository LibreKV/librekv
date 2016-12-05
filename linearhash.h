#ifndef _LINEAR_HASH_H_
#define _LINEAR_HASH_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"

#define SUCCESS  0
#define FAILURE -1
#define DEFAULT_INITIAL_LEVEL       1
#define DEFAULT_MAX_BUCKET_CAPACITY 4

//extern uint64_t twoToPowerOf(uint32_t power);
extern int twoToPowerOf(int power);
extern uint64_t leveledHash(uint64_t value, uint64_t depth);

/*typedef struct linearhash{
	bucket * bucketArray;
}__attribute__((__packed__));
*/

typedef struct linearHashEntry{
	uint64_t keyHash;
	uint64_t offset;
	bool isValid;
}__attribute__((__packed__)); 

typedef struct linearBucket
{
	struct linearHashEntry * m_linearHashEntry;
	struct linearBucket * next;
}__attribute__((__packed__));

struct linearBucket * m_linear_hashTable;

extern int librekv_linear_create(uint32_t i_initialLevel, uint64_t i_entriesPerBucket);
extern struct linearBucket * librekv_linear_initNewBucket(struct linearBucket * o_newBucket);
extern bool librekv_linear_insertToBucket(uint64_t keyHash, uint64_t offset, uint64_t bucketIndex);
void librekv_linear_displayHashTable(void);
void librekv_linear_displayBucketByIndex(uint64_t bucketIndex);
void librekv_linear_displayBucket(struct linearBucket * displayBucket);
//extern void librekv_linear_increaseHashTableLevel(uint32_t i_newHashTableLevel);
void librekv_lienar_compactBucket(uint64_t i_bucketIndex);
void librekv_linear_put(uint64_t keyHash, uint64_t offset);	


uint64_t m_currBucketNum;
uint64_t m_entriesPerBucket;
uint16_t m_currentLevel;
uint64_t m_currentMaxBucketsBeforeSplit;
uint64_t m_nextBucketIndexToSplit;
bool m_isOverflow;
bool m_isSplit;

#endif
