CC = gcc
CFLAGS = -pg -g -ggdb -Wall -O3
OBJS = color.o parallel_structures.o util.o hca.o tabucol.o
BIN = hca

all: $(OBJS) 
	$(CC) -o $(BIN) $(CFLAGS) $(OBJS) -lm -lrt -lpthread

clean:
	rm $(OBJS) color *~


