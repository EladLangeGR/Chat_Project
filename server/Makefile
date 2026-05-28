CC      = gcc
CFLAGS  = -Wall -Wextra -g -pthread
ROOT    = ..

INCLUDES = \
    -I. \
    -I$(ROOT)/protocol \
    -I$(ROOT)/gen_DS/gen_hash \
    -I$(ROOT)/gen_DS/gen_queue

SRCS = \
    server_main.c \
    server_net.c  \
    server_mng.c  \
    user_mng.c    \
    group_mng.c   \
    $(ROOT)/protocol/protocol.c          \
    $(ROOT)/gen_DS/gen_hash/gen_hash.c   \
    $(ROOT)/gen_DS/gen_queue/queue.c

TARGET = server

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: clean
