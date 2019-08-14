INCLUDE=-I include
BUILD=build
OBJ=$(BUILD)/executable_buffer.o

TARGETS=$(BUILD)/nop_example
FLAGS=-g -fPIC -static

all: $(BUILD) $(TARGETS)

clean:
	rm -rf $(BUILD)

$(BUILD)/nop_example: examples/nop_example.cpp
	g++ -o $@ $(FLAGS) $^ $(INCLUDE)

$(BUILD):
	mkdir -p $(BUILD)


