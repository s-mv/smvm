CC ?= clang
TITLE = smvm
OBJECTS = out/util.o out/smvm.o out/asmv.o out/dsmv.o out/functions.o
INCLUDE = -I ./src
CFLAGS = -std=c99 -fshort-enums -fPIC -g
PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include/$(TITLE)

dev: $(OBJECTS)
	$(CC) main.c $(OBJECTS) $(INCLUDE) -o out/$(TITLE) $(CFLAGS)

test: $(OBJECTS)
	$(CC) tests/tests.c $(OBJECTS) $(INCLUDE) -o out/tests $(CFLAGS)
	./out/tests

DIR = $(PREFIX)/bin
vm:
	sudo $(CC) main.c $(OBJECTS) $(INCLUDE) -o $(DIR)/$(TITLE) $(CFLAGS)

out/%.o: src/%.c
	$(CC) -c $< $(INCLUDE) -o $@ $(CFLAGS)

out/lib$(TITLE).a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

out/lib$(TITLE).so: $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS) $(CFLAGS)

install: out/lib$(TITLE).a out/lib$(TITLE).so
	sudo mkdir -p $(LIBDIR) $(INCDIR)
	sudo cp out/lib$(TITLE).a out/lib$(TITLE).so $(LIBDIR)/
	sudo cp src/*.h $(INCDIR)/
	sudo ldconfig

clean:
	rm -rf out/*
