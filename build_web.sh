#!/bin/bash
set -e

echo "=== Building RamaCraft for Web (Emscripten / WebGL 2.0) ==="

mkdir -p web/dist

EMCC_FLAGS=(
    -std=c++17
    -O3
    -s USE_SDL=2
    -s FULL_ES3=1
    -s MAX_WEBGL_VERSION=2
    -s MIN_WEBGL_VERSION=2
    -s ALLOW_MEMORY_GROWTH=1
    -s INITIAL_MEMORY=67108864
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
    -Isrc
)

emcc src/*.cpp "${EMCC_FLAGS[@]}" -o web/dist/ramacraft.js

echo "Build complete! Output generated in web/dist/ramacraft.js and web/dist/ramacraft.wasm"
