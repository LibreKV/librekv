#include "linearhash.h"
#include "cuckoo.h"
#include "log.h"
#include "util.h"

char log_buffer[100];

/*
returns 2^power
*/
extern int twoToPowerOf(int power)
{
        int result = 1;
        for (power--; power >= 0; power--)
        {
                result <<= 1;
        }
        return result;
}
extern void dectobin(uint64_t n)
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
}
/*
Determines the linear hash key of a given value using a given depth.
Returns the "depth" least-significant bits.
Ex. 
If value is 00110101 in binary, and depth is 1, hash will return 1 (the least significant bit).
If the depth is 4, hash will return 0101=3, the 4 least significant bits.
*/
extern uint64_t truncateHash(uint64_t key, uint16_t level) {

        //assert(key <= sizeof(uint64_t));
        uint64_t keyfragmask = (1 << level) - 1;
        uint64_t tmp = (key);
        tmp = (tmp) & keyfragmask;
 
        return tmp;
}


/*
Note: Initial size of the hash table is 2^level
*/
extern int librekv_linear_create(uint32_t i_initialLevel, uint64_t i_entriesPerBucket)
{
	if (i_initialLevel >= 0)
		m_currentLevel = i_initialLevel;
	else
		m_currentLevel = DEFAULT_INITIAL_LEVEL;

	if (i_entriesPerBucket > 0)
		m_entriesPerBucket = i_entriesPerBucket;
	else
		m_entriesPerBucket = DEFAULT_MAX_BUCKET_CAPACITY;
	
	m_currentMaxBucketsBeforeSplit = twoToPowerOf(m_currentLevel);
	m_currBucketNum = twoToPowerOf(m_currentLevel);
	printf("m_currentMaxBucketBeforeSplit is %16d\n",m_currentMaxBucketsBeforeSplit);
	// Allocate hash table
	m_linear_hashTable = (struct linearBucket * )malloc(m_currentMaxBucketsBeforeSplit * sizeof(struct linearBucket));
	// Allocate buckets of hash table
	for (int i = 0; i < m_currentMaxBucketsBeforeSplit; i++){
		librekv_linear_initNewBucket(&(m_linear_hashTable[i]));
	}
	m_nextBucketIndexToSplit = 0;
	
	return 0;
}

// this function is tested well.
struct linearBucket * librekv_linear_initNewBucket(struct linearBucket * o_newBucket)
{
        o_newBucket->m_linearHashEntry  = (struct linearHashEntry *)malloc(m_entriesPerBucket * sizeof(struct linearHashEntry));
        o_newBucket->next = NULL;
        for (int i = 0; i < m_entriesPerBucket; i++){
                o_newBucket->m_linearHashEntry[i].keyHash = -1;
                o_newBucket->m_linearHashEntry[i].offset = -10;
                o_newBucket->m_linearHashEntry[i].isValid = true;
        }
        return o_newBucket;
}

// This function is tested well.
void librekv_linear_displayHashTable(void)
{
	printf("Level = %i\n", m_currentLevel);
	struct linearBucket * currentBucket;
	for (int i = 0; i < m_currBucketNum;i++)//mm_nextBucketIndexToSplit + twoToPowerOf(m_currentLevel); i++)
	{
		//if (i == twoToPowerOf(m_currentLevel)){
		if (i % 2 == 0){
			printf("---------------------------------\n");
		}
		printf("Current bucket is %d\n", i);
		currentBucket = &(m_linear_hashTable[i]);
		do {
			for (int j = 0; j < m_entriesPerBucket; j++) {
				printf("hash entry %d keyhash is %d", j, currentBucket->m_linearHashEntry[j].keyHash);
				printf(", ");
				printf("offset is ");dectobin(currentBucket->m_linearHashEntry[j].keyHash);
				printf(", ");
				printf("isValid is %d\n", currentBucket->m_linearHashEntry[j].isValid);
			}
			currentBucket = currentBucket->next;
		} while(currentBucket);
		printf( "\n" );
	}
}


