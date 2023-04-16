TARGET := fcc
TARGET2 := fcc2
TARGET3 := fcc3
CFLAGS := -Wall -Wpedantic -g

.PHONY: all
all: $(TARGET) $(TARGET2) $(TARGET3)

$(TARGET): main.c util.c
	gcc $(CFLAGS) -o $@ $^

$(TARGET2): main.c util.c $(TARGET)
	./preprocess.py main.c >main_preprocessed.c
	./$(TARGET) <main_preprocessed.c >fcc2.s
	riscv32-unknown-elf-gcc -o $@ util.c fcc2.s
$(TARGET3): $(TARGET2)
	./$(TARGET2) <main_preprocessed.c >fcc3.s
	riscv32-unknown-elf-gcc -o $@ util.c fcc3.s

.PHONY: test clean debug
test: $(TARGET)
	$(MAKE) -C test
	./test/test

clean:
	rm -f $(TARGET) $(TARGET2) $(TARGET3) main_preprocessed.c fcc2.s fcc3.s

debug: $(TARGET)
	./$(TARGET) "$(ARGS)" >/tmp/a.s
	riscv32-unknown-elf-gcc -g -o /tmp/a.out /tmp/a.s test/predefined.c
	qemu-riscv32 -g 12345 /tmp/a.out &
	riscv32-unknown-elf-gdb /tmp/a.out -x qemu.cmd >qemu.out.txt </dev/null

podman-debug: clean
	podman run --interactive --rm -v ${PWD}:/work:z rv32-compiler make
	podman run --interactive --rm -v ${PWD}:/work:z rv32-compiler make debug ARGS="$(ARGS)"