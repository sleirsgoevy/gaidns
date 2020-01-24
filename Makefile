all: gaidns

clean:
	rm -f gaidns

gaidns: main.c handlers.c
	gcc main.c handlers.c -o gaidns
