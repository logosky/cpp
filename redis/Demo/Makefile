TARGET=HiredisDemo
LIB=-lhiredis
INCLUDE=
SRCFILE=$(wildcard *.cpp)

all:
	@g++ $(SRCFILE) $(INCLUDE) $(LIB) -o $(TARGET)
    
.PHONY:clean
clean:
	@rm -rf $(TARGET)
