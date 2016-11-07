CC=gcc
CFLAGS=-c
LDFLAGS=-lmraa -lm -pthread
SOURCES=main.c 9DOF.c server.c LSM9DS0.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(OBJECTS)

clobber: clean
	rm -rf *~
