#include <stdio.h>

void print_header()
{
    printf("\t.file	\"main.c\"\n");
    printf("\t.option nopic\n");
    printf("\t.text\n");
    printf("\t.section\t.rodata\n");
    printf("\t.align\t4\n");
    printf("\n");
}

void print_main()
{
    printf("\t.globl\tmain\n");
    printf("\t.type	main, @function\n");
    printf("main:\n");
    printf("\tli\ta0, 42\n");
    printf("\tret\n");
}

int main()
{
    print_header();
    print_main();
    return 0;
}