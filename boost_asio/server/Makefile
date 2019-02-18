TARGET=DemoServer
LIB= -lboost_system -lboost_thread
INCLUDE= -I../comm
FLAGS=-D__STDC_FORMAT_MACROS
SRCFILE=$(wildcard *.cpp)
SRCFILE+=$(wildcard ../comm/*.cpp)

all:
	@g++ $(FLAGS) $(SRCFILE) $(INCLUDE) $(LIB) -g -o $(TARGET)
    
.PHONY:clean
clean:
	@rm -rf $(TARGET)
