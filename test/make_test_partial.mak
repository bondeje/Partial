UNAME := $(shell uname)
CC = gcc

EXT = 

LFLAGS = -L./ 
CFLAGS = -std=c99 -O2 -Wall -pedantic
IFLAGS = -I../include
CCFLAGS = #-D DEVELOPMENT # -D DEVELOPMENT turns on internal print statements for diagnostics

ifeq ($(OS),Windows_NT)
	# might have to encapsulate with a check for MINGW. Need this because Windows f-s up printf with size_t and MINGW only handles it with their own implementation of stdio
	CFLAGS += -D__USE_MINGW_ANSI_STDIO
    IFLAGS += -I../ext/x86_64-w64-mingw64/
    LFLAGS += -L../ext/x86_64-w64-mingw64/ -llibffi-8
	EXT = .exe
    CCFLAGS += -D WIN32
    #ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    #    CCFLAGS += -D AMD64
    #else
    #    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    #        CCFLAGS += -D AMD64
    #    endif
    #    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
    #        CCFLAGS += -D IA32
    #    endif
    #endif
else
    UNAME_S := $(shell uname -s)
	# for dynamic memory allocation extensions in posix, e.g. getline()
	#CFLAGS += -D__STDC_WANT_LIB_EXT2__=1
	# really cool, -g creates symbols so that valgrind will actually show you the lines of errors
    CFLAGS += -g
    ifeq ($(UNAME_S),Linux)
		# needed because linux must link to the math
		LFLAGS += -lm
        IFLAGS += -I../ext/x86_64-pc-linux-gnu/
        LFLAGS += -L../ext/x86_64-pc-linux-gnu/ -lffi
        CCFLAGS += -D LINUX
    endif
    #ifeq ($(UNAME_S),Darwin)
    #    CCFLAGS += -D OSX
    #endif
    #UNAME_P := $(shell uname -p)
    #ifeq ($(UNAME_P),x86_64)
    #    CCFLAGS += -D AMD64
    #endif
    #ifneq ($(filter %86,$(UNAME_P)),)
    #    CCFLAGS += -D IA32
    #endif
    #ifneq ($(filter arm%,$(UNAME_P)),)
    #    CCFLAGS += -D ARM
    #endif
endif

CFLAGS += -o test_partial$(EXT)

all: build

build:
	$(CC) $(CFLAGS) $(IFLAGS) $(CCFLAGS) test_partial.c ../src/partial.c $(LFLAGS)
