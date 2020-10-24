
include ../Settings.mak

INCLUDES += -I../batchIP

LIBS += -L ../lib -l batchIPTools

BIN_DIR=bin/

EXEC = batchIP
TEST_EXEC = testImage


all: $(EXEC) $(TEST_EXEC)

libbatchIPTools:
	(cd ../batchIP; make)

clean:
	$(RM) $(BIN_DIR)$(EXEC)
	$(RM) $(BIN_DIR)$(TEST_EXEC)
	(cd ../batchIP; make clean)

DO_EXEC = $(CC) $(CCFLAGS) $(CPP_ERROR_MODEL) $(INCLUDES) -o $(BIN_DIR)$@ $@.cpp $(LIBS)

DO_TEST = $(CC) $(CCFLAGS) $(INCLUDES) -o $(BIN_DIR)$@ $@.cpp $(LIBS)

#------ now compile
$(EXEC):   		libbatchIPTools
				$(DO_EXEC)

$(TEST_EXEC):  libbatchIPTools
				$(DO_TEST)
