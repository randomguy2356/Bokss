PROJECT := Bokss
CC      ?= gcc

SRC := src/main.c src/glad.c src/Shader.c
OBJ := $(SRC:src/%.c=build/%.o)

CFLAGS ?= -O3 -g -Iinclude -Wall -Wextra -std=c23
LDLIBS  = -lGL -lglfw

.PHONY: all clean run

all: build/$(PROJECT)

build/$(PROJECT): $(OBJ)
	$(CC) $^ -o $@ $(LDLIBS) -Iinclude

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./build/$(PROJECT)

clean:
	rm -rf build

-include $(OBJ:.o=.d)

