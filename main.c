#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int label_index = 0;

int gen_label_index() { return label_index++; }

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
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
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
        *p == '<' || *p == '(' || *p == ')' || *p == '=' || *p == ';' ||
        *p == '{' || *p == '}') {
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
      } else if (strncmp(cur->str, "if", 2) == 0) {
        cur->kind = TK_IF;
      } else if (strncmp(cur->str, "else", 4) == 0) {
        cur->kind = TK_ELSE;
      } else if (strncmp(cur->str, "while", 5) == 0) {
        cur->kind = TK_WHILE;
      } else if (strncmp(cur->str, "for", 3) == 0) {
        cur->kind = TK_FOR;
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
  NODE_IF,
  NODE_WHILE,
  NODE_FOR,
  NODE_BLOCK,
  NODE_CALL,
} node_kind_t;

#define MAX_STATEMENTS 1024
typedef struct node_t {
  node_kind_t kind;
  struct node_t *lhs;
  struct node_t *rhs;
  int val;      // for NODE_NUM
  int offset;   // for NODE_LVAR, from fp
  bool ignore;  // if true, then pop(ignore) the value

  // for variable
  char *name;
  int len;

  // for 'if'/'while'
  struct node_t *cond;
  struct node_t *clause_then;
  struct node_t *clause_else;  // only 'if'

  // for 'for'
  struct node_t *init;
  // struct node_t *cond;
  struct node_t *next;
  // struct node_t *clause_then;

  // for 'block'
  struct node_t *statements[MAX_STATEMENTS];
  size_t statement_count;
} node_t;

typedef struct lvar_t {
  struct lvar_t *next;
  char *name;  // var's name
  size_t len;  // var name's length
  size_t size;
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

size_t calc_total_lvar_size(lvar_t *var) {
  size_t size = 0;
  while (var) {
    size += var->size;
    var = var->next;
  }
  return size;
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
    if (consume("(")) {
      // function call
      node->kind = NODE_CALL;
      node->name = tok->str;
      node->len = tok->len;
      expect(")");
    } else {
      // variable
      node->kind = NODE_LVAR;
      lvar_t *lvar = find_lvar(tok);
      if (lvar) {
        node->offset = lvar->offset;
      } else {
        lvar = calloc(1, sizeof(lvar_t));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->size = 4;
        lvar->offset = calc_total_lvar_size(locals);
        node->offset = lvar->offset;
        locals = lvar;
      }
      node->name = tok->str;
      node->len = tok->len;
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
  } else if (consume_reserved(TK_IF)) {
    node->kind = NODE_IF;
    expect("(");
    node->cond = parse_exp(0);
    expect(")");
    node->clause_then = parse_stmt();
    if (consume_reserved(TK_ELSE)) {
      node->clause_else = parse_stmt();
    }
  } else if (consume_reserved(TK_WHILE)) {
    node->kind = NODE_WHILE;
    expect("(");
    node->cond = parse_exp(0);
    expect(")");
    node->clause_then = parse_stmt();
  } else if (consume_reserved(TK_FOR)) {
    node->kind = NODE_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = parse_exp(0);
      expect(";");
    }
    if (!consume(";")) {
      node->cond = parse_exp(0);
      expect(";");
    }
    if (!consume(")")) {
      node->next = parse_exp(0);
      expect(")");
    }
    node->clause_then = parse_stmt();
  } else if (consume("{")) {
    node->kind = NODE_BLOCK;
    for (size_t i = 0; i < MAX_STATEMENTS && !consume("}"); i++) {
      node->statements[i] = parse_stmt();
      node->statement_count++;
    }
  } else {
    node = parse_exp(0);
    node->ignore = true;
    expect(";");
  }
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
  if (node->ignore) fprintf(stderr, "[ignore]");
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
    case NODE_LVAR: {
      char *name = calloc(node->len + 1, 1);
      memcpy(name, node->name, node->len);
      fprintf(stderr, "%s", name);
      break;
    }
    case NODE_ASSIGN:
      print_node_binop(node, "=");
      break;
    case NODE_RETURN:
      fprintf(stderr, "return ");
      print_node(node->rhs);
      fprintf(stderr, ";");
      break;
    case NODE_IF:
      fprintf(stderr, "if (");
      print_node(node->cond);
      fprintf(stderr, ") ");
      print_node(node->clause_then);
      if (node->clause_else) {
        fprintf(stderr, " else ");
        print_node(node->clause_else);
      }
      break;
    case NODE_WHILE:
      fprintf(stderr, "while (");
      print_node(node->cond);
      fprintf(stderr, ") ");
      print_node(node->clause_then);
      break;
    case NODE_FOR:
      fprintf(stderr, "for (");
      if (node->init) print_node(node->init);
      fprintf(stderr, "; ");
      if (node->cond) print_node(node->cond);
      fprintf(stderr, "; ");
      if (node->next) print_node(node->next);
      fprintf(stderr, ") ");
      print_node(node->clause_then);
      break;
    case NODE_BLOCK:
      fprintf(stderr, "{ ");
      for (size_t i = 0; i < node->statement_count; i++) {
        print_node(node->statements[i]);
      }
      fprintf(stderr, "}");
      break;
    case NODE_CALL: {
      char *name = calloc(node->len + 1, 1);
      memcpy(name, node->name, node->len);
      fprintf(stderr, "%s()", name);
      break;
    }
    default:
      fprintf(stderr, "unimplemented printer: %d\n", node->kind);
      assert(!"unimplemented printer");
      break;
  }
}

