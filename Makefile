CC := gcc
CFLAGS :=
DEPS := src/modled.h
OBJ := src/modled.o
BUILD := build
WARN := -Wall

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(WARN)

build/modled: $(OBJ)
	mkdir -p $(BUILD)
	$(CC) -o $@ $^ $(CFLAGS) $(WARN)

install:
	cp $(BUILD)/modled /usr/local/bin
	cp doc/man/modled.1 /usr/local/share/man/man1

clean:
	rm -r $(OBJ)
	rm -rf $(BUILD)/
