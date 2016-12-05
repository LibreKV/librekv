#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdlib.h>
#include "hash.h"
#include "linearhash.h"
#include "log.h"
#include "util.h"
int main(int argc, char **argv)
{ 
   uint32_t i_currentLevel = 1;
   uint64_t i_entriesPerBucket = 4;
   
   librekv_linear_create(i_currentLevel, i_entriesPerBucket);
   librekv_linear_displayHashTable(); 

   //librekv_linear_insertToBucket(10,100,2,false);
   uint64_t keyHash;
   uint64_t offset=10000;
   while (1){
	printf("Insert: ");
	scanf("%d",&keyHash);
	if (keyHash < 0) 
		break;
	librekv_linear_put2(keyHash,offset);
//	librekv_linear_put(keyHash,10000);
	librekv_linear_displayHashTable();
   }
   free(m_linear_hashTable);
   m_linear_hashTable = NULL;

/*
   char * key = "key1";
   char * value ="offset1";
   
   uint64_t keyHash = hash(key,sizeof(key));
   uint64_t offset = hash(value,sizeof(value));
   printf("keyhash is %llu\n",keyHash);
   printf("offset is %llu\n",offset);

   librekv_linear_put(1023,offset);
   printf("not arrival here\n"); 
   memset(log_buffer, 0, sizeof(log_buffer));
   sprintf(log_buffer, "linearhash create here.\n");
   librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));
*/
   return 0;
}
