/home/fre/local/emsdk/upstream/emscripten/emcc \
    -o stormCastle.js \
    -sUSE_SDL=2 \
    -sFULL_ES3=1 \
    -sWASM=0 \
    src/main.cpp \
    src/ecs.cpp \
    src/systems.cpp \
    src/object.cpp \
    src/tools.cpp \
    src/io.cpp \
    src/shaders.cpp \
    src/object_factory.cpp \
    --preload-file shaders \
    --preload-file assets/images/characters.png \
    --preload-file assets/images/tiles.png \
    --preload-file assets/images/font4_8.png \
    --preload-file assets/images/font8_12.png \
    --preload-file level.txt \
    --use-preload-plugins \
    -v
