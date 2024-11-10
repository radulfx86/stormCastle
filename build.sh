/home/fre/local/emsdk/upstream/emscripten/emcc \
    -o stormCastle.js \
    -sUSE_SDL=2 \
    -sFULL_ES3=1 \
    -sWASM=0 \
    src/main.cpp \
    src/ecs.cpp \
    src/systems.cpp \
    src/object.cpp \
    --preload-file shaders \
    --preload-file assets/images/characters.png \
    --use-preload-plugins \
    -v
