# Unified Makefile for sky-tools
# Skylander NFC Dump and Clone Tools
# https://github.com/kooscode/cpp-template
# kdupreez@hotmail.com

# Directories
OUT_DIR = bin
LIB_DIR = lib
OBJ_DIR = obj

# Tools
TOOLS = sky-dump sky-clone sky-reset sky-identify sky-make

# Compiler settings
CXX = g++
CFLAGS = -g -Wall -ansi
CPP_STD = c++17

# Include paths
INCLUDES = -I/usr/include/PCSC -I$(LIB_DIR)

# Libraries
LIBS = -lpcsclite

# Library source files
LIB_SOURCES = $(wildcard $(LIB_DIR)/*.cpp)
LIB_OBJECTS = $(patsubst $(LIB_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

# Default target: build all tools
.PHONY: all clean sky-dump sky-clone sky-reset sky-identify sky-make

all: $(TOOLS)

# Create output directories
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile library objects (shared between tools)
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) $(INCLUDES) -c $< -o $@

# Build sky-dump
sky-dump: $(OUT_DIR) $(LIB_OBJECTS)
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) \
		sky-dump/src/sky-dump.cpp \
		$(LIB_OBJECTS) \
		-o $(OUT_DIR)/sky-dump \
		$(INCLUDES) $(LIBS)

# Build sky-clone
sky-clone: $(OUT_DIR) $(LIB_OBJECTS)
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) \
		sky-clone/src/sky-clone.cpp \
		$(LIB_OBJECTS) \
		-o $(OUT_DIR)/sky-clone \
		$(INCLUDES) $(LIBS)

# Build sky-reset
sky-reset: $(OUT_DIR) $(LIB_OBJECTS)
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) \
		sky-reset/src/sky-reset.cpp \
		$(LIB_OBJECTS) \
		-o $(OUT_DIR)/sky-reset \
		$(INCLUDES) $(LIBS)

# Build sky-identify (no NFC reader required - just reads dump files)
sky-identify: $(OUT_DIR) $(OBJ_DIR)/skylanderDB.o
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) \
		sky-identify/src/sky-identify.cpp \
		$(OBJ_DIR)/skylanderDB.o \
		-o $(OUT_DIR)/sky-identify \
		-I$(LIB_DIR)

# Build sky-make (create new Skylanders from scratch)
sky-make: $(OUT_DIR) $(LIB_OBJECTS)
	$(CXX) $(CFLAGS) -std=$(CPP_STD) $(DEFS) \
		sky-make/src/sky-make.cpp \
		$(LIB_OBJECTS) \
		-o $(OUT_DIR)/sky-make \
		$(INCLUDES) $(LIBS)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(OUT_DIR)/sky-dump $(OUT_DIR)/sky-clone $(OUT_DIR)/sky-reset $(OUT_DIR)/sky-identify $(OUT_DIR)/sky-make

# Install dependencies (Debian/Ubuntu)
install-deps:
	sudo apt-get install -y libpcsclite1 libpcsclite-dev libccid opensc

# Help
help:
	@echo "sky-tools Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all tools (default)"
	@echo "  sky-dump     - Build sky-dump only"
	@echo "  sky-clone    - Build sky-clone only"
	@echo "  sky-reset    - Build sky-reset only"
	@echo "  sky-identify - Build sky-identify only"
	@echo "  sky-make     - Build sky-make only"
	@echo "  clean        - Remove build artifacts"
	@echo "  install-deps - Install required dependencies (Debian/Ubuntu)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - libpcsclite-dev"
	@echo "  - ACR122U NFC reader (except sky-identify)"
	@echo "  - pcscd daemon running"
