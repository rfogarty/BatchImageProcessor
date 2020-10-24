
include ../Settings.mak

# source files.
SRC =  utility/StringParse.cpp

OBJ = $(SRC:.cpp=.o)

OUT = ../lib/libbatchIPTools.a


.SUFFIXES: .cpp

.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	@mkdir -p ../lib
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)
