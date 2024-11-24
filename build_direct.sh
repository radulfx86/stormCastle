#!/bin/bash
g++ src/main.cpp \
src/ecs.cpp \
src/io.cpp \
src/object.cpp \
src/systems.cpp \
src/tools.cpp \
src/shaders.cpp \
src/object_factory.cpp \
	-lGL -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lSDL2_mixer -lSDL2_image -o stormCastle -Wall -g
