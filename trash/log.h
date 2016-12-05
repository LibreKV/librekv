#ifndef _LOG_H_
#define _LOG_H_

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG_FILE_MAX_SIZE (2*1024*1024)

void get_local_time(char* buffer);

long get_file_size(char* filename);

void librekv_log(char* filename,long max_size, char* buffer, unsigned buf_size);

#endif
