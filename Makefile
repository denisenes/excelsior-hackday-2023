compile_debug:
	g++ -g3 main.cpp traversal.cpp -o main

compile_release:
	g++ -O4 -DNDEBUG main.cpp traversal.cpp -o main

disasm:
	g++ -O4 -DNDEBUG -c -S -masm=intel main.cpp traversal.cpp
	#objdump --prefix-addresses -d -C -M intel main.o > main.asm
	#objdump --prefix-addresses -d -C -M intel traversal.o > traversal.asm

clean:
	rm *.s
	rm *.o
