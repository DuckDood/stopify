all: obj/ build/ obj/main.o build/stopify 

clean:
	rm -r obj/ build/ 
.PHONY: clean

obj/: 
	mkdir -p obj/

build/: 
	mkdir -p build/

obj/main.o: src/main.c
	${CC} src/main.c -c -o obj/main.o 

build/stopify: obj/main.o 
	${CXX} obj/main.o  -o build/stopify -lncurses -lmenu -lSDL2 -lSDL2_mixer -lpthread 

