# PowerBudget v0.00.01a 2024/08/20 calculate power dissipation and budget
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

# Makefile used to build to Linux, MinGW/Win, macOS

SRCCLI = powerb.c fileIo.c
SRCGUI = powerbGui.c fileIo.c
SRC = $(SRCCLI) $(SRCGUI)

OBJCLI = $(SRCCLI:.c=.o)
OBJGUI = $(SRCGUI:.c=.o)
OBJ = $(OBJCLI) $(OBJGUI)

BINCLI = powerb
BINGUI = powerbGui
BIN = $(BINCLI) $(BINGUI)

# Flags
CFLAGS += -std=gnu99 -O2 -I/usr/include/iniparser
GFLAGS += -std=gnu99 -O2
GFLAGS += `sdl2-config --cflags`

ifeq ($(OS),Windows_NT)
	BIN := $(BIN).exe
	LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS = -lSDL2 -framework OpenGL -lm
	else
		LIBS += -liniparser
		GLIBS += -lm -ldl `sdl2-config --libs`
	endif
endif

all: $(BIN)

$(BIN): fileIo.o
	$(CC) $(SRCCLI) $(CFLAGS) $(LIBS) -o $(BINCLI)
	$(CC) $(SRCGUI) $(GFLAGS) $(GLIBS) -o $(BINGUI)

fileIo.o: fileIo.c

debug:
	$(CC) -g -fsanitize=address $(SRCCLI) $(CFLAGS) $(LIBS) -o $(BINCLI)
	$(CC) -g -fsanitize=address $(SRCGUI) $(GFLAGS) $(GLIBS) -o $(BINGUI)

clean:
	rm -f $(BIN) $(OBJ)

strip:
	strip $(BIN)
