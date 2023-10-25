compile_debug:
	gcc -g3 main.c -o main

compile_baseline:
	gcc -O4 -DNDEBUG main.c -o main