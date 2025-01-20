CC = gcc
CFLAGS = -Wall -g

SRC = buffer_mgr.c storage_mgr.c dberror.c test_assign2_1.c buffer_mgr_stat.c

OBJ = $(SRC:.c=.o)

EXEC = buffer_manager

all: $(EXEC)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@


$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

clean:
	rm -f $(OBJ) $(EXEC)