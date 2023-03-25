#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) error("cannot open %s: %s", path, strerror(errno));

  // check file size
  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  // read file
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // "\n\0" is added to the end of the file
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int label_index = 0;

int gen_label_index() { return label_index++; }

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
  TK_TYPE,
  TK_STRING,
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
    error("not '%s', got '%s'\n", op, token->str);
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

token_t *consume_ident_or_fail() {
  if (token->kind != TK_IDENT) {
    return NULL;
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

void push_token(token_t *tok) {
  if (tok->next != token) {
    error("failed to push token");
  }
  token = tok;
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
  token_t *cur = &head;
  size_t str_len = 0;
  char *q;
  head.next = NULL;

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
        *p == '<' || *p == '(' || *p == ')' || *p == '[' || *p == ']' ||
        *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' ||
        *p == '&') {
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
      } else if (strncmp(cur->str, "int", 3) == 0) {
        cur->kind = TK_TYPE;
      } else if (strncmp(cur->str, "char", 4) == 0) {
        cur->kind = TK_TYPE;
      } else if (strncmp(cur->str, "size_t", 6) == 0) {
        cur->kind = TK_TYPE;
      } else if (strncmp(cur->str, "void", 4) == 0) {
        cur->kind = TK_TYPE;
      }

      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_INT, cur, p, 0);
      cur->num = strtol(p, &p, 10);
      continue;
    }

    if (*p == '\'') {
      p++;
      if (*p == '\\') {
        p++;
        cur = new_token(TK_INT, cur, p, 0);
        switch (*p) {
          case 'a':
            cur->num = '\a';
            break;
          case 'b':
            cur->num = '\b';
            break;
          case 'f':
            cur->num = '\f';
            break;
          case 'n':
            cur->num = '\n';
            break;
          case 't':
            cur->num = '\t';
            break;
          case '\\':
            cur->num = '\\';
            break;
          default:
            error("failed to tokenize at '%c'\n'\\?...", *p);
        }
      } else {
        cur = new_token(TK_INT, cur, p, 0);
        cur->num = *p;
      }
      p++;
      if (*p != '\'') {
        error("failed to tokenize at '%c'\n'x?...", *p);
      }
      p++;
      continue;
    }

    if (*p == '"') {
      q = p;
      p++;
      str_len = 1;
      while (*p != '"') {
        str_len++;
        p++;
      }
      p++;
      str_len++;  // includeing '"'s
      cur = new_token(TK_STRING, cur, q, str_len);
      continue;
    }

    error("failed to tokenize at '%c'\n", *p);
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

typedef enum type_kind_t {
  TYPE_INVALID,
  TYPE_VOID,
  TYPE_INT,
  TYPE_CHAR,
  TYPE_POITNER,
  TYPE_ARRAY,
  TYPE_FUNCTION,
} type_kind_t;

#define MAX_ARGS 8
typedef struct type_t {
  type_kind_t ty;
  struct type_t *ptr_to;
  int n;  // for TYPE_ARRAY and TYPE_FUNCTION(# of parameters)

  // TYPE_FUNCTION
  struct type_t *ret;  // return type
  struct type_t *args[MAX_ARGS];
  token_t *arg_names[MAX_ARGS];
  size_t arg_count;
} type_t;

type_t *new_type() { return calloc(1, sizeof(type_t)); }

type_t *new_type_with(type_kind_t tk, type_t *ptr) {
  type_t *t = calloc(1, sizeof(type_t));
  t->ty = tk;
  t->ptr_to = ptr;
  return t;
}

void print_type(type_t *t) {
  switch (t->ty) {
    case TYPE_VOID:
      fprintf(stderr, "void");
      break;
    case TYPE_INT:
      fprintf(stderr, "int");
      break;
    case TYPE_CHAR:
      fprintf(stderr, "char");
      break;
    case TYPE_POITNER:
      fprintf(stderr, "*");
      print_type(t->ptr_to);
      break;
    case TYPE_ARRAY:
      fprintf(stderr, "[%d]", t->n);
      print_type(t->ptr_to);
      break;
    case TYPE_INVALID:
      error("tried to print TYPE_INVALID!");
      break;
    case TYPE_FUNCTION:
      for (int i = 0; i < t->n; i++) {
        if (0 < i) {
          fprintf(stderr, "->");
        }
        print_type(t->args[i]);
      }
      fprintf(stderr, "->");
      print_type(t->ret);
      break;
    default:
      error("tried to print %d!", t->ty);
      break;
  }
}