// This function is tested well.
void librekv_linear_displayBucketByIndex(uint64_t bucketIndex)
{
	struct linearBucket * currentBucket;
	printf("Current bucket is %d\n", bucketIndex);
	currentBucket = &(m_linear_hashTable[bucketIndex]);
	if(currentBucket){
		for (int j = 0; j < m_entriesPerBucket; j++) {
			printf("hash entry %d keyhash is %d", j, currentBucket->m_linearHashEntry[j].keyHash);
			printf(", ");
			printf("offset is %d", currentBucket->m_linearHashEntry[j].offset);
			printf(", ");
			printf("isValid is %d\n", currentBucket->m_linearHashEntry[j].isValid);
		}
		currentBucket = currentBucket->next;
	}
	printf( "\n" );
}

// This function is tested well.
void librekv_linear_displayBucket(struct linearBucket * displayBucket)
{
	if(displayBucket){
		for (int j = 0; j < m_entriesPerBucket; j++) {
			printf("hash entry %d keyhash is %d", j, displayBucket->m_linearHashEntry[j].keyHash);
			printf(", ");
			printf("offset is %d", displayBucket->m_linearHashEntry[j].offset);
			printf(", ");
			printf("isValid is %d\n", displayBucket->m_linearHashEntry[j].isValid);
		}
		displayBucket = displayBucket->next;
	}
	printf( "\n" );
}

//lesson here : before initilization to a variable must need to malloc memory for the variable!!! 2016/10/30
extern bool librekv_linear_insertToBucket(uint64_t keyHash, uint64_t offset, uint64_t bucketIndex){
	
	struct linearBucket * currBucket;
	struct linearBucket * overflowBucket;
	currBucket = &(m_linear_hashTable[bucketIndex]);
	struct linearBucket * prevBucket = NULL;
	m_isSplit = false;
	m_isOverflow = false;	
	// Attempt to insert to target bucket's pages
	do{
		for (int i = 0; i < m_entriesPerBucket; i++)
		{
			printf("Bucket %d hash entry %d is %d\n", bucketIndex, i, currBucket->m_linearHashEntry[i].isValid);
			if (currBucket->m_linearHashEntry[i].isValid)
			{
				//printf("insert into %d entry.\n",i);
				currBucket->m_linearHashEntry[i].keyHash = keyHash;
				currBucket->m_linearHashEntry[i].offset = offset;
				currBucket->m_linearHashEntry[i].isValid = false;
				return m_isSplit;
			}
		}
		prevBucket = currBucket;
		currBucket = currBucket->next;
	} while (currBucket);
	//printf("bucket is full.\n");
	//printf("m_nextBucketIndexToSplit is %d\n",m_nextBucketIndexToSplit);
	if(m_nextBucketIndexToSplit != bucketIndex){
		printf("overflow bucket.\n");
		// Bucket is full; allocate and initialize overflow page, insert value into it, and append to bucket
		overflowBucket = (struct linearBucket *)malloc(sizeof(struct linearBucket));
		overflowBucket = librekv_linear_initNewBucket(overflowBucket);
		overflowBucket->m_linearHashEntry[0].keyHash = keyHash;
		overflowBucket->m_linearHashEntry[0].offset = offset;
		overflowBucket->m_linearHashEntry[0].isValid = false;
		prevBucket->next = overflowBucket;
		m_isSplit=true;
		//m_isOverflow = true;
		return m_isSplit;
	}
	//m_nextBucketIndexToSplit++;	
	// Indicate to caller that isSplit is necessary
	m_isSplit = true;
	return m_isSplit;
}

