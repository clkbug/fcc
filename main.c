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

void print_main_prologue() {
  printf("  .globl  main\n");
  printf("  .type	main, @function\n");
  printf("main:\n");
  printf("  addi sp, sp, -4\n");
  printf("  sw   fp, 0(sp)\n");  // save fp
  printf("  addi sp, sp, -104\n");
  printf("  mv   fp, sp\n");  // update fp
}

void print_main_epilogue() {
  // pop
  printf("  lw a0, 0(sp)\n");
  printf("  addi sp, sp, 4\n");

  // sp
  printf("  addi sp, sp, 104\n");
  printf("  lw   fp, 0(sp)\n");
  printf("  addi sp, sp, 4\n");
  // ret
  printf("  ret\n");
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

typedef enum {
  TK_INVALID,
  TK_RESERVED,
  TK_RETURN,
  TK_IDENT,
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

bool is_int() { return token->kind == TK_INT; }

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error("not '%c', got '%s'\n", op, token->str);
  }
  token = token->next;
}

int expect_int() {
  if (token->kind != TK_INT) {
    char *s = calloc(token->len + 1, sizeof(char));
    memcpy(s, token->str, token->len);
    error("'%s' is not int\n", s);
  }
  int val = token->num;
  token = token->next;
  return val;
}

token_t *consume_ident() {
  if (token->kind != TK_IDENT) {
    char *s = calloc(token->len + 1, sizeof(char));
    memcpy(s, token->str, token->len);
    error("'%s' is not ident\n", s);
  }
  token_t *tok = token;
  token = token->next;
  return tok;
}

token_t *consume_reserved(token_kind_t kind) {
  if (token->kind != kind) {
    return NULL;
  }
  token_t *tok = token;
  token = token->next;
  return tok;
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
        *p == '<' || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if (isalpha(*p)) {
      int n = 1;
      while (isalnum(*(p + n)) || *(p + n) == '_') {
        n++;
      }
      cur = new_token(TK_IDENT, cur, p, n);
      p += n;

      if (strncmp(cur->str, "return", 6) == 0) {
        cur->kind = TK_RETURN;
      }

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
  NODE_INVALID,
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
  NODE_ASSIGN,
  NODE_LVAR,
  NODE_RETURN,
} node_kind_t;

typedef struct node_t {
  node_kind_t kind;
  struct node_t *lhs;
  struct node_t *rhs;
  int val;     // for NODE_NUM
  int offset;  // for NODE_LVAR, from fp
} node_t;

typedef struct lvar_t {
  struct lvar_t *next;
  char *name;  // var's name
  size_t len;  // var name's length
  int offset;  // from fp
} lvar_t;

lvar_t *locals;

lvar_t *find_lvar(token_t *tok) {
  for (lvar_t *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

node_t *new_node() { return (node_t *)calloc(1, sizeof(node_t)); }

node_t *parse_int() {
  node_t *node = new_node();
  node->kind = NODE_NUM;
  node->val = expect_int();
  return node;
}

// binding power
// high is prior
const int NEG_RIGHT_BIND_POW = 151;
const int MUL_LEFT_BINDING_POWER = 130;
const int MUL_RIGHT_BINDING_POWER = 131;
const int DIV_LEFT_BINDING_POWER = 130;
const int DIV_RIGHT_BINDING_POWER = 131;
const int PLUS_LEFT_BINDING_POWER = 120;
const int PLUS_RIGHT_BINDING_POWER = 121;
const int MINUS_LEFT_BINDING_POWER = 120;
const int MINUS_RIGHT_BINDING_POWER = 121;

const int COMPARE_LEFT_BINDING_POWER = 100;
const int COMPARE_RIGHT_BINDING_POWER = 101;
const int EQ_LEFT_BINDING_POWER = 90;
const int EQ_RIGHT_BINDING_POWER = 91;

const int ASSIGN_LEFT_BINDING_POWER = 21;
const int ASSIGN_RIGHT_BINDING_POWER = 20;

node_t *parse_exp(int min_bind_pow);

node_t *parse_follower(node_t *leader, char *op, int right_binding_power,
                       node_kind_t kind) {
  consume(op);
  node_t *follower = parse_exp(right_binding_power);
  node_t *node = new_node();
  node->kind = kind;
  node->lhs = leader;
  node->rhs = follower;
  return node;
}

node_t *parse_exp(int min_bind_pow) {
  node_t *node = new_node();

  // parse leading operator
  if (consume("-")) {
    node_t *follower = parse_exp(NEG_RIGHT_BIND_POW);
    node->kind = NODE_MINUS;
    node->rhs = follower;
  } else if (consume("(")) {
    node = parse_exp(0);
    expect(")");
  } else if (is_int()) {
    node = parse_int();
  } else {
    token_t *tok = consume_ident();
    node->kind = NODE_LVAR;
    lvar_t *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(lvar_t));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + 8 : 0;
      node->offset = lvar->offset;
      locals = lvar;
    }
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
    } else if (peek("=")) {
      if (ASSIGN_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "=", ASSIGN_RIGHT_BINDING_POWER, NODE_ASSIGN);
    } else {
      return node;
    }
  }
}

