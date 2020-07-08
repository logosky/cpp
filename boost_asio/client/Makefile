TARGET=DemoClient
LIB= -lboost_system -lboost_thread -lpthread
LIBPATH  = -L/home/common_lib/c++11/boost
INCLUDE= -I../comm -I/home/common_inc/c++11/boost
FLAGS=-D__STDC_FORMAT_MACROS
SRCFILE=$(wildcard *.cpp)
SRCFILE+=$(wildcard ../comm/*.cpp)

all:
	@g++ $(FLAGS) $(SRCFILE) $(INCLUDE) $(LIB) $(LIBPATH) -g -o $(TARGET)
    
.PHONY:clean
clean:
	@rm -rf $(TARGET)
