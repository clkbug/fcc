TARGET := fcc
CFLAGS := -Wall -Werror -Wpedantic -g

SRCS := \
	main.c \

OBJS := $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: test clean debug
test: $(TARGET)
	./test/test

clean:
	rm -f $(TARGET) $(OBJS)

debug: $(TARGET)
	./$(TARGET) "$(ARGS)" >/tmp/a.s
	riscv32-unknown-elf-gcc -g -o /tmp/a.out /tmp/a.s test/predefined.c
	qemu-riscv32 -g 12345 /tmp/a.out &
	riscv32-unknown-elf-gdb /tmp/a.out -x qemu.cmd >qemu.out.txt </dev/null
