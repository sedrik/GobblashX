# Makefile

# Project info
PROJECT_NAME=GobblashX
EDITOR_NAME=TheGobbitor
PROJECT_DIR=./
SRC_DIR=$(PROJECT_DIR)src/
INCLUDE_DIR=$(PROJECT_DIR)include/
OBJS_DIR=$(PROJECT_DIR)objs/

# Header files for the game
HEADERS=$(INCLUDE_DIR)map.h $(INCLUDE_DIR)aura.h $(INCLUDE_DIR)controller.h \
$(INCLUDE_DIR)controllerkey.h $(INCLUDE_DIR)editor.h                        \
$(INCLUDE_DIR)creature.h $(INCLUDE_DIR)fxfield.h                            \
$(INCLUDE_DIR)graphics.h $(INCLUDE_DIR)grid.h $(INCLUDE_DIR)game.h          \
$(INCLUDE_DIR)physics.h $(INCLUDE_DIR)platform.h                            \
$(INCLUDE_DIR)player.h $(INCLUDE_DIR)pomgob.h $(INCLUDE_DIR)settings.h      \
$(INCLUDE_DIR)swordslash.h $(INCLUDE_DIR)grassplatform.h                    \
$(INCLUDE_DIR)sbbflyer.h $(INCLUDE_DIR)filefunctions.h                      \
$(INCLUDE_DIR)keymappings.h $(INCLUDE_DIR)trigger.h                         \
$(INCLUDE_DIR)zombo.h $(INCLUDE_DIR)scanresult.h $(INCLUDE_DIR)aitools.h    \
$(INCLUDE_DIR)hashtable.h $(INCLUDE_DIR)hashtable_gen.h                     \
$(INCLUDE_DIR)invizdmgfield.h $(INCLUDE_DIR)text.h $(INCLUDE_DIR)location.h

# Object files for the game
OBJS_GAME=$(OBJS_DIR)map.o $(OBJS_DIR)aura.o $(OBJS_DIR)controller.o  \
$(OBJS_DIR)creature.o $(OBJS_DIR)fxfield.o                            \
$(OBJS_DIR)graphics.o $(OBJS_DIR)grid.o $(OBJS_DIR)game.o             \
$(OBJS_DIR)physics.o $(OBJS_DIR)platform.o                            \
$(OBJS_DIR)player.o $(OBJS_DIR)pomgob.o $(OBJS_DIR)settings.o         \
$(OBJS_DIR)swordslash.o $(OBJS_DIR)grassplatform.o                    \
$(OBJS_DIR)sbbflyer.o $(OBJS_DIR)filefunctions.o                      \
$(OBJS_DIR)keymappings.o $(OBJS_DIR)trigger.o                         \
$(OBJS_DIR)zombo.o $(OBJS_DIR)scanresult.o $(OBJS_DIR)aitools.o       \
$(OBJS_DIR)hashtable.o $(OBJS_DIR)invizdmgfield.o $(OBJS_DIR)location.o \
$(OBJS_DIR)blubber.o

OBJS_GOBBLASHX=$(OBJS_DIR)engine.o $(OBJS_GAME)
OBJS_EDITOR=$(OBJS_DIR)editengine.o $(OBJS_DIR)editor.o  $(OBJS_GAME)   \
	$(OBJS_DIR)text.o

# Other gcc flags
CFLAGS += -Wall -Werror -Wno-unused -I $(INCLUDE_DIR) -I/usr/local/include -g -ggdb
#LDFLAGS += -lSDL -lpthread -L/usr/local/lib
MACCFLAGSLINKING += -lSDLmain -lSDL -Wl,-framework,Cocoa -lpthread -L/sw/lib
CFLAGSLINKING += -lSDL $(CFLAGS) -L/usr/local/lib
CFLAGSDEBUG += -ggdb $(CFLAGS)
CFLAGSDEBUGLINKING += -lSDL $(CFLAGSDEBUG)

# Compiler and linker commands
CC=g++

# Defines how object files (*.o) are created from C++ files (*.cpp)
$(OBJS_DIR)%.o: $(SRC_DIR)%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

# Deault: build everything!
all: gobblashx editor

# Build gobblashx
gobblashx: $(OBJS_GOBBLASHX)
	$(CC) $(OBJS_GOBBLASHX) -o $(PROJECT_NAME) $(CFLAGSLINKING) 

# Build the editor
editor: $(OBJS_EDITOR)
	$(CC) $(OBJS_EDITOR) -o $(EDITOR_NAME) $(CFLAGSLINKING)

# Build for mac
mac: $(OBJS_GOBBLASHX)
	$(CC) $(MACCFLAGSLINKING) -o $(PROJECT_NAME) $(OBJS_GOBBLASHX)

# clean: remove object files and emacs backup files
.PHONY: clean
clean:
	rm -f $(PROJECT_NAME) $(EDITOR_NAME) $(SRC_DIR)*.o *~ src/*~ include/*~ maps/*~ docs/*~ objs/*.o
