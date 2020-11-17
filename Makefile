CFLAGS = --std=gnu99

LFLAGS = -lpthread

main: line_processor.c
	$(CC) -o line_processor line_processor.c

clean:
	rm -f line_processor
