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
	function.c \
	pointer.c \
	array.c \
	global_variable.c \
	char.c \
	local_variable_initialization.c \
	string.c \
	pre_increment.c \
	loop_break_and_continue.c \
	prime.c \
	function_void.c \
	function_arg.c \
	gcd.c \
	typedef.c \
	struct.c \
	struct_arrow.c \
	struct_alignment.c \
	struct_recursive.c \
	logicalop.c \
	permutation.c \
	cmp_torture.c \
	string_putchar.c \
	string_null.c \
	calculator_stack.c \
	bitwiseop.c \
	const.c \
	string_double_quote.c \
	sizeof.c \
	struct_recursive_inc.c \
	binary_tree.c \
	call_printf.c \
	# post_increment.c 	\


REF_EXE := $(SRCS:.c=.ref.exe)
REF_STDOUT := $(REF_EXE:.exe=.stdout)

TEST_EXE := $(SRCS:.c=.test.exe)
TEST_ASM := $(SRCS:.c=.test.s)
TEST_STDOUT := $(TEST_EXE:.exe=.stdout)

TEST_CMP_RESULT := $(SRCS:.c=.cmp)

FCC := ../fcc
# GCC := podman run --rm -v ${PWD}:/work:z rv32-compiler /usr/local/gcc/riscv32im-unknown-elf/bin/riscv32-unknown-elf-gcc
# QEMU := podman run --rm -v ${PWD}:/work:z rv32-compiler qemu-riscv32
GCC := riscv32-unknown-elf-gcc
QEMU := qemu-riscv32-static

all: $(TEST_CMP_RESULT) $(REF_STDOUT) $(REF_EXE) $(TEST_STDOUT) $(TEST_ASM) $(TEST_EXE)
	@echo all tests passed!

# .PHONY: $(TEST_CMP_RESULT)
%.cmp: %.ref.stdout %.test.stdout
	diff $^     # 差分があればここで止まる
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
	cat $< | $(FCC) >$@

.PHONY: clean
clean:
	rm -f $(REF_EXE) $(REF_STDOUT) $(TEST_EXE) $(TEST_ASM) $(TEST_STDOUT) $(TEST_CMP_RESULT)