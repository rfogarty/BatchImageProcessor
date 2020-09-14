#all:
	#cd ../iptools; make
#	g++ -o bin/test test.cpp  -L ../lib -l iptools 
#	g++ -o bin/test2 test2.cpp  -L ../lib -l iptools -l mpeg 

	# source files.

BIN_DIR=bin/

EXEC = iptool
TEST_EXEC = testImage


all: $(EXEC) $(TEST_EXEC)

iptools:
	(cd ../iptools; make)

clean:
	$(RM) $(BIN_DIR)$(EXEC)
	(cd ../iptools; make clean)

#DO_EXEC = g++ -Wall -Wpedantic -Werror -std=c++03 -g3 -O0 -ggdb -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools
DO_EXEC = g++ -Wall -Wpedantic -Werror -std=c++03 -O2 -march=native -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools
#DO_EXEC = clang++ -Wall -Wpedantic -Werror -std=c++03 -O2 -march=native -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools
#DO_EXEC = g++ -Wall -Wpedantic -Werror -std=c++17 -O2 -march=native -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools

DO_TEST = g++ -Wall -Wpedantic -Werror -std=c++03 -O2 -march=native -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools

#------ now compile
$(EXEC):   		iptools
				$(DO_EXEC)

$(TEST_EXEC):  iptools
				$(DO_TEST)
