INCLUDE_DIR = -I ./include
CFLAGS = -Wall -Wextra -pedantic -std=c++20
SRC = ./src/*.cpp
OUTPUT = -o ./build/bin/cpm

build: src/*.cpp include/*.hpp
	mkdir -p ./build/bin
	g++ $(CFLAGS) $(INCLUDE_DIR) $(OUTPUT) $(SRC)

clean:
	rm -rf ./build