char indent[1024];
int depth = 0;

void update_depth() {
  for (int i = 0; i < 4 * depth; i += 4) {
    indent[i] = indent[i + 1] = indent[i + 2] = indent[i + 3] = ' ';
  }
  indent[4 * depth] = '\0';
}
void inc_depth() {
  depth++;
  update_depth();
}
void dec_depth() {
  depth--;
  update_depth();
}
void print_indent() { printf("%s", indent); }

void gen_pop(char *dst) {
  printf("%slw %s, 0(sp)          # pop\n", indent, dst);
  printf("%saddi sp, sp, +4       # ___\n", indent);
}

void gen_push(char *src) {
  printf("%saddi sp, sp, -4       # push\n", indent);
  printf("%ssw %s, 0(sp)          # ____\n", indent, src);
}

void gen_lval(node_t *node) {
  if (node->kind != NODE_LVAR) {
    error("左辺値が左辺値ではない！ kind=%d", node->kind);
  }
  printf("%saddi t0, fp, %d\n", indent, node->offset);
  gen_push("t0");
}

void gen_alloc_stack(lvar_t *lvar) {
  size_t bytes = calc_total_lvar_size(lvar);
  fprintf(stderr, "stack alloc %zd\n", bytes);
  printf("%saddi sp, sp, -%zd\n", indent, bytes);
}

void gen_free_stack(lvar_t *lvar) {
  size_t bytes = calc_total_lvar_size(lvar);
  fprintf(stderr, "stack free %zd\n", bytes);
  printf("%saddi sp, sp, %zd\n", indent, bytes);
}