extern struct linearBucket * librekv_linear_addBucket(){

	printf("bucket num is %d\n", m_currBucketNum);
	struct linearBucket * m_lastBucketBeforeSplit = &(m_linear_hashTable[m_currBucketNum]);
	//allocate and initialize a new bucket.
	struct linearBucket * newBucket = (struct linearBucket *)malloc(sizeof(struct linearBucket));
	newBucket = librekv_linear_initNewBucket(newBucket);
	m_currBucketNum++;
	m_linear_hashTable = (struct linearBucket *)realloc(m_linear_hashTable, m_currBucketNum * sizeof(struct linearBucket));
	m_linear_hashTable[m_currBucketNum-1] = *(newBucket);
	return newBucket;
}

/*
void librekv_linear_increaseHashTableLevel(uint32_t i_newHashTableLevel)
{
	if (i_newHashTableLevel == m_currentLevel)
		return;
	int newMaxBucketsBeforeSplit = twoToPowerOf(i_newHashTableLevel + 1);
	// Allocate new hash table
	struct linearBucket * m_newHashTable = (struct linearBucket *)malloc(newMaxBucketsBeforeSplit * sizeof(struct linearBucket));
	// Copy old bucket pointers into new hash table
	for (int i = 0 ; i < m_currentMaxBucketsBeforeSplit; i++){
		m_newHashTable[i] = m_linear_hashTable[i];
	}
	// Allocate additional buckets of new hash table
	for (int i = m_currentMaxBucketsBeforeSplit; i < newMaxBucketsBeforeSplit; i++){
		librekv_linear_initNewBucket(&(m_newHashTable[i]));
	}
	// De-allocate old hash table
	free(m_linear_hashTable);
	// Update member vars
	m_linear_hashTable = m_newHashTable;
	m_currentLevel = i_newHashTableLevel;
	m_currentMaxBucketsBeforeSplit = twoToPowerOf(m_currentLevel + 1);

}
*/
void librekv_linear_compactBucket(uint64_t i_bucketIndex)
{
	struct linearBucket * currBucket = &(m_linear_hashTable[i_bucketIndex]);
	// Allocate and initialize new bucket (will be the compacted bucket)
	struct linearBucket * compactedBucket;
	compactedBucket = librekv_linear_initNewBucket(compactedBucket);
	printf("successfully initilizied the compacted bucket.\n");
	struct linearBucket * nextBucketToInsertTo = compactedBucket;
	int nextIndexToInsertTo = 0;
	printf("into the compacted bucket.\n");
	// Loop through non-compact bucket and copy elements into new bucket
	do
	{
		for (int i = 0; i < 5; i++)//m_entriesPerBucket; i++)
		{
			if (currBucket->m_linearHashEntry[i].isValid)
			{
				// If next index points beyond page capacity, allocate and initialize a new overflow page
				printf("into the for compacted bucket.\n");
				if (nextIndexToInsertTo >= m_entriesPerBucket)
				{
					nextBucketToInsertTo->next = (struct linearBucket *)malloc(sizeof(struct linearBucket));
					nextBucketToInsertTo = nextBucketToInsertTo->next;
					nextBucketToInsertTo = librekv_linear_initNewBucket(nextBucketToInsertTo);
					nextIndexToInsertTo = 0;
				}
				nextBucketToInsertTo->m_linearHashEntry[nextIndexToInsertTo] = currBucket->m_linearHashEntry[i];
				nextIndexToInsertTo++;
			}
		}
		struct linearBucket * tempBucket = currBucket;
		currBucket = currBucket->next;
		// De-allocate non-compact bucket's page
		free(tempBucket->m_linearHashEntry);
	} 
	while (currBucket);
	m_linear_hashTable[i_bucketIndex] = *(compactedBucket);

}

