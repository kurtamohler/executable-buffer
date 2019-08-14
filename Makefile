
INCLUDE=-I include
BUILD=build
OBJ=$(BUILD)/executable_buffer.o

TARGETS=$(BUILD)/nop_example
FLAGS=-g -fPIC -static

all: $(BUILD) $(TARGETS)

clean:
	rm -rf $(BUILD)

$(BUILD)/nop_example.o: examples/nop_example.cpp
	g++ -o $@ $(INCLUDE) $(FLAGS) -c $^

$(BUILD)/nop_example: $(OBJ) $(BUILD)/nop_example.o
	g++ -o $@ $(FLAGS) $^

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/executable_buffer.o: src/executable_buffer.cpp
	g++ -o $@ $(INCLUDE) $(FLAGS) -c $^

