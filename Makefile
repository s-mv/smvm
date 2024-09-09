CFILES = src/util.c src/smvm.c src/asmv.c src/dsmv.c

example:
	$(CC) main.c $(CFILES) -I ./src -o out/example -std=c99 -fshort-enums -g

# header: TODO

clean:
	rm -rf out/*