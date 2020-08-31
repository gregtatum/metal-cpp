CC := clang++

MIN_MAC_VER := 10.12
CPPFLAGS := -std=c++2a -mmacosx-version-min=$(MIN_MAC_VER) -g # TODO: -Wall -Werror
OBJCFLAGS := -std=c++2a -x objective-c++ -mmacosx-version-min=$(MIN_MAC_VER) -g
LDFLAGS := -framework GLKit -framework Metal -framework MetalKit -framework Cocoa -framework CoreFoundation -fobjc-link-runtime
INCLUDES := -Iincludes -Isrc

SRC_DIR := src
CPP_SOURCES := $(shell FIND src -type f -name *.cpp)
OBJC_SOURCES := $(shell FIND src -type f -name *.mm)
DEPS_SOURCES := includes/mtlpp/mtlpp.mm

CPP_CODE_OBJECTS := $(patsubst src/%,build/%,$(CPP_SOURCES:.cpp=.cpp.o))
OBJC_CODE_OBJECTS := $(patsubst src/%,build/%,$(OBJC_SOURCES:.mm=.mm.o))
DEPS_CODE_OBJECTS := $(patsubst includes/%,build/includes/%,$(DEPS_SOURCES:.mm=.o))
CODE_OBJECTS := $(CPP_CODE_OBJECTS) $(OBJC_CODE_OBJECTS) $(DEPS_CODE_OBJECTS)

# Build C++ object files.
build/%.cpp.o: src/%.cpp
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) -c $< -o $@

# Build Objective C object files.
build/%.mm.o: src/%.mm
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c $< -o $@

# Retain the object files.
.PRECIOUS: build/%.cpp.o build/%.mm.o build/includes/%.o build/%.air bin/%.metallib

# Build Objective C dependencies
build/includes/%.o: $(DEPS_SOURCES)
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c $< -o $@

bin/%: examples/%.cpp $(CODE_OBJECTS) bin examples/%.metal bin/%.metallib
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(CODE_OBJECTS) -o $@ $<
	@echo "✨ Done building ✨"
	@echo ""

# Compile the intermediate representation of metal files.
build/%.air: examples/%.metal
	xcrun -sdk macosx metal -c $< -o $@

# Build the final metallib files.
bin/%.metallib: build/%.air
	xcrun -sdk macosx metallib $< -o $@

# Install the dependency of mtlpp.
includes/mtlpp: includes
	git clone git@github.com:naleksiev/mtlpp.git includes/mtlpp
	cd includes/mtlpp && \
		git checkout -b checkout-v1 71a38f4e8bcf7a06bdb234cbe13c6299a9bb127e

# Ensure the bin dir is created if needed.
bin:
	mkdir -p ./bin

# Ensure the includes dir is created if needed.
includes:
	mkdir -p ./includes

.PHONY: clean
clean:
	@echo " Cleaning...";
	$(RM) -r ./bin
	$(RM) -r ./build
