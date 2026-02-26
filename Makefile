build:
	gcc -Wall -std=c99 -g ./src/*.c -o echsec && rm -Rf src/echsec.dSYM
run:
	./echsec

clean:
	rm echsec