node_t *parse_stmt() {
  node_t *node = new_node();
  if (consume_reserved(TK_RETURN)) {
    node->kind = NODE_RETURN;
    node->rhs = parse_exp(0);
    expect(";");
    return node;
  }
  node = parse_exp(0);
  expect(";");
  return node;
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
    case NODE_LVAR:
      fprintf(stderr, "%c", node->offset / 4 + 'a');
      break;
    case NODE_ASSIGN:
      print_node_binop(node, "=");
      break;
    case NODE_RETURN:
      fprintf(stderr, "return ");
      print_node(node->rhs);
      break;
    default:
      fprintf(stderr, "unimplemented printer: %d\n", node->kind);
      assert(!"unimplemented printer");
      break;
  }
}

void gen_pop(char *dst) {
  printf("  lw %s, 0(sp)\n", dst);
  printf("  addi sp, sp, +4\n");
}

void gen_push(char *src) {
  printf("  addi sp, sp, -4\n");
  printf("  sw %s, 0(sp)\n", src);
}

void gen_lval(node_t *node) {
  if (node->kind != NODE_LVAR) {
    error("左辺値が左辺値ではない！ kind=%d", node->kind);
  }
  printf("  addi t0, fp, %d\n", node->offset);
  gen_push("t0");
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

  switch (node->kind) {
    case NODE_ADD:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  add t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_SUB:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  sub t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_MUL:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  mul t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_DIV:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  div t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_LT:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  slt t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_LE:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  slt t2, t1, t0\n");  // t2 <- t1 < t0
      printf("  sub t3, t0, t1\n");  // t3 <- t0 - t1
      printf("  snez t3, t3\n");     // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("  neg  t3, t3\n");     // t3 <- a == b -> 0, a != b -> -1
      printf("  addi t3, t3, 1\n");  // t3 <- a == b -> 1, a != b -> 0
      printf("  or   t0, t2, t3\n");
      gen_push("t0");
      break;
    case NODE_GT:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  sgt t0, t1, t0\n");
      gen_push("t0");
      break;
    case NODE_GE:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  slt t2, t0, t1\n");  // t2 <- t0 < t1
      printf("  sub t3, t1, t0\n");  // t3 <- t1 - t0
      printf("  snez t3, t3\n");     // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("  neg  t3, t3\n");     // t3 <- a == b -> 0, a != b -> -1
      printf("  addi t3, t3, 1\n");  // t3 <- a == b -> 1, a != b -> 0
      printf("  or   t0, t2, t3\n");
      gen_push("t0");
      break;
    case NODE_EQ:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  slt t2, t1, t0\n");  // a < b
      printf("  slt t3, t0, t1\n");  // a > b
      printf("  or  t1, t2, t3\n");  // (a < b) | (a > b) : a==b-> 0, a!=b->1
      printf("  li  t0, 1\n");
      printf("  sub t0, t0, t1\n");
      gen_push("t0");
      break;
    case NODE_NEQ:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("  sub t0, t1, t0\n");
      printf("  snez t0, t0\n");
      gen_push("t0");
      break;
    case NODE_LVAR:
      gen_lval(node);
      gen_pop("t0");
      printf("  lw t0, 0(t0)\n");
      gen_push("t0");
      break;
    case NODE_ASSIGN:
      gen(node->rhs);
      assert(node->lhs->kind == NODE_LVAR);
      gen_lval(node->lhs);
      gen_pop("t1");  // address
      gen_pop("t0");  // value
      printf("  sw t0, 0(t1)\n");
      gen_push("t0");  // value again
      break;
    case NODE_RETURN:
      gen(node->rhs);
      gen_pop("a0");
      printf("  ret\n");
      break;
    default:
      assert(!"gen invalid node");
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("argc = %d\n", argc);
    return 1;
  }
  token = tokenize(argv[1]);
  assert(!at_eof());

  print_header();
  print_main_prologue();

  while (!at_eof()) {
    node_t *node = parse_stmt();
    print_node(node);
    fprintf(stderr, "\n");
    gen(node);
  }

  print_main_epilogue();

  return 0;
}