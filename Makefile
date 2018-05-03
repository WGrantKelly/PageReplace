default: vmsim.c vmsim.h
	gcc -g vmsim.c -o vmsim
clean:
	rm vmsim