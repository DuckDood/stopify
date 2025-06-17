all: obj/ build/ obj/main.o build/output 

clean:
	rm -r obj/ build/ 
.PHONY: clean

obj/: 
	mkdir -p obj/

build/: 
	mkdir -p build/

obj/main.o: src/main.c
	${CC} src/main.c -c -o obj/main.o 

build/output: obj/main.o 
	${CXX} obj/main.o  -o build/output -lncurses -lmenu -lSDL2 -lSDL2_mixer -lpthread 

