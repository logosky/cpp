TARGET=HiredisDemo
LIB=-lhiredis -lboost_system -ljson -lboost_thread
INCLUDE=
SRCFILE=$(wildcard *.cpp)

all:
	@g++ $(SRCFILE) $(INCLUDE) $(LIB) -o $(TARGET)
    
.PHONY:clean
clean:
	@rm -rf $(TARGET)
