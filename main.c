#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_header() {
  printf("\t.file	\"main.c\"\n");
  printf("\t.option nopic\n");
  printf("\t.text\n");
  printf("\t.section\t.rodata\n");
  printf("\t.align\t4\n");
  printf("\n");
}

void print_main_header() {
  printf("\t.globl\tmain\n");
  printf("\t.type	main, @function\n");
  printf("main:\n");
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

typedef enum {
  TK_RESERVED,
  TK_INT,
  TK_EOF,
} token_kind_t;

typedef struct token_t {
  token_kind_t kind;
  struct token_t *next;
  char *str;
  int num;
} token_t;

token_t *token;

bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("not '%c', got '%s'\n", op, token->str);
  }
  token = token->next;
}

int expect_int() {
  if (token->kind != TK_INT) {
    error("'%s' is not int\n", token->str);
  }
  int val = token->num;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

token_t *new_token(token_kind_t kind, token_t *cur, char *str) {
  token_t *tok = calloc(1, sizeof(token_t));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

token_t *tokenize(char *p) {
  token_t head;
  head.next = NULL;
  token_t *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_INT, cur, p);
      cur->num = strtol(p, &p, 10);
      continue;
    }

    error("failed to tokenize at '%c'\n", *p);
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

typedef enum {
  NODE_MINUS,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
  NODE_NUM,
} node_kind_t;

typedef struct node_t {
  node_kind_t kind;
  struct node_t *lhs;
  struct node_t *rhs;
  int val;
} node_t;

node_t *new_node() { return (node_t *)calloc(1, sizeof(node_t)); }

node_t *parse_int() {
  node_t *node = new_node();
  node->kind = NODE_NUM;
  node->val = expect_int();
  return node;
}

node_t *parse() {
  node_t *node = new_node();

  // parse leading operator
  if (consume('-')) {
    node_t *follower = parse();
    node->kind = NODE_MINUS;
    node->rhs = follower;
  } else if (consume('(')) {
    node = parse();
    expect(')');
  } else {
    node = parse_int();
  }

  // parse following opertors
  if (consume('+')) {
    node_t *follower = parse();
    node_t *leader = node;
    node = new_node();
    node->kind = NODE_ADD;
    node->lhs = leader;
    node->rhs = follower;
  } else if (consume('-')) {
    node_t *follower = parse();
    node_t *leader = node;
    node = new_node();
    node->kind = NODE_SUB;
    node->lhs = leader;
    node->rhs = follower;
  }
  return node;
}

void print_node(node_t *node) {
  switch (node->kind) {
    case NODE_MINUS:
      fprintf(stderr, "(- ");
      print_node(node->rhs);
      fprintf(stderr, ")");
      break;
    case NODE_ADD:
      fprintf(stderr, "(+ ");
      print_node(node->lhs);
      fprintf(stderr, " ");
      print_node(node->rhs);
      fprintf(stderr, ")");
      break;
    case NODE_SUB:
      break;
    case NODE_MUL:
      break;
    case NODE_DIV:
      break;
    case NODE_NUM:
      fprintf(stderr, "%d", node->val);
      break;

    default:
      break;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("argc = %d\n", argc);
    return 1;
  }
  token = tokenize(argv[1]);
  assert(!at_eof());
  node_t *node = parse();
  print_node(node);
  fprintf(stderr, "\n");

  print_header();
  print_main_header();
  printf("\tli a0, %d\n", expect_int());
  while (!at_eof()) {
    if (consume('+')) {
      printf("\taddi a0, a0, %d\n", expect_int());
      continue;
    }

    expect('-');
    printf("\taddi a0, a0, -%d\n", expect_int());
  }

  printf("\tret\n");

  return 0;
}