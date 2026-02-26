#!/bin/bash
valgrind -s --leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	./echsec tests/parser/leakcheck.echse
