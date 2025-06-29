all: obj/ build/ obj/main.o obj/getAudInfo.o obj/getAudName.o build/stopify 

install:
	install build/stopify /usr/local/bin/
.PHONY: install

clean:
	rm -r obj/ build/ 
.PHONY: clean

obj/: 
	mkdir -p obj/

build/: 
	mkdir -p build/

obj/main.o: src/main.c
	${CC} src/main.c -c -o obj/main.o 

obj/getAudInfo.o: src/getAudInfo.c
	${CC} src/getAudInfo.c -c -o obj/getAudInfo.o 

obj/getAudName.o: src/getAudName.c
	${CC} src/getAudName.c -c -o obj/getAudName.o 

build/stopify: obj/main.o obj/getAudInfo.o obj/getAudName.o 
	${CXX} obj/main.o obj/getAudInfo.o obj/getAudName.o  -o build/stopify -lncurses -lmenu -lSDL2 -lSDL2_mixer -lavformat -lavutil 

