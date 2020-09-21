# source files.
SRC =  utility/StringParse.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../lib/libiptools.a

# include directories
INCLUDES = -I. -I/usr/local/include

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

# compiler
CCC = g++
#CCC = clang++


# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	@mkdir -p ../lib
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)