extern librekv_linear_transferToMirrorBucket(uint64_t keyHash, uint64_t offset, uint64_t bucketIndex){
  

}
void librekv_linear_put2(uint64_t keyHash, uint64_t offset){
	m_isSplit = false;
	uint64_t truncatedKeyHash;
	for(int i = m_currentLevel; i>0; i--) {
		truncatedKeyHash = truncateHash(keyHash, m_currentLevel);
		if(keyHash == 18) {
			printf("m_nextBucketIndexToSplit is %d\n\n\n",m_nextBucketIndexToSplit);
			printf("m_currentLevel is %d\n\n\n",m_currentLevel);
			printf("truncatedkeyHash is %d\n\n\n",truncatedKeyHash);
			//printf("m_isSplit is %d\n",m_isSplit);
		}
		m_isSplit = librekv_linear_insertToBucket(keyHash, offset, truncatedKeyHash);
		//printf("truncatedKeyHash is %d\n",truncatedKeyHash);
		printf("m_isSplit is %d\n",m_isSplit);
		//if(m_isSplit){
		break;
		//}
	}
	if(m_isSplit){
		struct linearBucket * currBucket = &(m_linear_hashTable[m_nextBucketIndexToSplit]);
		struct linearBucket * splitBucket;
		//uint64_t newMirrorImageBucketIndex = m_nextBucketIndexToSplit + twoToPowerOf(m_currentLevel);
		//printf("newMirrorImageBucketIndex is %d\n",newMirrorImageBucketIndex);
		//while(m_currBucketNum <= newMirrorImageBucketIndex){
	        printf("before add bucket, num is %d\n\n\n", m_currBucketNum);
		splitBucket = librekv_linear_addBucket();
	        printf("after add bucket, num is %d\n\n\n", m_currBucketNum);
		m_linear_hashTable[m_currBucketNum-1] = *(splitBucket);
		//printf("here 1, m_nextBucketIndexToSplit is %d\n",m_nextBucketIndexToSplit);
		for (int i = 0; i <= m_entriesPerBucket; i++)
		{
			//librekv_linear_displayBucket(currBucket);
			if (!m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].isValid)
			{
				// If value belongs in mirror image bucket, move it there, otherwise don't move it
				uint64_t nextLevelKey = truncateHash(m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].keyHash, m_currentLevel+1);
				if(m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].keyHash == 4 || m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].keyHash == 12 )
			{				
				printf("Next level key is %d\n\n\n\n",nextLevelKey);	
				printf("m_currentLevel is %d\n\n\n\n",m_currentLevel);	
			}
				//printf("now the next Level key is %d\n",nextLevelKey);	
				if (nextLevelKey == m_currBucketNum-1)
				{
					m_isSplit = librekv_linear_insertToBucket(m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].keyHash, m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].offset, m_currBucketNum-1);					m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].keyHash = -1;
					m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].offset = -10;
					m_linear_hashTable[m_nextBucketIndexToSplit].m_linearHashEntry[i].isValid = true;
					//if(m_linear_hashTable[m_nextBucketIndexToSplit]->next != NULL){
						//free(m_linear_hashTable[m_nextBucketIndexToSplit]->next);// = NULL;
					//}
				}
			}
		}
		m_nextBucketIndexToSplit++;
               	m_isSplit = librekv_linear_insertToBucket(keyHash, offset, truncatedKeyHash);
		//m_nextBucketIndexToSplit++;
		if(m_currBucketNum = twoToPowerOf(m_currentLevel)){
                        m_currentLevel++; 
			m_nextBucketIndexToSplit=0;
		}
		//m_nextBucketIndexToSplit++;
		m_isSplit = false;
		//else{
		//	 printf("yiding dao zhe li.\n\n\n\n\n\n");
                //         m_nextBucketIndexToSplit++;
                //}
		librekv_linear_displayHashTable();
	}
}

