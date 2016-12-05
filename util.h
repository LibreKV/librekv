#ifndef _UTIL_H_
#define _UTIL_H_

// Copyright 2016 Shanghai Jiao Tong University
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "common.h"

#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

typedef int bool;
#define true (1)
#define false (0)

#define LIBREKV_ROUNDUP8(x) (((x) + 7UL) & (~7UL))
#define LIBREKV_ROUNDUP64(x) (((x) + 63UL) & (~63UL))
#define LIBREKV_ROUNDUP4K(x) (((x) + 4095UL) & (~4095UL))
#define LIBREKV_ROUNDUP1M(x) (((x) + 1048575UL) & (~1048575UL))
#define LIBREKV_ROUNDUP2M(x) (((x) + 2097151UL) & (~2097151UL))

/*
void librekv_64dectobin(uint64_t n)  
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

void librekv_32dectobin(uint32_t n)
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
*/

//LIBREKV_BEGIN
/*
static
size_t
librekv_next_power_of_two(size_t v)
{
    size_t s = 0;
    while (((size_t)1 << s) < v)
        s++;
    return (size_t)1 << s;
}

static
void
memory_barrier()
{
    asm volatile("" ::: "memory");
}

static
void
librekv_memcpy8(uint8_t *dest, const uint8_t *src, size_t length)
{
    length = LIBREKV_ROUNDUP8(length);
#ifndef USE_DPDK
    switch (length >> 3)
    {
        case 0:
            break;
        case 1:
            *(uint64_t *)(dest + 0) = *(const uint64_t *)(src + 0);
            break;
        case 2:
            *(uint64_t *)(dest + 0) = *(const uint64_t *)(src + 0);
            *(uint64_t *)(dest + 8) = *(const uint64_t *)(src + 8);
            break;
        case 3:
            *(uint64_t *)(dest + 0) = *(const uint64_t *)(src + 0);
            *(uint64_t *)(dest + 8) = *(const uint64_t *)(src + 8);
            *(uint64_t *)(dest + 16) = *(const uint64_t *)(src + 16);
            break;
        case 4:
            *(uint64_t *)(dest + 0) = *(const uint64_t *)(src + 0);
            *(uint64_t *)(dest + 8) = *(const uint64_t *)(src + 8);
            *(uint64_t *)(dest + 16) = *(const uint64_t *)(src + 16);
            *(uint64_t *)(dest + 24) = *(const uint64_t *)(src + 24);
            break;
        default:
            memcpy(dest, src, length);
            break;
    }
#else
    rte_memcpy(dest, src, length);
#endif
}

static
bool
librekv_memcmp8(const uint8_t *dest, const uint8_t *src, size_t length)
{
    length = LIBREKV_ROUNDUP8(length);
    switch (length >> 3)
    {
        case 0:
            return true;
        case 1:
            if (*(const uint64_t *)(dest + 0) != *(const uint64_t *)(src + 0))
                return false;
            return true;
        case 2:
            if (*(const uint64_t *)(dest + 0) != *(const uint64_t *)(src + 0))
                return false;
            if (*(const uint64_t *)(dest + 8) != *(const uint64_t *)(src + 8))
                return false;
            return true;
        case 3:
            if (*(const uint64_t *)(dest + 0) != *(const uint64_t *)(src + 0))
                return false;
            if (*(const uint64_t *)(dest + 8) != *(const uint64_t *)(src + 8))
                return false;
            if (*(const uint64_t *)(dest + 16) != *(const uint64_t *)(src + 16))
                return false;
            return true;
        case 4:
            if (*(const uint64_t *)(dest + 0) != *(const uint64_t *)(src + 0))
                return false;
            if (*(const uint64_t *)(dest + 8) != *(const uint64_t *)(src + 8))
                return false;
            if (*(const uint64_t *)(dest + 16) != *(const uint64_t *)(src + 16))
                return false;
            if (*(const uint64_t *)(dest + 24) != *(const uint64_t *)(src + 24))
                return false;
            return true;
        default:
            return memcmp(dest, src, length) == 0;
    }
}

static
uint32_t
librekv_rand(uint64_t *state)
{
    // same as Java's
    *state = (*state * 0x5deece66dUL + 0xbUL) & ((1UL << 48) - 1);
    return (uint32_t)(*state >> (48 - 32));
}

static
double
librekv_rand_d(uint64_t *state)
{
    // caution: this is maybe too non-random
    *state = (*state * 0x5deece66dUL + 0xbUL) & ((1UL << 48) - 1);
    return (double)*state / (double)((1UL << 48) - 1);
}

static
size_t
librekv_get_memuse()
{
    struct rusage usage;
    int ret = getrusage(RUSAGE_SELF, &usage);
    assert(ret == 0);
    (void)ret;
    return (size_t)usage.ru_maxrss * 1024 + librekv_shm_get_memuse();
}
*/

//LIBREKV_END
#endif
