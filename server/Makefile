CC=gcc
CFLAGS=-c -g -Wall
LDFLAGS=-lmraa -lm -pthread -lfann
SOURCES=test_neural_net.c 9DOF.c server.c LSM9DS0.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test_neural_net

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(OBJECTS)
	rm -rf TEST.net
	rm -rf test_data.txt

clobber: clean
	rm -rf *~
