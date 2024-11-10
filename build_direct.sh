#!/bin/bash
g++ src/main.cpp src/ecs.cpp src/systems.cpp src/object.cpp -lGL -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lSDL2_mixer -lSDL2_image -o stormCastle -Wall -g
