#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include "cuckoo.h"
#include "hash.h"
#include "linearhash.h"
#include "log.h"


/*
int main(int argc, char **argv)
{ 
   uint32_t i_currentLevel = 1;
   uint64_t i_entriesPerBucket = 4;
 
   m_linear_hashTable = librekv_linear_create(i_currentLevel, i_entriesPerBucket);

   memset(log_buffer, 0, sizeof(log_buffer));
   sprintf(log_buffer, "linearhash create here.\n");
   librekv_logWrite(DEBUG,log_buffer,strlen(log_buffer));

   return 0;
}*/
