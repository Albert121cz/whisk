MAKEFLAGS += -j 5

CXX := clang++
CXXFLAGS := -Winvalid-pch -Wall -Wextra --std=c++17 --target=x86_64-w64-windows-gnu
LDFLAGS := -Wl,-subsystem,windows --target=x86_64-w64-windows-gnu

debug: CXXFLAGS += -g -DDEBUG
debug: LDFLAGS  += -Wl,-subsystem,console

OUTPUT := main.exe

SRC_DIR = ./src
BUILD_DIR = ./build
SHADERS_DIR = ./src/shaders

# including as system libs will suppress warnings
CXXFLAGS += -isystem./lib/gcc810_x64_dll/mswu -isystem./include
LDFLAGS += -L./lib/gcc810_x64_dll
LDFLAGS += -L./lib/glew
LDFLAGS += -lwxbase315u_gcc810_x64 -lwxmsw315u_core_gcc810_x64 -lwxmsw315u_gl_gcc810_x64
LDFLAGS += -lopengl32 -lglew32

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SOURCES:%.cpp=%.o)

SHADERS_SRC := $(wildcard $(SHADERS_DIR)/*.vert) $(wildcard $(SHADERS_DIR)/*.frag)
SHADERS := $(subst $(SHADERS_DIR),$(BUILD_DIR),$(SHADERS_SRC))

DEPS := $(wildcard $(SRC_DIR)/*.hpp)

all: $(OUTPUT)

debug: all

$(OUTPUT): $(OBJS) $(SHADERS)
	@echo linking...
	@$(CXX) -o ./build/$@ $(OBJS) $(LDFLAGS)

.PHONY: all debug clean test

%.o: %.cpp $(DEPS)
	@echo $(CXX) -c $<
	@$(CXX) $< -c -o $@ $(CXXFLAGS)

$(SHADERS): $(BUILD_DIR)/% : $(SHADERS_DIR)/%
	@echo copying $(@F)
	@powershell -Command "Copy-Item $< -Destination $@"

clean:
	@echo rm $(OBJS) $(SHADERS)
	@powershell -Command "echo $(OBJS) $(SHADERS) | Remove-Item"
