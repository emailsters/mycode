CC = g++
FLAGS = -g -Wall 
INCLUDES = 
BIN = main

SOURCE = $(wildcard *.cpp)  
OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS) -lpthread

$(OBJS):%.o:%.cpp
	$(CC) -g -c $< $(INCLUDES) $(FLAGS)	

.PHONY:clean
clean:
	-rm -f $(OBJS) $(BIN)