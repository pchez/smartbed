CC=gcc
CFLAGS=-c -g -Wall 
LDFLAGS=-lmraa -lm -pthread -lfann
#SOURCES=collect_train_data.c 9DOF.c server.c LSM9DS0.c
SOURCESFALLRISK=updated_fallrisk.c queue.c 9DOF.c server.c LSM9DS0.c
#SOURCES=collect_train_data.c 9DOF.c server.c LSM9DS0.c
SOURCESNEW=train_neural_net.c 
OBJECTS=$(SOURCESFALLRISK:.c=.o)
OBJECTSNEW=$(SOURCES:.c=.o)
#EXECUTABLE=test_neural_net
#EXECUTABLE=collect_train_data
EXECUTABLEFALLRISK=updated_fallrisk
EXECUTABLENEW = train_neural_net

all: $(SOURCES) $(EXECUTABLE)

train: $(SOURCESNEW) $(EXECUTABLENEW) 

fallrisk: $(SOURCESFALLRISK) $(EXECUTABLEFALLRISK)

$(EXECUTABLENEW): $(OBJECTSNEW)
	$(CC) $(LDFLAGS) $(OBJECTSNEW) -o $@

$(EXECUTABLEFALLRISK): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

#$(EXECUTABLENEW): train_neural_net.c 
#	gcc -c -g -Wall -lfann train_neural_net.c -o train_neural_net
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(EXECUTABLENEW)
	rm -rf $(OBJECTS)
	#rm -rf TEST.net
	#rm -rf train_data.txt
	#rm -rf test_data.txt

clobber: clean
	rm -rf *~
