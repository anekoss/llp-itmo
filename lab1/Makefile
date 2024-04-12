all: main

main: ./build/demo.o ./build/tests.o ./build/query.o ./build/operation.o ./build/condition.o ./build/file.o ./build/list.o ./build/doc.o ./build/element.o
	mkdir -p ./build
	gcc ./build/element.o ./build/doc.o ./build/list.o ./build/file.o ./build/condition.o ./build/operation.o ./build/query.o ./build/demo.o -o demo
	make test

test: ./build/tests.o ./build/query.o ./build/operation.o ./build/condition.o ./build/file.o ./build/list.o ./build/doc.o ./build/element.o
	mkdir -p ./build
	gcc ./build/element.o ./build/doc.o ./build/list.o ./build/file.o ./build/condition.o ./build/operation.o ./build/query.o ./build/tests.o -o test

./build/demo.o: ./src/demo.c
	mkdir -p ./build
	gcc -c ./src/demo.c -o ./build/demo.o

./build/tests.o: ./src/tests.c
	mkdir -p ./build
	gcc -c ./src/tests.c -o ./build/tests.o

./build/query.o: ./src/query.c ./include/query.h
	mkdir -p ./build
	gcc -c ./src/query.c -o ./build/query.o

./build/operation.o: ./src/operation.c ./include/operation.h
	mkdir -p ./build
	gcc -c ./src/operation.c -o ./build/operation.o

./build/condition.o: ./src/condition.c ./include/condition.h
	mkdir -p ./build
	gcc -c ./src/condition.c -o ./build/condition.o

./build/file.o: ./src/file.c ./include/file.h
	mkdir -p ./build
	gcc -c ./src/file.c -o ./build/file.o

./build/list.o: ./src/list.c ./include/list.h ./include/inode.h
	mkdir -p ./build
	gcc -c ./src/list.c -o ./build/list.o

./build/doc.o: ./src/doc.c ./include/doc.h
	mkdir -p ./build
	gcc -c ./src/doc.c -o ./build/doc.o

./build/element.o: ./src/element.c ./include/element.h
	mkdir -p ./build
	gcc -c ./src/element.c -o ./build/element.o

demo:
	./main

clean:
	rm -rf demo
	rm -rf test
	rm -rf ./build