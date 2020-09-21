
BIN_DIR=bin/

EXEC = iptool
TEST_EXEC = testImage

# compiler
CCC = g++
#CCC = clang++

# C++ compiler flags (e.g. -g -O2 -Wall)
# 
# NOTE: flags -DFAIL_WITH_ASSERT and -DNDEBUG are not appropriate for testImage unit test
#       and if provided will result in false positives. This is because the testImage
#       unit test performs various checks that assert that bad arguments are supplied
#       and would violate invariants within the Image or ImageView classes.
#
#CCFLAGS = -std=c++03 -g3 -ggdb -O0 -DFAIL_WITH_ASSERT -Wall -Wpedantic -Werror
CCFLAGS = -std=c++03 -g3 -ggdb -O0 -Wall -Wpedantic -Werror
#CCFLAGS = -std=c++03 -O2 -march=native -Wall -Wpedantic -Werror
# For ultimate speed disable any sort of invariant checking
#CCFLAGS = -std=c++03 -O2 -march=native -DNDEBUG -Wall -Wpedantic -Werror
#CCFLAGS = -std=c++17 -O2 -march=native -DNDEBUG -Wall -Wpedantic -Werror

all: $(EXEC) $(TEST_EXEC)

iptools:
	(cd ../iptools; make)

clean:
	$(RM) $(BIN_DIR)$(EXEC)
	$(RM) $(BIN_DIR)$(TEST_EXEC)
	(cd ../iptools; make clean)

DO_EXEC = $(CCC) $(CCFLAGS) -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools

DO_TEST = $(CCC) $(CCFLAGS) -I../iptools -o $(BIN_DIR)$@ $@.cpp -L ../lib -l iptools

#------ now compile
$(EXEC):   		iptools
				$(DO_EXEC)

$(TEST_EXEC):  iptools
				$(DO_TEST)
