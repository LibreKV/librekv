#include<stdio.h>  
#include<unistd.h>  
#include<sys/mman.h>  
#include<sys/types.h>  
#include<sys/stat.h>  
#include<fcntl.h>  
      
int librekv_mmap(const char * filepath, size_t size, off_t offset)  
{  
        unsigned char * map_base;  
        //FILE *f;  
        int fd;  
 	size_t map_size = size;   
	off_t map_offset =offset;   
        fd = open(filepath, O_RDWR|O_SYNC);  
//	printf("%s\n",filepath);
//	printf("map_size is\n");
//	printf("0x%x\n",(unsigned int)map_size);
//	printf("map_offset starts from\n");
//	printf("0x%x\n",(unsigned int)map_offset);

        if (fd == -1)  
        {  
            return (-1);  
        }  
      
        map_base = mmap(NULL, map_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, map_offset);  
      
       if (map_base == 0)  
        {  
            printf("NULL pointer!\n");  
        }  
        else  
        {  
            printf("Successfull!\n");  
        }  
//	printf("0x%x\n",(unsigned int)map_base);      
        unsigned long addr;  
        unsigned char content;  
      
        int i = 0;  
        for (;i < map_size; ++i)  
        {  
            addr = (unsigned long)(map_base + i);  
	    content = map_base[i];  
	    if(i==0){
		printf("first address\n");
	    	printf("0x%ld\n",addr);
            	printf("0x%c\n",content);
	    }
	    if(i==map_size-1){
                printf("second address\n");
                printf("0x%ld\n",addr);
                printf("0x%c\n",content);
	    }

//	    printf("address: 0x%lx   content 0x%x\t\t", addr, (unsigned int)content);  
      
           // map_base[i] = (unsigned char)i;  
           // content = map_base[i];  
           // printf("updated address: 0x%lx   content 0x%x\n", addr, (unsigned int)content);  
        }  
   
        close(fd);  
      
        munmap(map_base, map_size);  
      
        return (1);  
}

/*int main(int argc, char **argv)
{
   char *filepath = "/dev/pmem0";
   size_t size = 0x80000000;
   off_t offset = 0x0;

   librekv_mmap(filepath,size,offset);
   
   return 0;
   //exit(0);
}*/
 