size_t calc_size_of_type(type_t *t) {
  if (t->ty == TYPE_INT) {
    return 4;
  } else if (t->ty == TYPE_CHAR) {
    return 1;
  } else if (t->ty == TYPE_POITNER) {
    return 4;
  } else if (t->ty == TYPE_ARRAY) {
    return t->n * calc_size_of_type(t->ptr_to);
  } else if (t->ty == TYPE_FUNCTION) {
    return 4;
  }
  error("calc_size_of_type: invalid data type: %d", t->ty);
  return 0;
}

typedef struct type_and_name_t {
  type_t *t;
  char *name;
  size_t len;
} type_and_name_t;

type_and_name_t *parse_type_and_name() {
  type_and_name_t *a = NULL;
  token_t *tok;
  type_t *tmp;
  if ((tok = consume_reserved(TK_TYPE))) {
    a = calloc(sizeof(type_and_name_t), 1);
    a->t = new_type();
    if (tok->len == 3 && memcmp(tok->str, "int", 3) == 0) {
      a->t->ty = TYPE_INT;
    } else if (tok->len == 4 && memcmp(tok->str, "char", 4) == 0) {
      a->t->ty = TYPE_CHAR;
    } else {
      error("unknown type: %s", tok->str);
    }

    while (!(tok = consume_ident_or_fail())) {
      consume("*");
      a->t = new_type_with(TYPE_POITNER, a->t);
    }

    a->name = tok->str;
    a->len = tok->len;

    if (consume("[")) {
      a->t = new_type_with(TYPE_ARRAY, a->t);
      a->t->n = expect_int();
      expect("]");
    } else if (consume("(")) {
      // function
      tmp = a->t;
      a->t = new_type();
      a->t->ret = tmp;
      a->t->ty = TYPE_FUNCTION;
      for (size_t i = 0; i < MAX_ARGS; i++) {
        if (consume(")")) {
          break;
        }
        if (0 < i) {
          if (!consume(",")) {
            error("call f(x y)? needs comma?\n");
          }
        }
        if (!consume_reserved(TK_TYPE)) {
          error("expected TK_TYPE in function's parameters");
        }
        tok = consume_ident();
        a->t->args[i] = new_type();
        a->t->args[i]->ty = TYPE_INT;
        a->t->arg_names[i] = tok;
        a->t->arg_count = i + 1;
      }
    }
  }
  return a;
}

typedef enum {
  NODE_INVALID,
  NODE_VAR_DEC,
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
  NODE_LOCAL_VARIABLE,
  NODE_GLOBAL_VARIABLE,
  NODE_RETURN,
  NODE_IF,
  NODE_WHILE,
  NODE_FOR,
  NODE_BLOCK,
  NODE_CALL,
  NODE_ADDR,
  NODE_DEREF,
  // NODE_INDEX, // a[i] -> *(a + i)
} node_kind_t;

#define MAX_STATEMENTS 1024
typedef struct node_t {
  node_kind_t kind;
  struct node_t *lhs;
  struct node_t *rhs;
  int val;      // for NODE_NUM
  int offset;   // for NODE_LOCAL_VARIABLE, from fp
  bool ignore;  // if true, then pop(ignore) the value
  type_t *type;

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

  // for 'call'
  struct node_t *args[MAX_ARGS];
  size_t args_count;
} node_t;

typedef struct local_variable_t {
  struct local_variable_t *next;
  char *name;  // var's name
  size_t len;  // var name's length
  size_t size;
  int offset;  // from fp
  type_t *type;
} local_variable_t;

local_variable_t *local_variables = NULL;

