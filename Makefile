OBJS_DIR = build/
TARGET = librekv
CC = gcc

SRCS = super.c nvmmap.c util.c cuckoo.c city.c cuckoo_test.c hash.c log.c linearhash.c linearhash_test.c
	
CFLAGS = -g -std=c11 -Wall
#LDFLAGS = -lpthread
#LIBS =  -lpthread 
OBJS = $(addprefix $(OBJS_DIR), $(SRCS:.c=.o))

all:
	@mkdir -p $(OBJS_DIR)
	@make $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(OBJS_DIR)$@ $^

$(OBJS_DIR)%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	-rm -rf $(OBJS_DIR)
