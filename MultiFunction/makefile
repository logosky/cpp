

CFLAGS += -g -w -O2 -D_GNU_SOURCE 

PATH_BIN = .

INC += -I./

SRC += $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRC))

TARGET = test 

all : $(TARGET)

$(TARGET):$(OBJS)
	$(CXX) -o $(PATH_BIN)/$@ $^ $(LIB) $(LFLAGS)
	
%.o: %.cpp
	@printf "\033[35m$*\033[0m : make start ...\n" ;
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC)
	@printf "\033[32m%-30s[done]\033[0m\n" $*;
	@printf "\033[32m[done]\033[0m\n";

	
.PHONY:clean
clean:
	-rm -f $(OBJS) $(PATH_BIN)/$(TARGET)
	
