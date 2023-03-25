SRCS := \
	putchar.c \
	putchar_multi.c \
	putchar_escape_character.c \
	arith.c \
	if_else.c \
	cmp.c \
	local_variable.c \
	local_variable_long_name.c \
	loop_while.c \
	loop_for.c \


REF_EXE := $(SRCS:.c=.ref.exe)
REF_STDOUT := $(REF_EXE:.exe=.stdout)

TEST_EXE := $(SRCS:.c=.test.exe)
TEST_ASM := $(SRCS:.c=.test.s)
TEST_STDOUT := $(TEST_EXE:.exe=.stdout)

TEST_CMP_RESULT := $(SRCS:.c=.cmp)

FCC := ../fcc
GCC := /home/fukuda/opt/gcc/riscv32-unknown-elf-rv32im/bin/riscv32-unknown-elf-gcc
QEMU := qemu-riscv32

all: $(TEST_CMP_RESULT) $(TEST_STDOUT) $(TEST_ASM) $(TEST_EXE) $(REF_STDOUT) $(REF_EXE)
	@echo all tests passed!

# .PHONY: $(TEST_CMP_RESULT)
%.cmp: %.test.stdout %.ref.stdout
	diff $^ >$@

%.ref.stdout: %.ref.exe
	$(QEMU) $< > $@
%.test.stdout: %.test.exe
	$(QEMU) $< > $@

%.ref.exe: %.c
	$(GCC) -o $@ $<

%.test.exe: %.test.s
	$(GCC) -o $@ $<
%.test.s: %.c $(FCC)
	$(FCC) $< >$@

.PHONY: clean
clean:
	rm -f $(REF_EXE) $(REF_STDOUT) $(TEST_EXE) $(TEST_ASM) $(TEST_STDOUT) $(TEST_CMP_RESULT)