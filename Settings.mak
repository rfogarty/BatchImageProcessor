

# include directories
INCLUDES = -I. -I/usr/local/include $(shell pkg-config --cflags opencv4)
# pkg-config
LIBS = $(shell pkg-config --libs opencv4)

# compiler
CC = g++
#CC = clang++

# standard - note as of v0.3 C++03 is no longer supported
#            due to OpenCV4 reliance on C++11 or newer.
#CPP_STD = -std=c++03
CPP_STD = -std=c++11
#CPP_STD = -std=c++17

# optimization
#CPP_OPT_LEVEL = -g3 -ggdb -O0
CPP_OPT_LEVEL = -O2 -march=native
#CPP_OPT_LEVEL = -O2 -march=native -DNDEBUG

# warnings: always set to most Draconian...
CPP_WARNINGS = -Wall -Wpedantic -Werror

# enable AddressSanetizer
CPP_ADD_SAN =
#CPP_ADD_SAN = -fsanitize=address -fno-omit-frame-pointer

# error model
# NOTE: flags -DFAIL_WITH_ASSERT and -DNDEBUG are not appropriate for testImage unit test
#       and if provided will result in false positives. This is because the testImage
#       unit test performs various checks that assert that bad arguments are supplied
#       and would violate invariants within the Image or ImageView classes. For this reason
#       this value isn't written into CCFLAGS, but added directly to compile lines
#       where appropriate.
#
#CPP_ERROR_MODEL = -DFAIL_WITH_ASSERT
CPP_ERROR_MODEL = -DFAIL_WITH_EXCEPTION
#CPP_ERROR_MODEL = -DNDEBUG

# C++ compiler flags (e.g. -g -O2 -Wall)
CCFLAGS = $(CPP_STD) $(CPP_OPT_LEVEL) $(CPP_WARNINGS) $(CPP_ADD_SAN)

# linker flags
LDFLAGS = $(CPP_STD) $(CPP_OPT_LEVEL) $(CPP_ADD_SAN)