void gen(node_t *node) {
  inc_depth();
  print_node(node);
  fprintf(stderr, "\n");
  if (node->kind == NODE_NUM) {
    // push
    printf("%sli t0, %d\n", indent, node->val);
    gen_push("t0");
    dec_depth();
    return;
  }

  switch (node->kind) {
    case NODE_ADD:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sadd t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_SUB:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%ssub t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_MUL:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%smul t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_DIV:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sdiv t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_LT:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sslt t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_LE:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sslt t2, t1, t0\n", indent);  // t2 <- t1 < t0
      printf("%ssub t3, t0, t1\n", indent);  // t3 <- t0 - t1
      printf("%ssnez t3, t3\n",
             indent);  // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("%sneg  t3, t3\n", indent);     // t3 <- a == b -> 0, a != b -> -1
      printf("%saddi t3, t3, 1\n", indent);  // t3 <- a == b -> 1, a != b -> 0
      printf("%sor   t0, t2, t3\n", indent);
      gen_push("t0");
      break;
    case NODE_GT:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%ssgt t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_GE:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sslt t2, t0, t1\n", indent);  // t2 <- t0 < t1
      printf("%ssub t3, t1, t0\n", indent);  // t3 <- t1 - t0
      printf("%ssnez t3, t3\n",
             indent);  // t3 <- t3 != 0 : a == b -> 0, a != b -> 1
      printf("%sneg  t3, t3\n", indent);     // t3 <- a == b -> 0, a != b -> -1
      printf("%saddi t3, t3, 1\n", indent);  // t3 <- a == b -> 1, a != b -> 0
      printf("%sor   t0, t2, t3\n", indent);
      gen_push("t0");
      break;
    case NODE_EQ:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%sslt t2, t1, t0\n", indent);  // a < b
      printf("%sslt t3, t0, t1\n", indent);  // a > b
      printf("%sor  t1, t2, t3\n",
             indent);  // (a < b) | (a > b) : a==b-> 0, a!=b->1
      printf("%sli  t0, 1\n", indent);
      printf("%ssub t0, t0, t1\n", indent);
      gen_push("t0");
      break;
    case NODE_NEQ:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      printf("%ssub t0, t1, t0\n", indent);
      printf("%ssnez t0, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_LVAR:
      gen_lval(node);
      gen_pop("t0");
      printf("%slw t0, 0(t0)\n", indent);
      gen_push("t0");
      break;
    case NODE_ASSIGN:
      gen(node->rhs);
      assert(node->lhs->kind == NODE_LVAR);
      gen_lval(node->lhs);
      gen_pop("t1");  // address
      gen_pop("t0");  // value
      printf("%ssw t0, 0(t1)\n", indent);
      gen_push("t0");  // value again
      break;
    case NODE_RETURN:
      gen(node->rhs);
      gen_pop("a0");
      gen_free_stack(locals);
      gen_pop("fp");
      printf("%sret\n", indent);
      break;
    case NODE_IF:
      gen(node->cond);
      gen_pop("t0");
      int end_index = gen_label_index();
      int else_index = gen_label_index();
      printf("%sbeqz t0, .Lelse%d\n", indent, else_index);
      gen(node->clause_then);
      printf("%sj .Lifend%d\n", indent, end_index);
      printf(".Lelse%d:\n", else_index);
      if (node->clause_else) {
        gen(node->clause_else);
      }
      printf(".Lifend%d:\n", end_index);
      break;
    case NODE_WHILE: {
      int while_index = gen_label_index();
      int while_end_index = gen_label_index();
      printf(".Lwhile%d:\n", while_index);
      gen(node->cond);
      gen_pop("t0");
      printf("%sbeqz t0, .Lwhile_end%d\n", indent, while_end_index);
      gen(node->clause_then);
      printf("%sj .Lwhile%d\n", indent, while_index);
      printf(".Lwhile_end%d:\n", while_end_index);
      break;
    }
    case NODE_FOR: {
      printf("%s# for start\n", indent);
      int for_index = gen_label_index();
      if (node->init) {
        printf("%s# for init\n", indent);
        gen(node->init);
      }
      gen_pop("zero");
      printf(".Lfor%d:\n", for_index);
      if (node->cond) {
        printf("%s# for cond\n", indent);
        gen(node->cond);
        gen_pop("t0");
        printf("%sbeqz t0, .Lfor_end%d\n", indent, for_index);
      }
      printf("%s# for body\n", indent);
      gen(node->clause_then);
      if (node->next) {
        gen(node->next);
        gen_pop("zero");
      }
      printf("%sj .Lfor%d\n", indent, for_index);
      printf(".Lfor_end%d:\n", for_index);
      printf("%s# for end\n", indent);
      break;
    }
    case NODE_BLOCK:
      for (size_t i = 0; i < node->statement_count; i++) {
        gen(node->statements[i]);
      }
      break;
    case NODE_CALL: {
      char *name = calloc(node->len + 1, 1);
      memcpy(name, node->name, node->len);
      printf("%scall %s\n", indent, name);
      gen_push("a0");
      break;
    }
    default:
      error("gen invalid node, kind=%d", node->kind);
  }
  if (node->ignore) {
    gen_pop("zero");
  }
  dec_depth();
}

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
  gen_push("fp");  // save fp
  gen_alloc_stack(locals);
  printf("  mv   fp, sp\n");  // update fp
}

void print_main_epilogue() {}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("argc = %d\n", argc);
    return 1;
  }
  token = tokenize(argv[1]);
  assert(!at_eof());

  node_t *nodes[1024];
  size_t i = 0;
  for (i = 0; i < 1024 && !at_eof(); i++) {
    nodes[i] = parse_stmt();
    print_node(nodes[i]);
    fprintf(stderr, "\n");
  }
  print_header();
  print_main_prologue();
  for (size_t j = 0; j < i; j++) {
    gen(nodes[j]);
  }

  print_main_epilogue();

  return 0;
}