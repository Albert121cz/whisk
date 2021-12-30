CXX := g++
CXXFLAGS := -Winvalid-pch -Wall
LDFLAGS  := -Wl,-subsystem,windows

debug: CXXFLAGS += -g -DDEBUG
debug: LDFLAGS  += -Wl,-subsystem,console

OUTPUT := main.exe

CXXFLAGS += -I./lib/gcc810_x64_dll/mswu -I./include
LDFLAGS += -L./lib/gcc810_x64_dll
LDFLAGS += -L./lib/glew
LDFLAGS += -lwxbase315u_gcc810_x64 -lwxmsw315u_core_gcc810_x64 -lwxmsw315u_gl_gcc810_x64 -lopengl32
LDFLAGS += -lglew32

SOURCES := $(wildcard ./src/*.cpp)
OBJS := $(SOURCES:%.cpp=%.o)

DEPS := $(wildcard ./src/*.hpp)

all: $(OUTPUT)

debug: all

$(OUTPUT): $(OBJS)
	@echo linking...
	@$(CXX) -o ./build/$@ $(OBJS) $(LDFLAGS)

.PHONY: all debug clean

%.o: %.cpp $(DEPS)
	@echo $(CXX) -c $<
	@$(CXX) $< -c -o $@ $(CXXFLAGS)

%.hpp.gch: %.hpp
	@echo $(CXX) -c $<
	@$(CXX) $< -c -o $@ $(CXXFLAGS)

clean:
	@echo rm $(OBJS)
	@powershell -Command "echo $(OBJS) | Remove-Item"
