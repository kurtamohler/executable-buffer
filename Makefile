INCLUDE=-I include
BUILD=build

TARGETS=$(BUILD)/nop_example
FLAGS=-g -fPIC -static

all: $(BUILD) $(TARGETS)

clean:
	rm -rf $(BUILD)

$(BUILD)/nop_example: examples/nop_example.cpp
	g++ -o $@ $(FLAGS) $^ $(INCLUDE)

$(BUILD):
	mkdir -p $(BUILD)


