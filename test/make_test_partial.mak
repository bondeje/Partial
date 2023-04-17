UNAME := $(shell uname)
CC = gcc

EXT = 
LFLAGS = 
CFLAGS = -std=c99 -O2 -Wall -pedantic
IFLAGS = -I../include

ifeq ($(OS),Windows_NT)
	# might have to encapsulate with a check for MINGW. Need this because Windows f-s up printf with size_t and MINGW only handles it with their own implementation of stdio
	CFLAGS += -D__USE_MINGW_ANSI_STDIO
	EXT = .exe
    #CCFLAGS += -D WIN32
    #ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
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
    CFLAGS += -g
    ifeq ($(UNAME_S),Linux)
		# needed because linux must link to the math
		LFLAGS += -lm
        #CCFLAGS += -D LINUX
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
	$(CC) $(CFLAGS) $(IFLAGS) test_partial.c ../src/partial.c $(LFLAGS)
