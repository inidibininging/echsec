#!/bin/bash
emcc src/*.c --preload-file tests/parser/leakcheck.echse -o echsec.wasm --no-entry