local_variable_t *find_local_variable(token_t *tok) {
  for (local_variable_t *var = local_variables; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

size_t calc_total_local_variable_size(local_variable_t *var) {
  size_t size = 0;
  while (var) {
    size += var->size;
    var = var->next;
  }
  return size;
}

void add_local_variable(char *name, size_t len, type_t *ty) {
  local_variable_t *lvar = calloc(1, sizeof(local_variable_t));
  lvar->next = local_variables;
  lvar->name = name;
  lvar->len = len;
  lvar->type = ty;
  lvar->size = calc_size_of_type(ty);
  lvar->offset = calc_total_local_variable_size(local_variables);
  local_variables = lvar;
}
typedef struct constant_string_t {
  struct constant_string_t *next;
  token_t *tok;
  size_t id;
} constant_string_t;

constant_string_t *constant_string;
size_t constant_string_count = 1;

constant_string_t *add_constant_string(token_t *tok) {
  constant_string_t *s = calloc(1, sizeof(constant_string_t));
  s->next = constant_string;
  s->tok = tok;
  s->id = constant_string_count;
  constant_string_count++;
  constant_string = s;
  return s;
}
typedef struct global_variable_t {
  struct global_variable_t *next;
  char *name;  // var's name
  size_t len;  // var name's length
  size_t size;
  type_t *type;

  constant_string_t *constant_string;  // for string
} global_variable_t;

// global variable
global_variable_t *global_variables = NULL;

global_variable_t *find_global_variable(token_t *tok) {
  for (global_variable_t *var = global_variables; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

void add_global_variable(char *name, size_t len, type_t *ty) {
  global_variable_t *var = calloc(1, sizeof(global_variable_t));
  var->next = global_variables;
  var->name = name;
  var->len = len;
  var->type = ty;
  var->size = calc_size_of_type(ty);
  global_variables = var;
}

constant_string_t *add_global_variable_with_constant_string(char *name,
                                                            size_t len,
                                                            type_t *ty,
                                                            token_t *tok) {
  global_variable_t *var = calloc(1, sizeof(global_variable_t));
  var->constant_string = add_constant_string(tok);
  var->next = global_variables;
  var->name = name;
  var->len = len;
  var->type = ty;
  var->size = calc_size_of_type(ty);
  global_variables = var;
  return var->constant_string;
}

typedef enum {
  DECLARATION_FUNCTION,
  DECLARATION_GLOBAL_VARIABLE,
} declaration_type_t;

typedef struct declaration_t {
  declaration_type_t declaration_type;
  char *name;
  size_t name_length;
  type_t *type;

  token_t *func_arg[MAX_ARGS];
  size_t func_arg_count;

  node_t *func_statements[MAX_STATEMENTS];
  size_t func_statement_count;

  constant_string_t *constant_string;
} declaration_t;

declaration_t *new_declaration() {
  declaration_t *d = calloc(1, sizeof(declaration_t));
  return d;
}

node_t *new_node() { return (node_t *)calloc(1, sizeof(node_t)); }

node_t *parse_int() {
  node_t *node = new_node();
  node->kind = NODE_NUM;
  node->val = expect_int();
  node->type = new_type_with(TYPE_INT, NULL);
  return node;
}

// binding power
// high is prior
const int INDEX_LEFT_BINDING_POW = 201;
const int NEG_RIGHT_BIND_POW = 151;
const int ADDR_RIGHT_BIND_POW = 151;
const int DEREF_RIGHT_BIND_POW = 151;
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
  } else if (consume("&")) {
    node_t *follower = parse_exp(ADDR_RIGHT_BIND_POW);
    node->kind = NODE_ADDR;
    node->rhs = follower;
  } else if (consume("*")) {
    node_t *follower = parse_exp(DEREF_RIGHT_BIND_POW);
    node->kind = NODE_DEREF;
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
      for (size_t i = 0; i < MAX_ARGS; i++) {
        if (consume(")")) {
          break;
        }
        if (0 < i) {
          if (!consume(",")) {
            error("call f(x y)? needs comma?\n");
          }
        }
        node->args[i] = parse_exp(0);
        node->args_count = i + 1;
      }
    } else {
      // variable
      local_variable_t *lvar = find_local_variable(tok);
      global_variable_t *gvar = find_global_variable(tok);
      if (lvar) {
        node->kind = NODE_LOCAL_VARIABLE;
        node->offset = lvar->offset;
        node->type = lvar->type;
      } else if (gvar) {
        node->kind = NODE_GLOBAL_VARIABLE;
        node->type = gvar->type;
        node->name = gvar->name;
        node->len = gvar->len;
      } else {
        error("%s not declared", tok->str);
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
    } else if (peek("[")) {
      if (INDEX_LEFT_BINDING_POW <= min_bind_pow) {
        return node;
      }
      node_t *deref = new_node();
      deref->kind = NODE_DEREF;
      deref->rhs = parse_follower(node, "[", 0, NODE_ADD);
      node = deref;
      expect("]");

    } else {
      return node;
    }
  }
}

node_t *parse_stmt() {
  node_t *node = new_node();
  type_and_name_t *type_and_name = parse_type_and_name();

  if (type_and_name) {
    node->kind = NODE_VAR_DEC;
    node->name = type_and_name->name;
    node->len = type_and_name->len;

    add_local_variable(node->name, node->len, type_and_name->t);

    expect(";");
  } else if (consume_reserved(TK_RETURN)) {
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
      node->init->ignore = true;
      expect(";");
    }
    if (!consume(";")) {
      node->cond = parse_exp(0);
      expect(";");
    }
    if (!consume(")")) {
      node->next = parse_exp(0);
      node->next->ignore = true;
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

void add_type(node_t *node) {
  switch (node->kind) {
    case NODE_NUM:
      node->type = new_type_with(TYPE_INT, NULL);
      break;
    case NODE_ADD:
    case NODE_SUB:
      add_type(node->lhs);
      add_type(node->rhs);
      if (node->lhs->type->ty == TYPE_POITNER) {
        node->type = node->lhs->type;
      } else if (node->rhs->type->ty == TYPE_POITNER) {
        node->type = node->rhs->type;
      } else {
        node->type = node->lhs->type;
      }
      break;
    case NODE_MUL:
    case NODE_DIV:
    case NODE_LT:
    case NODE_LE:
    case NODE_GT:
    case NODE_GE:
    case NODE_EQ:
    case NODE_NEQ:
    case NODE_ASSIGN:
      add_type(node->lhs);
      add_type(node->rhs);
      node->type = node->lhs->type;
      break;
    case NODE_RETURN:
      add_type(node->rhs);
      node->type = new_type_with(TYPE_VOID, NULL);
      break;
    case NODE_IF:
    case NODE_WHILE:
    case NODE_FOR:
      if (node->init) {
        add_type(node->init);  // only for NODE_FOR
      }
      add_type(node->cond);
      add_type(node->clause_then);
      if (node->clause_else) {
        add_type(node->clause_else);  // only for NODE_IF
      }
      node->type = new_type_with(TYPE_VOID, NULL);
      break;
    case NODE_BLOCK:
      for (size_t i = 0; i < node->statement_count; i++) {
        add_type(node->statements[i]);
      }
      node->type = new_type_with(TYPE_VOID, NULL);
      break;
    case NODE_LOCAL_VARIABLE:
    case NODE_GLOBAL_VARIABLE:
      // typed in parsing
      if (node->type == NULL) {
        error("type is not set");
      }
      break;
    case NODE_CALL:
      for (int i = 0; i < node->args_count; i++) {
        add_type(node->args[i]);
      }
      node->type = new_type_with(TYPE_VOID, NULL);
      break;
    case NODE_ADDR:
      add_type(node->rhs);
      node->type = new_type_with(TYPE_POITNER, node->rhs->type);
      break;
    case NODE_DEREF:
      add_type(node->rhs);
      node->type = node->rhs->type->ptr_to;
      break;
    case NODE_VAR_DEC:
      break;  // var declaration does not have type
    default:
      error("in add_type, unknown node kind: %d", node->kind);
  }
}

declaration_t *parse_declaration() {
  declaration_t *d = new_declaration();
  type_and_name_t *type_and_name = parse_type_and_name();
  token_t *tok;

  if (consume(";")) {
    d->declaration_type = DECLARATION_GLOBAL_VARIABLE;
    d->type = type_and_name->t;
    d->name = type_and_name->name;
    d->name_length = type_and_name->len;
    add_global_variable(type_and_name->name, type_and_name->len,
                        type_and_name->t);
    return d;
  }

  if (consume("=")) {
    d->declaration_type = DECLARATION_GLOBAL_VARIABLE;
    d->type = type_and_name->t;
    d->name = type_and_name->name;
    d->name_length = type_and_name->len;
    tok = consume_reserved(TK_STRING);
    if (!tok) {
      error("not supported yet");
    }
    d->constant_string = add_global_variable_with_constant_string(
        type_and_name->name, type_and_name->len, type_and_name->t, tok);
    expect(";");
    return d;
  }

  if (!consume("{")) {
    error("failed to parse declaration: '{' expected");
  }

  assert(type_and_name->t->ty == TYPE_FUNCTION);
  d->declaration_type = DECLARATION_FUNCTION;
  d->type = type_and_name->t;
  d->func_arg_count = type_and_name->t->arg_count;
  d->name = type_and_name->name;
  d->name_length = type_and_name->len;

  for (size_t i = 0; i < d->func_arg_count; i++) {
    d->func_arg[i] = type_and_name->t->arg_names[i];
    add_local_variable(d->func_arg[i]->str, d->func_arg[i]->len,
                       type_and_name->t->args[i]);
  }

  for (size_t i = 0; i < MAX_STATEMENTS; i++) {
    if (consume("}")) {
      break;
    }
    d->func_statements[i] = parse_stmt();
    add_type(d->func_statements[i]);
    d->func_statement_count = i + 1;
  }

  return d;
}

void print_str_len(FILE *fp, char *str, size_t len) {
  char *name = calloc(len + 1, 1);
  memcpy(name, str, len);
  fprintf(fp, "%s", name);
  free(name);
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
    case NODE_LOCAL_VARIABLE:
    case NODE_GLOBAL_VARIABLE: {
      print_str_len(stderr, node->name, node->len);
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
      print_str_len(stderr, node->name, node->len);
      fprintf(stderr, "(");
      for (size_t i = 0; i < node->args_count; i++) {
        if (0 < i) {
          fprintf(stderr, ", ");
        }
        print_node(node->args[i]);
      }
      fprintf(stderr, ")");
      break;
    }
    case NODE_ADDR:
      fprintf(stderr, "&");
      print_node(node->rhs);
      break;
    case NODE_DEREF:
      fprintf(stderr, "*");
      print_node(node->rhs);
      break;
    case NODE_VAR_DEC:
      fprintf(stderr, "int ");
      print_str_len(stderr, node->name, node->len);
      fprintf(stderr, ";\n");
      break;
    default:
      fprintf(stderr, "unimplemented printer: %d\n", node->kind);
      assert(!"unimplemented printer");
      break;
  }
}

void print_declaration(declaration_t *dec) {
  print_type(dec->type);
  print_str_len(stderr, dec->name, dec->name_length);
  if (dec->declaration_type == DECLARATION_GLOBAL_VARIABLE) {
    fprintf(stderr, ";\n");
  } else if (dec->declaration_type == DECLARATION_FUNCTION) {
    fprintf(stderr, "(");
    for (size_t i = 0; i < dec->func_arg_count; i++) {
      if (0 < i) {
        fprintf(stderr, ", ");
      }
      print_str_len(stderr, dec->func_arg[i]->str, dec->func_arg[i]->len);
    }
    fprintf(stderr, ") {\n");
    for (size_t i = 0; i < dec->func_statement_count; i++) {
      print_node(dec->func_statements[i]);
      fprintf(stderr, ";\n");
    }
    fprintf(stderr, "}\n");
  } else {
    error("failed to print declaration! type: %d", dec->declaration_type);
  }
}

char indent[1024];
int depth;

void update_indent() {
  for (int i = 0; i < 4 * depth; i += 4) {
    indent[i] = indent[i + 1] = indent[i + 2] = indent[i + 3] = ' ';
  }
  indent[4 * depth] = '\0';
}
void inc_depth() {
  depth++;
  update_indent();
}
void dec_depth() {
  depth--;
  update_indent();
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

void gen(node_t *node);

void gen_lval(node_t *node) {
  if (node->kind == NODE_LOCAL_VARIABLE) {
    // local variable address
    printf("%saddi t0, fp, %d\t\t# local variable: ", indent, node->offset);
    print_str_len(stdout, node->name, node->len);
    printf("\n");
    gen_push("t0");
  } else if (node->kind == NODE_GLOBAL_VARIABLE) {
    // global variable address
    printf("%slui t0, %%hi(", indent);
    print_str_len(stdout, node->name, node->len);
    printf(")\n");
    printf("%saddi t0, t0, %%lo(", indent);
    print_str_len(stdout, node->name, node->len);
    printf(")\n");
    gen_push("t0");
  } else if (node->kind == NODE_DEREF) {
    gen(node->rhs);
  } else {
    error("左辺値が左辺値ではない！ kind=%d", node->kind);
  }
}

void gen_alloc_stack(local_variable_t *lvar) {
  size_t bytes = calc_total_local_variable_size(lvar);
  fprintf(stderr, "stack alloc %zd\n", bytes);
  printf("%saddi sp, sp, -%zd\n", indent, bytes);
}

void gen_free_stack(local_variable_t *lvar) {
  size_t bytes = calc_total_local_variable_size(lvar);
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
    case NODE_VAR_DEC:
      // do nothing
      break;
    case NODE_ADD:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");  // rhs
      gen_pop("t1");  // lhs
      if (node->lhs->type->ty == TYPE_POITNER ||
          node->lhs->type->ty == TYPE_ARRAY) {
        printf("%sslli t0, t0, 2\n", indent);  // rhs * 4
      } else if (node->rhs->type->ty == TYPE_POITNER ||
                 node->rhs->type->ty == TYPE_ARRAY) {
        printf("%sslli t1, t1, 2\n", indent);  // lhs * 4
      }
      printf("%sadd t0, t1, t0\n", indent);
      gen_push("t0");
      break;
    case NODE_SUB:
      gen(node->lhs);
      gen(node->rhs);
      gen_pop("t0");
      gen_pop("t1");
      if (node->lhs->type->ty == TYPE_POITNER ||
          node->lhs->type->ty == TYPE_ARRAY) {
        printf("%sslli t0, t0, 2\n", indent);  // rhs * 4
      } else if (node->rhs->type->ty == TYPE_POITNER ||
                 node->rhs->type->ty == TYPE_ARRAY) {
        printf("%sslli t1, t1, 2\n", indent);  // lhs * 4
      }
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
    case NODE_LOCAL_VARIABLE:
      gen_lval(node);
      if (node->type->ty != TYPE_ARRAY) {
        gen_pop("t0");
        printf("%slw t0, 0(t0)\n", indent);
        gen_push("t0");
      }
      break;
    case NODE_GLOBAL_VARIABLE:
      gen_lval(node);
      if (node->type->ty != TYPE_ARRAY) {
        gen_pop("t0");
        printf("%slw t0, 0(t0)\n", indent);
        gen_push("t0");
      }
      break;
    case NODE_ASSIGN:
      gen(node->rhs);
      gen_lval(node->lhs);
      gen_pop("t1");  // address
      gen_pop("t0");  // value
      printf("%ssw t0, 0(t1)\n", indent);
      gen_push("t0");  // value again
      break;
    case NODE_RETURN:
      gen(node->rhs);
      gen_pop("a0");
      gen_free_stack(local_variables);
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
      } else {
        printf("%s# for init: empty\n", indent);
      }
      printf(".Lfor%d:\n", for_index);
      if (node->cond) {
        printf("%s# for cond\n", indent);
        gen(node->cond);
        gen_pop("t0");
        printf("%sbeqz t0, .Lfor_end%d\n", indent, for_index);
      } else {
        printf("%s# for cond: empty\n", indent);
      }
      printf("%s# for body\n", indent);
      gen(node->clause_then);
      if (node->next) {
        printf("%s# for next\n", indent);
        gen(node->next);
      } else {
        printf("%s# for next: empty\n", indent);
      }
      printf("%sj .Lfor%d\n", indent, for_index);
      printf(".Lfor_end%d: # for end\n", for_index);
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

      for (int i = 0; i < node->args_count; i++) {
        gen(node->args[node->args_count - 1 - i]);
      }
      for (int i = 0; i < node->args_count; i++) {
        char s[3] = "a0";
        s[1] = 48 + i;
        gen_pop(s);
      }

      // stack aligned 16
      gen_push("ra");
      gen_push("s1");
      printf("%sandi s1, sp, 0xF\n", indent);  // s1 = SP & 0xF
      printf("%ssub  sp, sp, s1\n", indent);   // align SP
      printf("%scall %s\n", indent, name);
      printf("%sadd  sp, sp, s1\n", indent);  // recover SP
      gen_pop("s1");
      gen_pop("ra");
      gen_push("a0");
      break;
    }
    case NODE_ADDR:
      gen_lval(node->rhs);
      break;
    case NODE_DEREF:
      gen(node->rhs);
      gen_pop("t0");
      printf("%slw t0, 0(t0)\n", indent);
      gen_push("t0");
      break;
    default:
      error("gen invalid node, kind=%d", node->kind);
  }
  if (node->ignore) {
    gen_pop("zero");
  }
  dec_depth();
}

void print_func_prologue(declaration_t *dec) {
  printf("  .text\n");
  printf("  .align 4\n");
  printf("  .globl    ");
  print_str_len(stdout, dec->name, dec->name_length);
  printf("\n");
  printf("  .type	    ");
  print_str_len(stdout, dec->name, dec->name_length);
  printf(", @function\n");
  print_str_len(stdout, dec->name, dec->name_length);
  printf(":\n");
  gen_push("fp");  // save fp

  gen_alloc_stack(local_variables);
  printf("%smv   fp, sp\n", indent);  // update fp

  // push arguments
  for (size_t i = 0; i < dec->func_arg_count; i++) {
    char reg[] = "a0";
    local_variable_t *var = find_local_variable(dec->func_arg[i]);
    reg[1] += i;
    fprintf(stderr, "push arg %s\n", reg);
    printf("%ssw %s, %d(fp)\n", indent, reg, var->offset);
  }
}

void print_func_epilogue(declaration_t *dec) {}

void gen_declaration(declaration_t *dec) {
  depth = 1;
  switch (dec->declaration_type) {
    case DECLARATION_GLOBAL_VARIABLE:
      printf("  .globl  ");
      print_str_len(stdout, dec->name, dec->name_length);
      printf("\n");
      printf("  .section  .sdata,\"aw\"\n");
      printf("  .type     ");
      print_str_len(stdout, dec->name, dec->name_length);
      printf(", @object\n");

      printf("  .size     ");
      print_str_len(stdout, dec->name, dec->name_length);
      printf(", %zd\n", calc_size_of_type(dec->type));

      print_str_len(stdout, dec->name, dec->name_length);
      printf(":\n");
      if (dec->constant_string) {
        printf("  .word .LC%zd", dec->constant_string->id);
      } else {
        printf("  .zero %zd\n", calc_size_of_type(dec->type));
      }
      printf("\n\n");
      break;
    case DECLARATION_FUNCTION:
      update_indent();
      print_func_prologue(dec);
      for (size_t i = 0; i < dec->func_statement_count; i++) {
        gen(dec->func_statements[i]);
      }
      print_func_epilogue(dec);
      break;
    default:
      error("unreachable! invalid declaration");
  }
}

void print_header() {
  printf("  .file	\"main.c\"\n");
  printf("  .option nopic\n");
  printf("  .align  4\n");
  printf("\n");
}

void print_constant_strings() {
  constant_string_t *cur = constant_string;
  while (cur) {
    printf(".LC%zd:\n", cur->id);
    printf("  .string ");
    print_str_len(stdout, cur->tok->str, cur->tok->len);
    printf("\n");
    cur = cur->next;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("argc = %d\n", argc);
    return 1;
  }

  token = tokenize(read_file(argv[1]));
  assert(!at_eof());

  print_header();
  while (!at_eof()) {
    declaration_t *dec = parse_declaration();
    print_declaration(dec);
    gen_declaration(dec);
  }
  print_constant_strings();

  return 0;
}