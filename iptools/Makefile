# source files.
SRC =  utility/StringParse.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../lib/libiptools.a

# include directories
INCLUDES = -I. -I/usr/local/include

# C++ compiler flags (-g -O2 -Wall)
#CCFLAGS = -std=c++03 -g3 -ggdb -O0 -DFAIL_WITH_ASSERT -Wall -Wpedantic -Werror 
#CCFLAGS = -std=c++03 -g3 -ggdb -O0 -Wall -Wpedantic -Werror 
CCFLAGS = -std=c++03 -O2 -march=native -Wall -Wpedantic -Werror 
#CCFLAGS = -std=c++17 -O2 -march=native -Wall -Wpedantic -Werror 

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
