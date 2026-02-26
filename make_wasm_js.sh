#!/bin/bash
# emcc src/*.c -o echsec.js -sMODULARIZE -sEXPORTED_RUNTIME_METHODS=ccall
emcc src/*.c -o echsec.js -sMODULARIZE -s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap", "Pointer_stringify"]' -s 'WASM=0' -s 'NO_FILESYSTEM=1' -s ALLOW_MEMORY_GROWTH=1  -s ENVIRONMENT=node -O0 -g3 -s "SAFE_HEAP=1" -s "STACK_OVERFLOW_CHECK=1" -s "ASSERTIONS=1" -s 'ALIASING_FUNCTION_POINTERS=0'
mv echsec.js web
mv echsec.wasm web
