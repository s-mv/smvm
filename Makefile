CC ?= clang
TITLE = smvm
OBJECTS = out/util.o out/smvm.o out/asmv.o out/dsmv.o out/functions.o
INCLUDE = -I ./src
CFLAGS = -std=c99 -fshort-enums -g

dev: $(OBJECTS)
	$(CC) main.c $(OBJECTS) $(INCLUDE) -o out/$(TITLE) $(CFLAGS)

test: $(OBJECTS)
	$(CC) tests/tests.c $(OBJECTS) $(INCLUDE) -o out/tests $(CFLAGS)
	./out/tests

DIR = /usr/local/bin
install:
	sudo $(CC) main.c $(OBJECTS) $(INCLUDE) -o $(DIR)/$(TITLE) $(CFLAGS)

out/%.o: src/%.c
	$(CC) -c $< $(INCLUDE) -o $@ -std=c99 -fshort-enums -g

clean:
	rm -rf out/*
