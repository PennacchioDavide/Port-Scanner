all: program

exe:
	mkdir -p exe

exe/main.o: src/main.c | exe
	gcc -c -o exe/main.o src/main.c


FILEO = exe/main.o

program: $(FILEO)
	gcc -o main $(FILEO)

clean:
	rm -f main exe/*.o