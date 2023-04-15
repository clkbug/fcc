TARGET := fcc
TARGET2 := fcc2
CFLAGS := -Wall -Werror -Wpedantic -g

SRCS := \
	main.c \

OBJS := $(SRCS:.c=.o)

.PHONY: all
all: $(TARGET) $(TARGET2)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET2): $(SRCS) $(TARGET)
	./preprocess.py $< >/tmp/$<
	./$(TARGET) /tmp/$< >/tmp/a.s
	$(CC) $(CFLAGS) -I. -o $@ /tmp/a.s


.PHONY: test clean debug
test: $(TARGET)
	$(MAKE) -C test
	./test/test

clean:
	rm -f $(TARGET) $(OBJS)

debug: $(TARGET)
	./$(TARGET) "$(ARGS)" >/tmp/a.s
	riscv32-unknown-elf-gcc -g -o /tmp/a.out /tmp/a.s test/predefined.c
	qemu-riscv32 -g 12345 /tmp/a.out &
	riscv32-unknown-elf-gdb /tmp/a.out -x qemu.cmd >qemu.out.txt </dev/null

podman-debug: clean
	podman run --interactive --rm -v ${PWD}:/work:z rv32-compiler make
	podman run --interactive --rm -v ${PWD}:/work:z rv32-compiler make debug ARGS="$(ARGS)"