a-Shell: src/main.c src/utility.c src/Definitions.h
	mkdir -p bin
	gcc -Wall src/main.c src/utility.c -o bin/a-Shell