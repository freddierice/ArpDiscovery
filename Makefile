# general configurations
CC=gcc
LD=$(CC)
CFLAGS=-Wall -g
LDFLAGS=-lpcap
TARGET=arpfind

# directory structure
SRC_DIR=src
BIN_DIR=bin

# generate files list
FILES=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst %.c,%.o,$(FILES))
DIRS=$(SRC_DIR) $(BIN_DIR)
all: $(DIRS) $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJS) 
	$(LD) -o $@ $^ $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(DIRS):
	mkdir -p $@

clean: 
	rm -r $(BIN_DIR) $(OBJS)
