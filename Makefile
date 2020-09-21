CC := clang++

# This mac version has std::optional::value()
MIN_MAC_VER := 10.15
METAL_FLAGS :=
CPP_FLAGS := \
	-std=c++2a \
	-mmacosx-version-min=$(MIN_MAC_VER) -g \
	-Wno-unused-command-line-argument \
	# TODO: -Wall -Werror

ifdef RELEASE
CPP_FLAGS += -DNDEBUG
else
CPP_FLAGS += -DDEBUG
METAL_FLAGS += -gline-tables-only -MO
endif

OBJCFLAGS := \
	-std=c++2a \
	-x \
	objective-c++ \
	-mmacosx-version-min=$(MIN_MAC_VER) \
	-g

LDFLAGS := \
	-framework Cocoa \
	-framework CoreFoundation \
	-framework GLKit \
	-framework Metal \
	-framework MetalKit \
	-fobjc-link-runtime

INCLUDES := -Iincludes -Isrc

CPP_SOURCES := $(shell FIND src/viz -type f -name *.cpp)
OBJC_SOURCES := $(shell FIND src/viz -type f -name *.mm)
METAL_SOURCES := $(shell FIND src -type f -name *.metal)
DEPS_SOURCES := includes/mtlpp/mtlpp.mm

CPP_CODE_OBJECTS := $(patsubst src/viz/%,build/%,$(CPP_SOURCES:.cpp=.cpp.o))
OBJC_CODE_OBJECTS := $(patsubst src/viz/%,build/%,$(OBJC_SOURCES:.mm=.mm.o))
DEPS_CODE_OBJECTS := $(patsubst includes/%,build/includes/%,$(DEPS_SOURCES:.mm=.o))
METAL_AIR := $(patsubst src/%,build/%,$(METAL_SOURCES:.metal=.air))
CODE_OBJECTS := $(CPP_CODE_OBJECTS) $(OBJC_CODE_OBJECTS) $(DEPS_CODE_OBJECTS)

# Build C++ object files.
build/%.cpp.o: src/viz/%.cpp src/viz/%.h
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(CPP_FLAGS) $(LDFLAGS) $(INCLUDES) -c $< -o $@

# Build Objective C object files.
build/%.mm.o: src/viz/%.mm src/viz/%.h
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c $< -o $@

# Retain the object files.
.PRECIOUS: build/%.cpp.o build/%.mm.o build/includes/%.o build/%.air bin/%.metallib

# Build Objective C dependencies
build/includes/%.o: $(DEPS_SOURCES)
	@mkdir -p $(shell echo $@ | sed -e 's/\/[^\/]*\.o//g')
	$(CC) $(OBJCFLAGS) $(INCLUDES) -c $< -o $@

bin/%: src/examples/%.cpp $(CODE_OBJECTS) $(METAL_SOURCES) bin bin/Info.plist bin/%.metallib
	$(CC) $(CPP_FLAGS) $(LDFLAGS) $(INCLUDES) $(CODE_OBJECTS) -o $@ $<
	@echo "✨ Done building ✨"
	@echo ""

# Compile the intermediate representation of metal files.
build/%.air: src/%.metal
	mkdir -p $(shell dirname $@)
	xcrun -sdk macosx metal $(INCLUDES) $(METAL_FLAGS) -c $< -o $@

# Build the final metallib files.
bin/%.metallib: $(METAL_AIR)

	xcrun -sdk macosx metallib build/examples/$(shell basename $@ .metallib).air $< -o $@

# Install the dependency of mtlpp.
includes/mtlpp: includes
	git clone git@github.com:naleksiev/mtlpp.git includes/mtlpp && \
		cd includes/mtlpp && \
		git checkout -b checkout-v1 71a38f4e8bcf7a06bdb234cbe13c6299a9bb127e

# Ensure the bin dir is created if needed.
bin:
	mkdir -p ./bin

# Ensure the bin dir is created if needed.
bin/Info.plist: bin
	cp src/Info.plist bin/Info.plist

# Ensure the includes dir is created if needed.
includes:
	mkdir -p ./includes

.PHONY: clean
clean:
	@echo " Cleaning...";
	$(RM) -r ./bin
	$(RM) -r ./build

.PHONY: clean-cpp
clean-cpp:
	@echo " Cleaning cpp object files and binaries ...";
	$(RM) -r ./bin
	$(RM) -r ./build/*.cpp.o

# The scripts directory needs the node modules installed.
./scripts/node_modules:
	cd scripts && npm install

# Enable live-reloading. This is built with a node script that orchestrates things.
# Run `make watch EXAMPLE=bunny` where bunny is the name of the example.
.PHONY: watch
watch: ./scripts/node_modules
ifndef EXAMPLE
	@echo Error: An EXAMPLE is required.
	@echo Usage: make watch EXAMPLE=bunny
else
	node ./scripts/watch ${EXAMPLE}
endif

.PHONY: init
init: includes/mtlpp

.PHONY: all
all:
	make ./bin/box
	make ./bin/bunny
