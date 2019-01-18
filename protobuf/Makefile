GCC = g++
INC = -I.
LIB = -L/usr/local/commlib/outerlib/protobuf/lib/ -lprotobuf

SRC = test.cpp student.pb.cc
    
test:
	$(GCC) $(SRC) $(INC) $(LIB) -o test

.PHONY:clean
clean:
	-rm -rf test *.o
