CXX = g++

SRC += $(wildcard ./src/*.cpp)
INC += -I/data/kavenliu/file/world/inc
LIB += 

CFLAGS += -g -w -DTEST -m64 -DSUS_LINUX -O2
CFLAGS += -MMD -MP
#LFLAGS += -Wl,-rpath=./ -shared -ldl -lz -pthread

TARGET = world
ALL : $(TARGET)

OBJS = $(patsubst %.cpp,%.o,$(SRC))
DEPS = $(SRC:.cpp=.d)

$(TARGET):$(OBJS)
	@printf "target:$(TARGET)\nobjs: $(OBJS)"
	$(CXX) -o $@ $^ $(LIB)
    
-include $(DEPS)

%.o: %.cpp
	@printf "make:$@\n"
	$(CXX) -g -c -o $@ $< $(INC) $(CFLAGS)

    
.PHONEY:clean
clean:
	-rm -rf $(TARGET) $(OBJS) $(DEPS)
