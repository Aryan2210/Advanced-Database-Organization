CC = gcc
CFLAGS = -Wall -Wextra -g

SOURCES = storage_mgr.c buffer_mgr.c buffer_mgr_stat.c record_mgr.c expr.c rm_serializer.c dberror.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: test_assign3 test_expr

test_assign3: test_assign3_1.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

test_expr: test_expr.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test_assign3 test_expr $(OBJECTS)