#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_header() {
  printf("  .file	\"main.c\"\n");
  printf("  .option nopic\n");
  printf("  .text\n");
  printf("  .section  .rodata\n");
  printf("  .align  4\n");
  printf("\n");
}

void print_main_header() {
  printf("  .globl  main\n");
  printf("  .type	main, @function\n");
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
  size_t len;  // TK_RESERVED length
  int num;
} token_t;

token_t *token;

bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

bool peek(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  return true;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
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

token_t *new_token(token_kind_t kind, token_t *cur, char *str, size_t len) {
  token_t *tok = calloc(1, sizeof(token_t));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
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

    if (2 <= strlen(p)) {
      if (memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0 ||
          memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0) {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '>' ||
        *p == '<' || *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_INT, cur, p, 0);
      cur->num = strtol(p, &p, 10);
      continue;
    }

    error("failed to tokenize at '%c'\n", *p);
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

typedef enum {
  NODE_MINUS,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
  NODE_EQ,
  NODE_NEQ,
  NODE_LT,
  NODE_LE,
  NODE_GT,
  NODE_GE,
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

// binding power
// high is prior
const int NEG_RIGHT_BIND_POW = 91;
const int MUL_LEFT_BINDING_POWER = 80;
const int MUL_RIGHT_BINDING_POWER = 81;
const int DIV_LEFT_BINDING_POWER = 80;
const int DIV_RIGHT_BINDING_POWER = 81;
const int PLUS_LEFT_BINDING_POWER = 50;
const int PLUS_RIGHT_BINDING_POWER = 51;
const int MINUS_LEFT_BINDING_POWER = 50;
const int MINUS_RIGHT_BINDING_POWER = 51;

const int COMPARE_LEFT_BINDING_POWER = 30;
const int COMPARE_RIGHT_BINDING_POWER = 31;
const int EQ_LEFT_BINDING_POWER = 20;
const int EQ_RIGHT_BINDING_POWER = 21;

node_t *parse(int min_bind_pow);

node_t *parse_follower(node_t *leader, char *op, int right_binding_power,
                       node_kind_t kind) {
  consume(op);
  node_t *follower = parse(right_binding_power);
  node_t *node = new_node();
  node->kind = kind;
  node->lhs = leader;
  node->rhs = follower;
  return node;
}

node_t *parse(int min_bind_pow) {
  node_t *node = new_node();

  // parse leading operator
  if (consume("-")) {
    node_t *follower = parse(NEG_RIGHT_BIND_POW);
    node->kind = NODE_MINUS;
    node->rhs = follower;
  } else if (consume("(")) {
    node = parse(0);
    expect(")");
  } else {
    node = parse_int();
  }

  // parse following opertors
  for (;;) {
    if (peek("+")) {
      if (PLUS_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "+", PLUS_RIGHT_BINDING_POWER, NODE_ADD);
    } else if (peek("-")) {
      if (MINUS_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "-", MINUS_RIGHT_BINDING_POWER, NODE_SUB);
    } else if (peek("*")) {
      if (MUL_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "*", MUL_RIGHT_BINDING_POWER, NODE_MUL);
    } else if (peek("/")) {
      if (DIV_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "/", DIV_RIGHT_BINDING_POWER, NODE_DIV);
    } else if (peek("<")) {
      if (COMPARE_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "<", COMPARE_RIGHT_BINDING_POWER, NODE_LT);
    } else if (peek("<=")) {
      if (COMPARE_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "<=", COMPARE_RIGHT_BINDING_POWER, NODE_LE);
    } else if (peek(">")) {
      if (COMPARE_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, ">", COMPARE_RIGHT_BINDING_POWER, NODE_GT);
    } else if (peek(">=")) {
      if (COMPARE_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, ">=", COMPARE_RIGHT_BINDING_POWER, NODE_GE);
    } else if (peek("==")) {
      if (EQ_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "==", EQ_RIGHT_BINDING_POWER, NODE_EQ);
    } else if (peek("!=")) {
      if (EQ_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "!=", EQ_RIGHT_BINDING_POWER, NODE_NEQ);
    } else {
      return node;
    }
  }
}

void print_node(node_t *node);

void print_node_binop(node_t *node, char *op) {
  fprintf(stderr, "(%s ", op);
  print_node(node->lhs);
  fprintf(stderr, " ");
  print_node(node->rhs);
  fprintf(stderr, ")");
}

void print_node(node_t *node) {
  switch (node->kind) {
    case NODE_MINUS:
      fprintf(stderr, "(- ");
      print_node(node->rhs);
      fprintf(stderr, ")");
      break;
    case NODE_ADD:
      print_node_binop(node, "+");
      break;
    case NODE_SUB:
      print_node_binop(node, "-");
      break;
    case NODE_MUL:
      print_node_binop(node, "*");
      break;
    case NODE_DIV:
      print_node_binop(node, "/");
      break;
    case NODE_LT:
      print_node_binop(node, "<");
      break;
    case NODE_LE:
      print_node_binop(node, "<=");
      break;
    case NODE_GT:
      print_node_binop(node, ">");
      break;
    case NODE_GE:
      print_node_binop(node, ">=");
      break;
    case NODE_EQ:
      print_node_binop(node, "==");
      break;
    case NODE_NEQ:
      print_node_binop(node, "!=");
      break;
    case NODE_NUM:
      fprintf(stderr, "%d", node->val);
      break;

    default:
      assert(!"unimplemented printer");
      break;
  }
}

void gen(node_t *node) {
  // print_node(node);
  // fprintf(stderr, "\n");
  if (node->kind == NODE_NUM) {
    // push
    printf("  addi sp, sp, -4\n");
    printf("  li t0, %d\n", node->val);
    printf("  sw t0, 0(sp)\n");
    return;
  }
  if (node->lhs != NULL) {
    gen(node->lhs);
  }
  if (node->rhs != NULL) {
    gen(node->rhs);
  }

  // pop x2
  printf("  lw t0, 0(sp)\n");   // rhs
  printf("  lw t1, +4(sp)\n");  // lhs
  printf("  addi sp, sp, +8\n");

  switch (node->kind) {
    case NODE_ADD:
      printf("  add t0, t1, t0\n");
      break;
    case NODE_SUB:
      printf("  sub t0, t1, t0\n");
      break;
    case NODE_MUL:
      printf("  mul t0, t1, t0\n");
      break;
    case NODE_DIV:
      printf("  div t0, t1, t0\n");
      break;
    case NODE_LT:
      printf("  slt t0, t1, t0\n");
      break;
    case NODE_LE:
      printf("  slt t2, t1, t0\n");  // t2 <- t1 < t0
      printf("  sub t3, t0, t1\n");  // t3 <- t0 - t1
      printf("  snez t3, t3\n");     // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("  neg  t3, t3\n");     // t3 <- a == b -> 0, a != b -> -1
      printf("  addi t3, t3, 1\n");  // t3 <- a == b -> 1, a != b -> 0
      printf("  or   t0, t2, t3\n");
      break;
    case NODE_GT:
      printf("  sgt t0, t1, t0\n");
      break;
    case NODE_GE:
      printf("  slt t2, t0, t1\n");  // t2 <- t0 < t1
      printf("  sub t3, t1, t0\n");  // t3 <- t1 - t0
      printf("  snez t3, t3\n");     // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("  neg  t3, t3\n");     // t3 <- a == b -> 0, a != b -> -1
      printf("  addi t3, t3, 1\n");  // t3 <- a == b -> 1, a != b -> 0
      printf("  or   t0, t2, t3\n");
      break;
    case NODE_EQ:
      printf("  slt t2, t1, t0\n");  // a < b
      printf("  slt t3, t0, t1\n");  // a > b
      printf("  or  t1, t2, t3\n");  // (a < b) | (a > b) : a==b-> 0, a!=b->1
      printf("  li  t0, 1\n");
      printf("  sub t0, t0, t1\n");
      break;
    case NODE_NEQ:
      printf("  sub t0, t1, t0\n");
      printf("  snez t0, t0\n");
      break;
    default:
      assert(!"gen invalid node");
  }
  printf("  addi sp, sp, -4\n");
  printf("  sw t0, 0(sp)\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("argc = %d\n", argc);
    return 1;
  }
  token = tokenize(argv[1]);
  assert(!at_eof());
  node_t *node = parse(0);
  assert(at_eof());

  print_node(node);
  fprintf(stderr, "\n");

  print_header();
  print_main_header();

  gen(node);

  // pop
  printf("  lw a0, 0(sp)\n");
  printf("  addi sp, sp, 4\n");

  // ret
  printf("  ret\n");

  return 0;
}