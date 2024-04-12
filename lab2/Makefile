all:build

build: parser lex ast.c main.c
	gcc parser.tab.c lex.yy.c ast.c main.c -o build

parser: parser.y
	bison -d  parser.y

lex: lex.l
	flex lex.l

run:
	build
	./build
clean:
	rm -r lex.yy.c parser.tab.c parser.tab.h build