CC := clang++

BIN_DIR := bin
BUILD_DIR := build
INCLUDES_DIR := includes
EXAMPLES_DIR := examples

MIN_MAC_VER := 10.12
CPPFLAGS := -std=c++2a -mmacosx-version-min=$(MIN_MAC_VER) -g # TODO: -Wall -Werror
OBJCFLAGS := -std=c++2a -x objective-c++ -mmacosx-version-min=$(MIN_MAC_VER) -g
LDFLAGS := -framework Metal -framework MetalKit -framework Cocoa -framework CoreFoundation -fobjc-link-runtime
INCLUDES := -Iincludes -Isrc

SRC_DIR := src
CPP_SOURCES := $(shell FIND $(SRC_DIR) -type f -name *.cpp)
OBJC_SOURCES := $(shell FIND $(SRC_DIR) -type f -name *.mm)
DEPS_SOURCES := $(INCLUDES_DIR)/mtlpp/mtlpp.mm

CPP_CODE_OBJECTS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(CPP_SOURCES:.cpp=.cpp.o))
OBJC_CODE_OBJECTS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(OBJC_SOURCES:.mm=.mm.o))
DEPS_CODE_OBJECTS := $(patsubst $(INCLUDES_DIR)/%,$(BUILD_DIR)/$(INCLUDES_DIR)/%,$(DEPS_SOURCES:.mm=.o))
CODE_OBJECTS := $(CPP_CODE_OBJECTS) $(OBJC_CODE_OBJECTS) $(DEPS_CODE_OBJECTS)

# Build C++ object files.
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) -c -o $@ $<

# Build Objective C object files.
$(BUILD_DIR)/%.mm.o: $(SRC_DIR)/%.mm
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c -o $@ $<

# Build Objective C dependencies
$(BUILD_DIR)/$(INCLUDES_DIR)/%.o: $(DEPS_SOURCES)
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c -o $@ $<

$(BIN_DIR)/%: $(EXAMPLES_DIR)/%.cpp $(CODE_OBJECTS) $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(CODE_OBJECTS) -o $@ $<

# Install the dependency of mtlpp.
$(INCLUDES_DIR)/mtlpp: $(INCLUDES_DIR)
	git clone git@github.com:naleksiev/mtlpp.git $(INCLUDES_DIR)/mtlpp
	cd includes/mtlpp && \
		git checkout -b checkout-v1 71a38f4e8bcf7a06bdb234cbe13c6299a9bb127e

# Ensure the bin dir is created if needed.
$(BIN_DIR):
	mkdir -p ./$(BIN_DIR)

# Ensure the includes dir is created if needed.
$(INCLUDES_DIR):
	mkdir -p ./$(INCLUDES_DIR)

.PHONY: clean
clean:
	@echo " Cleaning...";
	$(RM) -r ./$(BIN_DIR)
	$(RM) -r ./$(BUILD_DIR)
