TARGET=Demo
LIB=-lmysqlclient -L/usr/lib64/mysql/ -lboost_system -ljson -lboost_thread
INCLUDE=
FLAGS=-D__STDC_FORMAT_MACROS
SRCFILE=$(wildcard *.cpp)

all:
	@g++ $(FLAGS) $(SRCFILE) $(INCLUDE) $(LIB) -g -o $(TARGET)
    
.PHONY:clean
clean:
	@rm -rf $(TARGET)
