# PowerBudget v0.00.01a 2024/09/08 calculate power dissipation and budget
# Copyright 2024 Valerio Messina http://users.iol.it/efa
# Makefile is part of PowerBudget
# PowerBudget is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# PowerBudget is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with PowerBudget. If not, see <http://www.gnu.org/licenses/>.

# Makefile used to build to: Linux, MinGW/Msys2/Win, macOS
# Win: MinGw64=>bin64 (MinGW/MSYS2)
# depend: $ sudo apt install libiniparser, libSDL2
#
# To build for debug use: $ make debug

HOST=$(shell uname -o)
PKG=$(HOST)
BIT=64

# Files
SRCCLI = powerb.c powerbLib.c fileIo.c
SRCGUI = powerbGui.c powerbLib.c fileIo.c
SRC = $(SRCCLI) $(SRCGUI)

OBJCLI = $(SRCCLI:.c=.o)
OBJGUI = $(SRCGUI:.c=.o)
OBJ = $(OBJCLI) $(OBJGUI)

BINCLI = powerb
BINGUI = powerbGui
BIN = $(BINCLI) $(BINGUI)

# Flags
CFLAGS = -std=gnu99 -Wall
GFLAGS = -std=gnu99 -Wall
#CINCS = -I/usr/include/iniparser
#GINCS = `sdl2-config --cflags` # -I/usr/include/SDL2 -D_REENTRANT
#CLIBS = -L../iniparser-v4.2.4/build
#GLIBS = -L../iniparser-v4.2.4/build
#LDFLAGS = -liniparser
#LGFLAGS = `sdl2-config --libs` # -lSDL2

ifeq ($(OS),Windows_NT)
	BIN := $(BIN).exe
	CINCS += -I../iniparser-v4.2.4/src
	GINCS += -I../iniparser-v4.2.4/src `sdl2-config --cflags`
	CLIBS += -L../iniparser-v4.2.4/build
	GLIBS += -L../iniparser-v4.2.4/build
	LDFLAGS += -liniparser
	LGFLAGS += -liniparser `sdl2-config --libs` -lSDL2main
else # Unix
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin) # macOS
		CINCS += -I/usr/include/iniparser
		GINCS += -I/usr/include/iniparser `sdl2-config --cflags`
		CLIBS += -L/usr/lib/x86_64-linux-gnu
		GLIBS += -L/usr/lib/x86_64-linux-gnu
		LDFLAGS += -liniparser
		LGFLAGS += -liniparser `sdl2-config --libs` -lm
	else # Linux
		CINCS += -I/usr/include/iniparser
		GINCS += -I/usr/include/iniparser `sdl2-config --cflags` # -I/usr/include/SDL2 -D_REENTRANT
		CLIBS += -L/usr/lib/x86_64-linux-gnu
		GLIBS += -L/usr/lib/x86_64-linux-gnu
		LDFLAGS += -liniparser
		LGFLAGS += -liniparser `sdl2-config --libs` -lm # -lSDL2
	endif
endif

all: CFLAGS+=-O3
all: GFLAGS+=-O3
all: $(BIN)

$(BIN):
	$(CC) $(CFLAGS) $(CINCS) $(SRCCLI) $(CLIBS) $(LDFLAGS) -o $(BINCLI)
	$(CC) $(GFLAGS) $(GINCS) $(SRCGUI) $(GLIBS) $(LGFLAGS) -o $(BINGUI)

#fileIo.o: fileIo.c

#powerbLib.o: powerbLib.c

strip:
	strip $(BIN)

cleanobj:
	rm -f *.o

cleanbin:
	rm -f $(CLI) $(GUI)

clean:
	rm -f $(BIN) $(OBJ)

debug: CFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer
debug: GFLAGS+=-O1 -g -fsanitize=address -fno-omit-frame-pointer
debug: LDFLAGS+=-fsanitize=address
debug: LGFLAGS+=-fsanitize=address
debug:
	$(CC) $(CFLAGS) $(CINCS) $(SRCCLI) $(CLIBS) $(LDFLAGS) -o $(BINCLI)
	$(CC) $(GFLAGS) $(GINCS) $(SRCGUI) $(GLIBS) $(LGFLAGS) -o $(BINGUI)

bin: all cleanobj strip

force: clean bin
	rm -f *.o

pkg:
	@echo ""
	mv $(CLI) $(GUI) ..
	@makePkg.sh $(PKG) $(BIT)

rel: force pkg