void librekv_linear_put(uint64_t keyHash, uint64_t offset){
	// Whether or not the insert requires a isSplit
	bool isSplit = false;
	// Determine hash key of value for current level
	uint64_t truncatedKeyHash = truncateHash(keyHash, m_currentLevel);
	printf("Attempting to insert compelete keyHash %llu into bucket %lu\n", keyHash, truncatedKeyHash);
	printf("m_nextBucketIndexToSpilt is %d\n",m_nextBucketIndexToSplit);
	printf("twoToPowerOf m_currentLevel is %d\n",twoToPowerOf(m_currentLevel));
	printf("truncatedKeyHash is %d\n",truncatedKeyHash);
	// If hash key is in [nextBucketIndexToSplit,currentLevelRange), then value goes into this bucket
	if(truncatedKeyHash >= m_nextBucketIndexToSplit && truncatedKeyHash < twoToPowerOf(m_currentLevel)){
		isSplit = librekv_linear_insertToBucket(keyHash, offset, truncatedKeyHash);
		printf("first insert.\n");
	}
	else{
		// Hash key for current level falls into [0,nextBucketIndexToSplit)
		// See if value should go into isSplit image by calculating next level's hash key
		uint64_t nextLevelKey = truncateHash(keyHash, m_currentLevel + 1);
		printf( "Attempting to insert compelete keyHash %llu into bucket %lu\n", keyHash, nextLevelKey);
		// If next level's hash key > last isSplit image, then value does not go into isSplit image (since isSplit image does not exist)
		// Otherwise, value goes into isSplit image
		if (nextLevelKey > (m_nextBucketIndexToSplit - 1) + twoToPowerOf(m_currentLevel)){
			isSplit = librekv_linear_insertToBucket(keyHash, offset, truncatedKeyHash);
			printf("second insert.\n");
		}
		else{
			isSplit = librekv_linear_insertToBucket(keyHash, offset, nextLevelKey);
			printf("third insert.\n");
		}
	}
	if(isSplit){
		printf("split is true.\n");
		// Split next bucket and re-distribute values in both buckets
		uint64_t newMirrorImageBucketIndex = m_nextBucketIndexToSplit + twoToPowerOf(m_currentLevel);
		printf("newMirrorImageBucketIndex is %d\n",newMirrorImageBucketIndex);
		struct linearBucket * currBucket = &(m_linear_hashTable[m_nextBucketIndexToSplit]);
		printf("here 1, m_nextBucketIndexToSplit is %d\n",m_nextBucketIndexToSplit);
		do
		{
			for (int i = 0; i < m_entriesPerBucket; i++)
			{
				if (!currBucket->m_linearHashEntry[i].isValid)
				{
					printf("here7 into the for.\n");
					// If value belongs in mirror image bucket, move it there, otherwise don't move it
					uint64_t nextLevelKey = truncateHash(currBucket->m_linearHashEntry[i].keyHash, m_currentLevel + 1 );
					if (nextLevelKey == newMirrorImageBucketIndex)
					{
						isSplit = librekv_linear_insertToBucket(currBucket->m_linearHashEntry[i].keyHash, currBucket->m_linearHashEntry[i].offset, newMirrorImageBucketIndex);
						currBucket->m_linearHashEntry[i].keyHash = -1;
						currBucket->m_linearHashEntry[i].offset = -10;
						currBucket->m_linearHashEntry[i].isValid = true;
					}
				}
			}
			currBucket = currBucket->next;
		} 
		while (currBucket);
		printf("compacted bucket.\n");
		// Compact the original bucket page that was split
		//librekv_linear_compactBucket(m_nextBucketIndexToSplit);
		printf( "Split occured! Bucket %i split to %i\n", m_nextBucketIndexToSplit, newMirrorImageBucketIndex);
		// Increment next bucket index to split
		m_nextBucketIndexToSplit++;
		// If next bucket index to split is outside of current level range, 
		// increment level and set next bucket index to split back to 0
		if (m_nextBucketIndexToSplit >= twoToPowerOf(m_currentLevel))
		{
		//	librekv_linear_increaseHashTableLevel(m_currentLevel + 1);
			m_nextBucketIndexToSplit = 0;
			printf( "Level increased from %i to %i\n", m_currentLevel - 1, m_currentLevel);
		}
		printf( "Next bucket to split will be %i\n", m_nextBucketIndexToSplit);
	}
}
