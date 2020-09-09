# source files.
SRC =  utility/StringParse.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../lib/libiptools.a

# include directories
INCLUDES = -I. -I/usr/local/include

# C++ compiler flags (-g -O2 -Wall)
#CCFLAGS = -std=c++03 -g3 -ggdb -O0
CCFLAGS = -std=c++03 -O2 -march=native

# compiler
CCC = g++


# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak# DO NOT DELETE
