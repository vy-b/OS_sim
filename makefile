all:
	gcc -Wall OS_sim.c pcb.c list.c -o OS_sim

valgrind: build
	valgrind --leak-check=full ./OS_sim

clean:
	rm OS_sim