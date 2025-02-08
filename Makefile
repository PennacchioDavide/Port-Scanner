all:
	gcc -c -o main.o main.c
	gcc -o main main.o

clean:
	rm -f main exe/*.o