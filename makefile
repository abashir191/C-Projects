dissem:
	g++ -std=c++11 -o dissem disassembler.cpp
	rm -f *.o

clean: 
	rm -f *.o core a.out dissem
	rm -f out.list.txt
