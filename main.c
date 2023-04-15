#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int last_loop_label_index = 0;
int label_index = 0;

int gen_loop_label_index() {
  last_loop_label_index = label_index;
  ++label_index;
  return last_loop_label_index;
}

int gen_label_index() {
  int old_label_index = label_index;
  ++label_index;
  return old_label_index;
}

typedef int token_kind_t;

const token_kind_t TK_INVALID = 0;
const token_kind_t TK_RESERVED = 1;
const token_kind_t TK_RETURN = 2;
const token_kind_t TK_BREAK = 3;
const token_kind_t TK_CONTINUE = 4;
const token_kind_t TK_IF = 5;
const token_kind_t TK_ELSE = 6;
const token_kind_t TK_WHILE = 7;
const token_kind_t TK_FOR = 8;
const token_kind_t TK_IDENT = 9;
const token_kind_t TK_INT = 10;
const token_kind_t TK_TYPE_INT = 11;
const token_kind_t TK_TYPE_CHAR = 12;
const token_kind_t TK_TYPE_VOID = 13;
const token_kind_t TK_STRING = 14;
const token_kind_t TK_TYPEDEF = 15;
const token_kind_t TK_STRUCT = 16;
const token_kind_t TK_SIZEOF = 17;
const token_kind_t TK_EOF = 18;

struct token_t {
  token_kind_t kind;
  struct token_t *next;
  char *str;
  size_t len;  // TK_RESERVED length
  int num;
};

typedef struct token_t token_t;

token_t *token;

bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return 0;
  }
  token = token->next;
  return 1;
}

bool peek(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return 0;
  }
  return 1;
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

token_t *peek_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  return token;
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

token_t *consume_any_type() {
  if (token->kind == TK_TYPE_INT || token->kind == TK_TYPE_CHAR ||
      token->kind == TK_TYPE_VOID) {
    token_t *tok = token;
    token = token->next;
    return tok;
  }
  return NULL;
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

int compare_token(token_t *tok, char *str, size_t len) {
  return tok->len == len && strncmp(tok->str, str, len) == 0;
}

token_t *tokenize(char *p) {
  token_t head;
  token_t *cur = &head;
  size_t str_len = 0;
  char *q;
  head.next = NULL;

  while (*p) {
    if (isspace(*p)) {
      ++p;
      continue;
    }

    if (strncmp(p, "//", 2) == 0) {
      p = p + 2;
      while (*p != '\n') {
        ++p;
      }
      continue;
    }

    if (strncmp(p, "/*", 2) == 0) {
      char *q = strstr(p + 2, "*/");
      if (!q) {
        error("unclosed comment");
      }
      p = q + 2;
      continue;
    }

    // skip preprocessor
    if (*p == '#') {
      p = p + 1;
      while (*p != '\n') {
        ++p;
      }
      continue;
    }
    if (6 <= strlen(p) && strncmp(p, "const", 5) == 0 && isspace(p[5])) {
      p = p + 5;
      continue;
    }

    if (2 <= strlen(p)) {
      if (memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0 ||
          memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0 ||
          memcmp(p, "++", 2) == 0 || memcmp(p, "--", 2) == 0 ||
          memcmp(p, "->", 2) == 0 || memcmp(p, "||", 2) == 0 ||
          memcmp(p, "&&", 2) == 0) {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p = p + 2;
        continue;
      }
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' ||
        *p == '>' || *p == '<' || *p == '(' || *p == ')' || *p == '[' ||
        *p == ']' || *p == '=' || *p == ';' || *p == '{' || *p == '}' ||
        *p == ',' || *p == '&' || *p == '.' || *p == '|' || *p == '!' ||
        *p == '^') {
      cur = new_token(TK_RESERVED, cur, p, 1);
      ++p;
      continue;
    }

    if (isalpha(*p)) {
      int n = 1;
      while (isalnum(*(p + n)) || *(p + n) == '_') {
        ++n;
      }
      cur = new_token(TK_IDENT, cur, p, n);
      p = p + n;

      if (compare_token(cur, "return", 6)) {
        cur->kind = TK_RETURN;
      } else if (compare_token(cur, "if", 2)) {
        cur->kind = TK_IF;
      } else if (compare_token(cur, "else", 4)) {
        cur->kind = TK_ELSE;
      } else if (compare_token(cur, "while", 5)) {
        cur->kind = TK_WHILE;
      } else if (compare_token(cur, "for", 3)) {
        cur->kind = TK_FOR;
      } else if (compare_token(cur, "break", 5)) {
        cur->kind = TK_BREAK;
      } else if (compare_token(cur, "continue", 8)) {
        cur->kind = TK_CONTINUE;
      } else if (compare_token(cur, "int", 3)) {
        cur->kind = TK_TYPE_INT;
      } else if (compare_token(cur, "char", 4)) {
        cur->kind = TK_TYPE_CHAR;
      } else if (compare_token(cur, "size_t", 6)) {
        cur->kind = TK_TYPE_INT;
      } else if (compare_token(cur, "bool", 4)) {
        cur->kind = TK_TYPE_INT;
      } else if (compare_token(cur, "void", 4)) {
        cur->kind = TK_TYPE_VOID;
      } else if (compare_token(cur, "typedef", 7)) {
        cur->kind = TK_TYPEDEF;
      } else if (compare_token(cur, "struct", 6)) {
        cur->kind = TK_STRUCT;
      } else if (compare_token(cur, "sizeof", 6)) {
        cur->kind = TK_SIZEOF;
      } else if (compare_token(cur, "NULL", 4)) {
        cur->kind = TK_INT;
        cur->num = 0;
      }

      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_INT, cur, p, 0);
      cur->num = strtol(p, &p, 10);
      continue;
    }

    if (*p == '\'') {
      ++p;
      if (*p == '\\') {
        ++p;
        cur = new_token(TK_INT, cur, p, 0);
        if (*p == '0') {
          cur->num = '\0';
        } else if (*p == 'a') {
          cur->num = '\a';
        } else if (*p == 'b') {
          cur->num = '\b';
        } else if (*p == 'f') {
          cur->num = '\f';
        } else if (*p == 'n') {
          cur->num = '\n';
        } else if (*p == 't') {
          cur->num = '\t';
        } else if (*p == '\\') {
          cur->num = '\\';
        } else if (*p == '\'') {
          cur->num = '\'';
        } else {
          error("failed to tokenize at '%c'\n'\\?...", *p);
        }
      } else {
        cur = new_token(TK_INT, cur, p, 0);
        cur->num = *p;
      }
      ++p;
      if (*p != '\'') {
        error("failed to tokenize at '%c'\n'x?...", *p);
      }
      ++p;
      continue;
    }

    if (*p == '"') {
      q = p;
      ++p;
      str_len = 1;
      while (*p != '"') {
        if (*p == '\\') {
          ++str_len;
          ++p;
        }
        ++str_len;
        ++p;
      }
      ++p;
      ++str_len;  // includeing '"'s
      cur = new_token(TK_STRING, cur, q, str_len);
      continue;
    }

    error("failed to tokenize at '%c'\n", *p);
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

typedef int type_kind_t;

type_kind_t TYPE_INVALID = 0;
type_kind_t TYPE_VOID = 1;
type_kind_t TYPE_INT = 2;
type_kind_t TYPE_CHAR = 3;
type_kind_t TYPE_POINTER = 4;
type_kind_t TYPE_ARRAY = 5;
type_kind_t TYPE_FUNCTION = 6;
type_kind_t TYPE_STRUCT = 7;

#define MAX_ARGS 8
#define MAX_STRUCT_MEMBERS 32

struct type_struct_t {
  token_t *name;
  size_t member_count;
  token_t *member_names[MAX_STRUCT_MEMBERS];
  struct type_t *member_types[MAX_STRUCT_MEMBERS];
  size_t member_offsets[MAX_STRUCT_MEMBERS];
  struct type_struct_t *next;
};
typedef struct type_struct_t type_struct_t;

type_struct_t *type_struct = NULL;

type_struct_t *new_type_struct() { return calloc(1, sizeof(type_struct_t)); }

type_struct_t *find_type_struct(token_t *name) {
  type_struct_t *t = type_struct;
  while (t) {
    if (t->name->len == name->len &&
        memcmp(t->name->str, name->str, name->len) == 0) {
      return t;
    }
    t = t->next;
  }
  return NULL;
}

void add_type_struct(token_t *tok, type_struct_t *t) {
  t->next = type_struct;
  t->name = tok;
  type_struct = t;
}

size_t get_member_index(type_struct_t *s, token_t *name) {
  for (size_t i = 0; i < s->member_count; ++i) {
    if (s->member_names[i]->len == name->len &&
        memcmp(s->member_names[i]->str, name->str, name->len) == 0) {
      return i;
    }
  }
  error("failed to get member offset at '%.*s'", name->len, name->str);
  return 0;
}

struct type_t {
  type_kind_t ty;
  struct type_t *ptr_to;
  int n;  // for TYPE_ARRAY and TYPE_FUNCTION(# of parameters)

  // TYPE_FUNCTION
  struct type_t *ret;  // return type
  struct type_t *args[MAX_ARGS];
  token_t *arg_names[MAX_ARGS];
  size_t arg_count;

  // TYPE_STRUCT
  type_struct_t *struct_type;

  token_t *name;  // for TEMPORAL TYPE
};
typedef struct type_t type_t;

type_t *new_type() { return calloc(1, sizeof(type_t)); }

type_t *new_type_with(type_kind_t tk, type_t *ptr) {
  type_t *t = calloc(1, sizeof(type_t));
  t->ty = tk;
  t->ptr_to = ptr;
  return t;
}

void print_type(type_t *t) {
  if (t->ty == TYPE_VOID) {
    fprintf(stderr, "void");
  } else if (t->ty == TYPE_INT) {
    fprintf(stderr, "int");
  } else if (t->ty == TYPE_CHAR) {
    fprintf(stderr, "char");
  } else if (t->ty == TYPE_POINTER) {
    fprintf(stderr, "*");
    print_type(t->ptr_to);
  } else if (t->ty == TYPE_ARRAY) {
    fprintf(stderr, "[%d]", t->n);
    print_type(t->ptr_to);
  } else if (t->ty == TYPE_FUNCTION) {
    for (int i = 0; i < t->n; ++i) {
      if (0 < i) {
        fprintf(stderr, "->");
      }
      print_type(t->args[i]);
    }
    fprintf(stderr, "->");
    print_type(t->ret);
  } else if (t->ty == TYPE_INVALID) {
    error("tried to print TYPE_INVALID!");
  } else if (t->ty == TYPE_STRUCT) {
    fprintf(stderr, "struct xxx");
  } else {
    error("tried to print %d!", t->ty);
  }
}

size_t calc_size_of_type(type_t *t) {
  if (t->ty == TYPE_INT) {
    return 4;
  } else if (t->ty == TYPE_CHAR) {
    return 1;
  } else if (t->ty == TYPE_POINTER) {
    return 4;
  } else if (t->ty == TYPE_ARRAY) {
    return t->n * calc_size_of_type(t->ptr_to);
  } else if (t->ty == TYPE_FUNCTION) {
    return 4;
  } else if (t->ty == TYPE_STRUCT) {
    return t->struct_type->member_offsets[t->struct_type->member_count - 1] +
           calc_size_of_type(
               t->struct_type->member_types[t->struct_type->member_count - 1]);
  }
  error("calc_size_of_type: invalid data type: %d", t->ty);
  return 0;
}

size_t calc_size_of_struct(type_struct_t *s) {
  return s->member_offsets[s->member_count - 1] +
         calc_size_of_type(s->member_types[s->member_count - 1]);
}

struct type_alias_t {
  token_t *name;
  struct type_t *type;
  struct type_alias_t *next;
};
typedef struct type_alias_t type_alias_t;

type_alias_t *type_alias = NULL;

void add_type_alias(token_t *name, type_t *type) {
  type_alias_t *t = calloc(1, sizeof(type_alias_t));
  t->name = name;
  t->type = type;
  t->next = type_alias;
  type_alias = t;
}

type_t *find_type_alias(token_t *name) {
  type_alias_t *t = type_alias;
  while (t) {
    if (t->name->len == name->len &&
        memcmp(t->name->str, name->str, name->len) == 0) {
      return t->type;
    }
    t = t->next;
  }
  return NULL;
}

struct type_and_name_t {
  type_t *t;
  token_t *name;
};

typedef struct type_and_name_t type_and_name_t;

type_and_name_t *parse_type_and_name() {
  type_and_name_t *a = NULL;
  token_t *tok;
  type_t *tmp;
  size_t offset = 0;
  type_and_name_t *t;
  tok = consume_any_type();

  if (!tok) {
    tok = consume_reserved(TK_STRUCT);
  }
  if (!tok) {
    tok = peek_ident();
  }

  if (!tok) {
    return NULL;
  }

  a = calloc(sizeof(type_and_name_t), 1);
  if (tok->kind == TK_TYPE_INT) {
    a->t = new_type();
    a->t->ty = TYPE_INT;
  } else if (tok->kind == TK_TYPE_CHAR) {
    a->t = new_type();
    a->t->ty = TYPE_CHAR;
  } else if (tok->kind == TK_TYPE_VOID) {
    a->t = new_type();
    a->t->ty = TYPE_VOID;
  } else if (tok->kind == TK_STRUCT) {
    tok = consume_ident();
    a->t = new_type();
    a->t->ty = TYPE_STRUCT;
    a->t->struct_type = find_type_struct(tok);

    if (!a->t->struct_type) {
      // struct definition or opaque pointer
      if (consume("{")) {
        // struct definition
        a->t->struct_type = new_type_struct();
        while (1) {
          t = parse_type_and_name();
          if (!t) {
            break;
          }
          expect(";");
          a->t->struct_type->member_names[a->t->struct_type->member_count] =
              t->name;
          a->t->struct_type->member_types[a->t->struct_type->member_count] =
              t->t;

          a->t->struct_type->member_offsets[a->t->struct_type->member_count] =
              offset;
          offset =
              offset + calc_size_of_type(
                           a->t->struct_type
                               ->member_types[a->t->struct_type->member_count]);
          if (offset % 4 != 0) {
            offset = offset + 4 - (offset % 4);
          }
          ++a->t->struct_type->member_count;
        }
        // register struct type
        add_type_struct(tok, a->t->struct_type);

        expect("}");
        // expect(";");
        return a;
      } else {
        // should be pointer of struct
        a->t->struct_type = new_type_struct();
        a->t->name = tok;

        while (1) {
          if (consume("*")) {
            a->t = new_type_with(TYPE_POINTER, a->t);
          } else {
            break;
          }
        }
        a->name = consume_ident();
        // expect(";");

        if (consume("[")) {
          a->t = new_type_with(TYPE_ARRAY, a->t);
          a->t->n = expect_int();
          expect("]");
        }

        // function pointer in struct is not supported

        return a;
      }
    }
    // struct variable (may be function definition)
  } else {
    a->t = find_type_alias(tok);
    if (!a->t) return NULL;
    consume_ident();
  }

  while (!(tok = consume_ident_or_fail())) {
    consume("*");
    a->t = new_type_with(TYPE_POINTER, a->t);
  }

  a->name = tok;

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
    for (size_t i = 0; i < MAX_ARGS; ++i) {
      if (consume(")")) {
        break;
      }
      if (0 < i) {
        if (!consume(",")) {
          error("call f(x y)? needs comma?\n");
        }
      }
      if (consume_reserved(TK_STRUCT)) {
        tok = consume_ident();
        a->t->args[i] = new_type();
        a->t->args[i]->ty = TYPE_STRUCT;
        a->t->args[i]->struct_type = find_type_struct(tok);
      } else {
        tok = consume_any_type();
        if (!tok) {
          tok = consume_ident();
        }
        if (tok->kind == TK_TYPE_INT) {
          a->t->args[i] = new_type();
          a->t->args[i]->ty = TYPE_INT;
        } else if (tok->kind == TK_TYPE_CHAR) {
          a->t->args[i] = new_type();
          a->t->args[i]->ty = TYPE_CHAR;
        } else if (tok->kind == TK_TYPE_VOID) {
          a->t->args[i] = new_type();
          a->t->args[i]->ty = TYPE_VOID;
        } else {
          a->t->args[i] = find_type_alias(tok);
          if (!a->t) return NULL;
        }
      }

      while (!(tok = consume_ident_or_fail())) {
        assert(consume("*"));
        a->t->args[i] = new_type_with(TYPE_POINTER, a->t->args[i]);
      }

      // tok = consume_ident();
      a->t->arg_names[i] = tok;

      a->t->arg_count = i + 1;
    }
  }
  return a;
}

struct constant_string_t {
  struct constant_string_t *next;
  token_t *tok;
  size_t id;
};
typedef struct constant_string_t constant_string_t;

constant_string_t *constant_string;
size_t constant_string_count = 1;

constant_string_t *add_constant_string(token_t *tok) {
  constant_string_t *s = calloc(1, sizeof(constant_string_t));
  s->next = constant_string;
  s->tok = tok;
  s->id = constant_string_count;
  ++constant_string_count;
  constant_string = s;
  return s;
}

typedef int node_kind_t;
const node_kind_t NODE_INVALID = 0;
const node_kind_t NODE_VAR_DEC = 1;
const node_kind_t NODE_TYPEDEF = 2;
const node_kind_t NODE_MINUS = 3;
const node_kind_t NODE_ADD = 4;
const node_kind_t NODE_SUB = 5;
const node_kind_t NODE_MUL = 6;
const node_kind_t NODE_DIV = 7;
const node_kind_t NODE_MOD = 8;
const node_kind_t NODE_EQ = 9;
const node_kind_t NODE_NEQ = 10;
const node_kind_t NODE_LT = 11;
const node_kind_t NODE_LE = 12;
const node_kind_t NODE_GT = 13;
const node_kind_t NODE_GE = 14;
const node_kind_t NODE_LOGICAL_AND = 15;
const node_kind_t NODE_LOGICAL_OR = 16;
const node_kind_t NODE_LOGICAL_NOT = 17;
const node_kind_t NODE_BITWISE_AND = 18;
const node_kind_t NODE_BITWISE_OR = 19;
// NODE_BITWISE_NOT, // not implemented yet
const node_kind_t NODE_BITWISE_XOR = 20;
const node_kind_t NODE_NUM = 21;
const node_kind_t NODE_CONST_STRING = 22;
const node_kind_t NODE_ASSIGN = 23;
const node_kind_t NODE_LOCAL_VARIABLE = 24;
const node_kind_t NODE_GLOBAL_VARIABLE = 25;
const node_kind_t NODE_RETURN = 26;
const node_kind_t NODE_BREAK = 27;
const node_kind_t NODE_CONTINUE = 28;
const node_kind_t NODE_IF = 29;
const node_kind_t NODE_WHILE = 30;
const node_kind_t NODE_FOR = 31;
const node_kind_t NODE_BLOCK = 32;
const node_kind_t NODE_CALL = 33;
const node_kind_t NODE_ADDR = 34;
const node_kind_t NODE_DEREF = 35;
const node_kind_t NODE_STRUCT_MEMBER = 36;
const node_kind_t NODE_DOT = 37;
const node_kind_t NODE_ARROW = 38;
// NODE_INDEX, // a[i] -> *(a + i)

#define MAX_STATEMENTS 1024
struct node_t {
  node_kind_t kind;
  struct node_t *lhs;
  struct node_t *rhs;
  int val;      // for NODE_NUM
  int offset;   // for NODE_LOCAL_VARIABLE, from fp, or for NODE_STRUCT_MEMBER
  bool ignore;  // if 1, then pop(ignore) the value
  type_t *type;

  // for variable
  token_t *name;

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

  // for 'const string'
  constant_string_t *const_str;
};
typedef struct node_t node_t;

struct local_variable_t {
  struct local_variable_t *next;
  token_t *name;
  size_t size;
  size_t size_on_stack;  // aligned size
  int offset;            // from fp
  type_t *type;
};
typedef struct local_variable_t local_variable_t;

local_variable_t *local_variables = NULL;

local_variable_t *find_local_variable(token_t *name) {
  for (local_variable_t *var = local_variables; var; var = var->next) {
    if (var->name->len == name->len &&
        !memcmp(name->str, var->name->str, var->name->len)) {
      return var;
    }
  }
  return NULL;
}

size_t calc_total_local_variable_size_on_stack(local_variable_t *var) {
  size_t size = 0;
  while (var) {
    size = size + var->size_on_stack;
    var = var->next;
  }
  return size;
}

void add_local_variable(token_t *name, type_t *ty) {
  local_variable_t *lvar = calloc(1, sizeof(local_variable_t));
  lvar->next = local_variables;
  lvar->name = name;
  lvar->type = ty;
  lvar->size = calc_size_of_type(ty);
  lvar->size_on_stack = (lvar->size + 3) / 4 * 4;  // align 4
  lvar->offset = calc_total_local_variable_size_on_stack(local_variables);
  local_variables = lvar;
}

struct global_variable_t {
  struct global_variable_t *next;
  token_t *name;
  size_t size;
  type_t *type;

  constant_string_t *constant_string;  // for string
};
typedef struct global_variable_t global_variable_t;

// global variable
global_variable_t *global_variables = NULL;

global_variable_t *find_global_variable(token_t *tok) {
  for (global_variable_t *var = global_variables; var; var = var->next) {
    if (var->name->len == tok->len &&
        !memcmp(tok->str, var->name->str, var->name->len)) {
      return var;
    }
  }
  return NULL;
}

void add_global_variable(token_t *name, type_t *ty) {
  global_variable_t *var = calloc(1, sizeof(global_variable_t));
  var->next = global_variables;
  var->name = name;
  var->type = ty;
  var->size = calc_size_of_type(ty);
  global_variables = var;
}

constant_string_t *add_global_variable_with_constant_string(token_t *name,
                                                            type_t *ty,
                                                            token_t *tok) {
  global_variable_t *var = calloc(1, sizeof(global_variable_t));
  var->constant_string = add_constant_string(tok);
  var->next = global_variables;
  var->name = name;
  var->type = ty;
  var->size = calc_size_of_type(ty);
  global_variables = var;
  return var->constant_string;
}

typedef int declaration_type_t;
const declaration_type_t DECLARATION_INVALID = 0;
const declaration_type_t DECLARATION_FUNCTION = 1;
const declaration_type_t DECLARATION_GLOBAL_VARIABLE = 2;
const declaration_type_t DECLARATION_TYPEDEF = 3;
// DECLARATION_STRUCT,

struct declaration_t {
  declaration_type_t declaration_type;
  token_t *name;
  type_t *type;

  token_t *func_arg[MAX_ARGS];
  size_t func_arg_count;

  node_t *func_statements[MAX_STATEMENTS];
  size_t func_statement_count;

  constant_string_t *constant_string;
  int constant_int;
};
typedef struct declaration_t declaration_t;

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
const int INDEX_LEFT_BINDING_POW = 200;
const int POST_INC_LEFT_BINDING_POW = 200;

// . ->
const int MEMBER_ACCESS_LEFT_BINDING_POW = 200;
const int MEMBER_ACCESS_RIGHT_BINDING_POW = 201;

// ++/-- +/- & !
const int NEG_RIGHT_BIND_POW = 151;
const int ADDR_RIGHT_BIND_POW = 151;
const int DEREF_RIGHT_BIND_POW = 151;
const int PRE_INC_RIGHT_BIND_POW = 151;
const int LOGICAL_NOT_RIGHT_BIND_POW = 151;

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
const int BITWISE_AND_LEFT_BINDING_POWER = 80;
const int BITWISE_AND_RIGHT_BINDING_POWER = 81;
const int BITWISE_XOR_LEFT_BINDING_POWER = 70;
const int BITWISE_XOR_RIGHT_BINDING_POWER = 71;
const int BITWISE_OR_LEFT_BINDING_POWER = 60;
const int BITWISE_OR_RIGHT_BINDING_POWER = 61;

const int LOGICAL_AND_LEFT_BINDING_POWER = 50;
const int LOGICAL_AND_RIGHT_BINDING_POWER = 51;
const int LOGICAL_OR_LEFT_BINDING_POWER = 40;
const int LOGICAL_OR_RIGHT_BINDING_POWER = 41;

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
  token_t *tok;
  type_t *type;
  type_struct_t *struc;

  // parse leading operator
  if (consume("-")) {
    node_t *follower = parse_exp(NEG_RIGHT_BIND_POW);
    node->kind = NODE_MINUS;
    node->rhs = follower;
  } else if (consume("&")) {
    node_t *follower = parse_exp(ADDR_RIGHT_BIND_POW);
    node->kind = NODE_ADDR;
    node->rhs = follower;
  } else if (consume("!")) {
    node_t *follower = parse_exp(LOGICAL_NOT_RIGHT_BIND_POW);
    node->kind = NODE_LOGICAL_NOT;
    node->rhs = follower;
  } else if (consume("*")) {
    node_t *follower = parse_exp(DEREF_RIGHT_BIND_POW);
    node->kind = NODE_DEREF;
    node->rhs = follower;
  } else if (consume("++")) {
    node_t *follower = parse_exp(PRE_INC_RIGHT_BIND_POW);
    node->kind = NODE_ASSIGN;
    node->lhs = follower;
    node->rhs = new_node();
    node->rhs->kind = NODE_ADD;
    node->rhs->lhs = follower;
    node->rhs->rhs = new_node();
    node->rhs->rhs->kind = NODE_NUM;
    node->rhs->rhs->val = 1;
    node->rhs->rhs->type = new_type_with(TYPE_INT, NULL);
  } else if (consume("--")) {
    node_t *follower = parse_exp(PRE_INC_RIGHT_BIND_POW);
    node->kind = NODE_ASSIGN;
    node->lhs = follower;
    node->rhs = new_node();
    node->rhs->kind = NODE_SUB;
    node->rhs->lhs = follower;
    node->rhs->rhs = new_node();
    node->rhs->rhs->kind = NODE_NUM;
    node->rhs->rhs->val = 1;
    node->rhs->rhs->type = new_type_with(TYPE_INT, NULL);
  } else if (consume("(")) {
    node = parse_exp(0);
    expect(")");
  } else if (is_int()) {
    node = parse_int();
  } else if ((tok = consume_reserved(TK_STRING))) {
    node->kind = NODE_CONST_STRING;
    node->const_str = add_constant_string(tok);
  } else if ((tok = consume_reserved(TK_SIZEOF))) {
    expect("(");
    node->type = new_type_with(TYPE_INT, NULL);
    node->kind = NODE_NUM;
    if ((tok = consume_reserved(TK_TYPE_INT))) {
      node->val = 4;
    } else if ((tok = consume_reserved(TK_TYPE_CHAR))) {
      node->val = 1;
    } else if ((tok = consume_reserved(TK_STRUCT))) {
      tok = consume_ident();
      struc = find_type_struct(tok);
      node->val = calc_size_of_struct(struc);
    } else {
      tok = consume_ident();
      type = find_type_alias(tok);
      if (!type) {
        error("not found type_alias, sizeof");
      }
      node->val = calc_size_of_type(type);
    }
    expect(")");
  } else {
    tok = consume_ident();
    if (consume("(")) {
      // function call
      node->kind = NODE_CALL;
      node->name = tok;
      for (size_t i = 0; i < MAX_ARGS; ++i) {
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
      } else {
        // skip for now, because of struct member access
        node->kind = NODE_STRUCT_MEMBER;
        node->name = tok;
      }
      node->name = tok;
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
    } else if (peek("%")) {
      if (DIV_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "%", DIV_RIGHT_BINDING_POWER, NODE_MOD);
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
    } else if (peek("&")) {
      if (BITWISE_AND_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "&", BITWISE_AND_RIGHT_BINDING_POWER,
                            NODE_BITWISE_AND);
    } else if (peek("^")) {
      if (BITWISE_XOR_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "^", BITWISE_XOR_RIGHT_BINDING_POWER,
                            NODE_BITWISE_XOR);
    } else if (peek("|")) {
      if (BITWISE_OR_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "|", BITWISE_OR_RIGHT_BINDING_POWER,
                            NODE_BITWISE_OR);
    } else if (peek("&&")) {
      if (LOGICAL_AND_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "&&", LOGICAL_AND_RIGHT_BINDING_POWER,
                            NODE_LOGICAL_AND);
    } else if (peek("||")) {
      if (LOGICAL_OR_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "||", LOGICAL_OR_RIGHT_BINDING_POWER,
                            NODE_LOGICAL_OR);
    } else if (peek("=")) {
      if (ASSIGN_LEFT_BINDING_POWER <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "=", ASSIGN_RIGHT_BINDING_POWER, NODE_ASSIGN);
    } else if (peek(".")) {
      if (MEMBER_ACCESS_LEFT_BINDING_POW <= min_bind_pow) {
        return node;
      }
      node =
          parse_follower(node, ".", MEMBER_ACCESS_RIGHT_BINDING_POW, NODE_DOT);
    } else if (peek("->")) {
      if (MEMBER_ACCESS_LEFT_BINDING_POW <= min_bind_pow) {
        return node;
      }
      node = parse_follower(node, "->", MEMBER_ACCESS_RIGHT_BINDING_POW,
                            NODE_ARROW);
    } else if (peek("[")) {
      if (INDEX_LEFT_BINDING_POW <= min_bind_pow) {
        return node;
      }
      node_t *deref = new_node();
      deref->kind = NODE_DEREF;
      deref->rhs = parse_follower(node, "[", 0, NODE_ADD);
      node = deref;
      expect("]");
    } else if (peek("++") || peek("--")) {
      error("postfix ++/-- not supported yet");
      // if (POST_INC_LEFT_BINDING_POW <= min_bind_pow) {
      //   return node;
      // }
      // node_t *inc = new_node();
      // inc->kind = NODE_ASSIGN;
      // inc->lhs = node;
      // inc->rhs = new_node();
      // inc->rhs->kind = NODE_ADD;
      // inc->rhs->lhs = node;
      // inc->rhs->rhs = new_node();
      // inc->rhs->rhs->kind = NODE_NUM;
      // inc->rhs->rhs->val = 1;
      // inc->rhs->rhs->type = new_type_with(TYPE_INT, NULL);
      // node = parse_follower(inc, "++", 0, NODE_ASSIGN);
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

    add_local_variable(node->name, type_and_name->t);
    if (consume("=")) {
      local_variable_t *lvar = find_local_variable(node->name);
      node->lhs = new_node();
      node->lhs->kind = NODE_LOCAL_VARIABLE;
      node->lhs->name = lvar->name;
      node->lhs->offset = lvar->offset;
      node->lhs->type = lvar->type;
      node->type = node->lhs->type;
      node->rhs = parse_exp(0);
    }
    expect(";");
  } else if (consume_reserved(TK_RETURN)) {
    node->kind = NODE_RETURN;
    if (!consume(";")) {
      node->rhs = parse_exp(0);
      expect(";");
    }
  } else if (consume_reserved(TK_BREAK)) {
    node->kind = NODE_BREAK;
    expect(";");
  } else if (consume_reserved(TK_CONTINUE)) {
    node->kind = NODE_CONTINUE;
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
      node->init->ignore = 1;
      expect(";");
    }
    if (!consume(";")) {
      node->cond = parse_exp(0);
      expect(";");
    }
    if (!consume(")")) {
      node->next = parse_exp(0);
      node->next->ignore = 1;
      expect(")");
    }
    node->clause_then = parse_stmt();
  } else if (consume("{")) {
    node->kind = NODE_BLOCK;
    for (size_t i = 0; i < MAX_STATEMENTS && !consume("}"); ++i) {
      node->statements[i] = parse_stmt();
      ++node->statement_count;
    }
  } else {
    node = parse_exp(0);
    node->ignore = 1;
    expect(";");
  }
  return node;
}

void add_type(node_t *node) {
  size_t i;

  if (node->kind == NODE_NUM) {
    node->type = new_type_with(TYPE_INT, NULL);
  } else if (node->kind == NODE_CONST_STRING) {
    node->type = new_type_with(TYPE_POINTER, new_type_with(TYPE_CHAR, NULL));
  } else if (node->kind == NODE_MINUS) {
    add_type(node->rhs);
    node->type = node->rhs->type;
  } else if (node->kind == NODE_ADD || node->kind == NODE_SUB) {
    add_type(node->lhs);
    add_type(node->rhs);
    if (node->lhs->type->ty == TYPE_POINTER) {
      node->type = node->lhs->type;
    } else if (node->rhs->type->ty == TYPE_POINTER) {
      node->type = node->rhs->type;
    } else {
      node->type = node->lhs->type;
    }
  } else if (node->kind == NODE_MUL || node->kind == NODE_DIV ||
             node->kind == NODE_MOD || node->kind == NODE_LT ||
             node->kind == NODE_LE || node->kind == NODE_GT ||
             node->kind == NODE_GE || node->kind == NODE_EQ ||
             node->kind == NODE_NEQ || node->kind == NODE_ASSIGN ||
             node->kind == NODE_LOGICAL_AND || node->kind == NODE_LOGICAL_OR ||
             node->kind == NODE_BITWISE_AND || node->kind == NODE_BITWISE_OR ||
             node->kind == NODE_BITWISE_XOR) {
    add_type(node->lhs);
    add_type(node->rhs);
    node->type = node->lhs->type;
  } else if (node->kind == NODE_DOT) {
    add_type(node->lhs);
    assert(node->lhs->type->ty == TYPE_STRUCT);

    if (node->lhs->type->struct_type->member_count == 0) {
      node->lhs->type->struct_type = find_type_struct(node->lhs->type->name);
    }
    assert(node->lhs->type->struct_type->member_count > 0);
    // assert(node->rhs->kind == NODE_STRUCT_MEMBER);
    i = get_member_index(node->lhs->type->struct_type, node->rhs->name);
    node->rhs->kind = NODE_STRUCT_MEMBER;
    node->rhs->offset = node->lhs->type->struct_type->member_offsets[i];
    node->rhs->type = node->lhs->type->struct_type->member_types[i];
    node->type = node->rhs->type;
  } else if (node->kind == NODE_ARROW) {
    add_type(node->lhs);
    assert(node->lhs->type->ty == TYPE_POINTER);
    assert(node->lhs->type->ptr_to->ty == TYPE_STRUCT);

    if (node->lhs->type->ptr_to->struct_type->member_count == 0) {
      node->lhs->type->ptr_to->struct_type =
          find_type_struct(node->lhs->type->ptr_to->name);
    }
    assert(node->lhs->type->ptr_to->struct_type->member_count > 0);
    // assert(node->rhs->kind == NODE_STRUCT_MEMBER);
    i = get_member_index(node->lhs->type->ptr_to->struct_type, node->rhs->name);
    node->rhs->kind = NODE_STRUCT_MEMBER;
    node->rhs->offset = node->lhs->type->ptr_to->struct_type->member_offsets[i];
    node->rhs->type = node->lhs->type->ptr_to->struct_type->member_types[i];
    node->type = node->rhs->type;
  } else if (node->kind == NODE_RETURN) {
    if (node->rhs) {
      add_type(node->rhs);
    }
    node->type = new_type_with(TYPE_VOID, NULL);
  } else if (node->kind == NODE_BREAK || node->kind == NODE_CONTINUE) {
    node->type = new_type_with(TYPE_VOID, NULL);
  } else if (node->kind == NODE_IF || node->kind == NODE_WHILE ||
             node->kind == NODE_FOR) {
    if (node->init) {
      add_type(node->init);  // only for NODE_FOR
    }
    if (node->cond) {
      add_type(node->cond);
    }
    if (node->clause_then) {
      add_type(node->clause_then);
    }
    if (node->clause_else) {
      add_type(node->clause_else);  // only for NODE_IF
    }
    if (node->next) {
      add_type(node->next);
    }
    node->type = new_type_with(TYPE_VOID, NULL);
  } else if (node->kind == NODE_BLOCK) {
    for (size_t i = 0; i < node->statement_count; ++i) {
      add_type(node->statements[i]);
    }
    node->type = new_type_with(TYPE_VOID, NULL);
  } else if (node->kind == NODE_LOCAL_VARIABLE ||
             node->kind == NODE_GLOBAL_VARIABLE) {
    // typed in parsing
    if (node->type == NULL) {
      error("type is not set");
    }
  } else if (node->kind == NODE_CALL) {
    for (int i = 0; i < node->args_count; ++i) {
      add_type(node->args[i]);
    }
    node->type = new_type_with(TYPE_VOID, NULL);
  } else if (node->kind == NODE_ADDR) {
    add_type(node->rhs);
    node->type = new_type_with(TYPE_POINTER, node->rhs->type);
  } else if (node->kind == NODE_DEREF) {
    add_type(node->rhs);
    node->type = node->rhs->type->ptr_to;
  } else if (node->kind == NODE_LOGICAL_NOT) {
    add_type(node->rhs);
    node->type = node->rhs->type;
  } else if (node->kind == NODE_VAR_DEC) {
    assert((node->lhs && node->rhs) || (!node->lhs && !node->rhs));
    if (node->lhs) {
      add_type(node->lhs);
      node->type = node->lhs->type;
      if (!node->rhs) {
        error("node->lhs is not NULL but node->rhs is NULL");
      }
      add_type(node->rhs);
    }
    // var declaration does not have type
  } else {
    error("in add_type, unknown node kind: %d", node->kind);
  }
}

declaration_t *parse_declaration() {
  declaration_t *d = new_declaration();
  type_and_name_t *type_and_name;
  token_t *tok;

  if (consume_reserved(TK_TYPEDEF)) {
    d->declaration_type = DECLARATION_TYPEDEF;
    type_and_name = parse_type_and_name();
    d->type = type_and_name->t;
    d->name = type_and_name->name;
    add_type_alias(type_and_name->name, type_and_name->t);
    expect(";");
    return d;
  }

  type_and_name = parse_type_and_name();
  if (consume(";")) {
    if (type_and_name->name == NULL) {
      // struct definition
      return NULL;
    } else {
      d->declaration_type = DECLARATION_GLOBAL_VARIABLE;
      d->type = type_and_name->t;
      d->name = type_and_name->name;
      add_global_variable(type_and_name->name, type_and_name->t);
      return d;
    }
  }

  if (consume("=")) {
    d->declaration_type = DECLARATION_GLOBAL_VARIABLE;
    d->type = type_and_name->t;
    d->name = type_and_name->name;
    if ((tok = consume_reserved(TK_STRING))) {
      d->constant_string = add_global_variable_with_constant_string(
          type_and_name->name, type_and_name->t, tok);
    } else if ((tok = consume_reserved(TK_INT))) {
      d->constant_int = tok->num;
      add_global_variable(d->name, d->type);
    }
    expect(";");
    return d;
  }

  if (!consume("{")) {
    error("failed to parse declaration: '{' expected");
  }

  if (type_and_name->t->ty == TYPE_STRUCT &&
      !find_type_struct(type_and_name->name)) {
    type_struct_t *ts = new_type_struct();
    size_t offset = 0;
    size_t index = 0;

    ts->name = type_and_name->name;

    // struct definition
    while (!consume("}")) {
      type_and_name_t *tn = parse_type_and_name();
      ts->member_names[index] = tn->name;
      ts->member_types[index] = tn->t;
      ts->member_offsets[index] = offset;
      ts->member_count = index + 1;
      consume(";");
    }
    ts->next = type_struct;
    type_struct = ts;
    return NULL;
  }

  assert(type_and_name->t->ty == TYPE_FUNCTION);
  d->declaration_type = DECLARATION_FUNCTION;
  d->type = type_and_name->t;
  d->func_arg_count = type_and_name->t->arg_count;
  d->name = type_and_name->name;

  for (size_t i = 0; i < d->func_arg_count; ++i) {
    d->func_arg[i] = type_and_name->t->arg_names[i];
    add_local_variable(d->func_arg[i], type_and_name->t->args[i]);
  }

  for (size_t i = 0; i < MAX_STATEMENTS; ++i) {
    if (consume("}")) {
      break;
    }
    d->func_statements[i] = parse_stmt();
    add_type(d->func_statements[i]);
    d->func_statement_count = i + 1;
  }

  if (d->func_statements[d->func_statement_count - 1]->kind != NODE_RETURN) {
    d->func_statements[d->func_statement_count] = new_node();
    d->func_statements[d->func_statement_count]->kind = NODE_RETURN;
    ++d->func_statement_count;
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
  if (node->kind == NODE_MINUS) {
    fprintf(stderr, "(- ");
    print_node(node->rhs);
    fprintf(stderr, ")");
  } else if (node->kind == NODE_ADD) {
    print_node_binop(node, "+");
  } else if (node->kind == NODE_SUB) {
    print_node_binop(node, "-");
  } else if (node->kind == NODE_MUL) {
    print_node_binop(node, "*");
  } else if (node->kind == NODE_DIV) {
    print_node_binop(node, "/");
  } else if (node->kind == NODE_MOD) {
    print_node_binop(node, "%");
  } else if (node->kind == NODE_LT) {
    print_node_binop(node, "<");
  } else if (node->kind == NODE_LE) {
    print_node_binop(node, "<=");
  } else if (node->kind == NODE_GT) {
    print_node_binop(node, ">");
  } else if (node->kind == NODE_GE) {
    print_node_binop(node, ">=");
  } else if (node->kind == NODE_LOGICAL_AND) {
    print_node_binop(node, "&&");
  } else if (node->kind == NODE_LOGICAL_OR) {
    print_node_binop(node, "||");
  } else if (node->kind == NODE_EQ) {
    print_node_binop(node, "==");
  } else if (node->kind == NODE_NEQ) {
    print_node_binop(node, "!=");
  } else if (node->kind == NODE_BITWISE_AND) {
    print_node_binop(node, "&");
  } else if (node->kind == NODE_BITWISE_OR) {
    print_node_binop(node, "|");
  } else if (node->kind == NODE_BITWISE_XOR) {
    print_node_binop(node, "^");
  } else if (node->kind == NODE_DOT) {
    print_node_binop(node, ".");
  } else if (node->kind == NODE_ARROW) {
    print_node_binop(node, "->");
  } else if (node->kind == NODE_NUM) {
    fprintf(stderr, "%d", node->val);
  } else if (node->kind == NODE_CONST_STRING) {
    print_str_len(stderr, node->const_str->tok->str, node->const_str->tok->len);
  } else if (node->kind == NODE_LOCAL_VARIABLE ||
             node->kind == NODE_GLOBAL_VARIABLE ||
             node->kind == NODE_STRUCT_MEMBER) {
    print_str_len(stderr, node->name->str, node->name->len);
  } else if (node->kind == NODE_ASSIGN) {
    print_node_binop(node, "=");
  } else if (node->kind == NODE_RETURN) {
    fprintf(stderr, "return ");
    if (node->rhs) {
      print_node(node->rhs);
    }
    fprintf(stderr, ";");
  } else if (node->kind == NODE_BREAK) {
    fprintf(stderr, "break;");
  } else if (node->kind == NODE_CONTINUE) {
    fprintf(stderr, "continue;");
  } else if (node->kind == NODE_IF) {
    fprintf(stderr, "if (");
    print_node(node->cond);
    fprintf(stderr, ") ");
    print_node(node->clause_then);
    if (node->clause_else) {
      fprintf(stderr, " else ");
      print_node(node->clause_else);
    }
  } else if (node->kind == NODE_WHILE) {
    fprintf(stderr, "while (");
    print_node(node->cond);
    fprintf(stderr, ") ");
    print_node(node->clause_then);
  } else if (node->kind == NODE_FOR) {
    fprintf(stderr, "for (");
    if (node->init) print_node(node->init);
    fprintf(stderr, "; ");
    if (node->cond) print_node(node->cond);
    fprintf(stderr, "; ");
    if (node->next) print_node(node->next);
    fprintf(stderr, ") ");
    print_node(node->clause_then);
  } else if (node->kind == NODE_BLOCK) {
    fprintf(stderr, "{ ");
    for (size_t i = 0; i < node->statement_count; ++i) {
      print_node(node->statements[i]);
    }
    fprintf(stderr, "}");
  } else if (node->kind == NODE_CALL) {
    print_str_len(stderr, node->name->str, node->name->len);
    fprintf(stderr, "(");
    for (size_t i = 0; i < node->args_count; ++i) {
      if (0 < i) {
        fprintf(stderr, ", ");
      }
      print_node(node->args[i]);
    }
    fprintf(stderr, ")");
  } else if (node->kind == NODE_ADDR) {
    fprintf(stderr, "&");
    print_node(node->rhs);
  } else if (node->kind == NODE_DEREF) {
    fprintf(stderr, "*");
    print_node(node->rhs);
  } else if (node->kind == NODE_LOGICAL_NOT) {
    fprintf(stderr, "!");
    print_node(node->rhs);
  } else if (node->kind == NODE_VAR_DEC) {
    fprintf(stderr, "int ");
    print_str_len(stderr, node->name->str, node->name->len);
    fprintf(stderr, ";\n");
  } else {
    fprintf(stderr, "unimplemented printer: %d\n", node->kind);
    assert(!"unimplemented printer");
  }
}

void print_declaration(declaration_t *dec) {
  print_type(dec->type);
  print_str_len(stderr, dec->name->str, dec->name->len);
  if (dec->declaration_type == DECLARATION_GLOBAL_VARIABLE) {
    fprintf(stderr, ";\n");
  } else if (dec->declaration_type == DECLARATION_FUNCTION) {
    fprintf(stderr, "(");
    for (size_t i = 0; i < dec->func_arg_count; ++i) {
      if (0 < i) {
        fprintf(stderr, ", ");
      }
      print_str_len(stderr, dec->func_arg[i]->str, dec->func_arg[i]->len);
    }
    fprintf(stderr, ") {\n");
    for (size_t i = 0; i < dec->func_statement_count; ++i) {
      print_node(dec->func_statements[i]);
      fprintf(stderr, ";\n");
    }
    fprintf(stderr, "}\n");
  } else if (dec->declaration_type == DECLARATION_TYPEDEF) {
    print_str_len(stderr, dec->name->str, dec->name->len);
    fprintf(stderr, "\n");
  } else {
    error("failed to print declaration! type: %d", dec->declaration_type);
  }
}

char indent[1024];
int depth;

void update_indent() {
  for (int i = 0; i < 4 * depth; i = i + 4) {
    indent[i] = indent[i + 1] = indent[i + 2] = indent[i + 3] = ' ';
  }
  indent[4 * depth] = '\0';
}
void inc_depth() {
  ++depth;
  update_indent();
}
void dec_depth() {
  --depth;
  update_indent();
}
void print_indent() { printf("%s", indent); }

void gen_pop(char *dst) {
  printf("%slw %s, 0(sp)          # pop\n", indent, dst);
  printf("%saddi sp, sp, +4       #  %s\n", indent, dst);
}

void gen_push(char *src) {
  printf("%saddi sp, sp, -4       # push\n", indent);
  printf("%ssw %s, 0(sp)          #  %s\n", indent, src, src);
}

void gen(node_t *node);

void gen_lval(node_t *node) {
  if (node->kind == NODE_LOCAL_VARIABLE) {
    // local variable address
    printf("%saddi t0, fp, %d\t\t# local variable: ", indent, node->offset);
    print_str_len(stdout, node->name->str, node->name->len);
    printf("\n");
    gen_push("t0");
  } else if (node->kind == NODE_GLOBAL_VARIABLE) {
    // global variable address
    printf("%slui t0, %%hi(", indent);
    print_str_len(stdout, node->name->str, node->name->len);
    printf(")\n");
    printf("%saddi t0, t0, %%lo(", indent);
    print_str_len(stdout, node->name->str, node->name->len);
    printf(")\n");
    gen_push("t0");
  } else if (node->kind == NODE_DEREF) {
    gen(node->rhs);
  } else if (node->kind == NODE_DOT) {
    gen_lval(node->lhs);
    gen_pop("t0");
    printf("%saddi t0, t0, %d\t\t# member: ", indent, node->rhs->offset);
    print_str_len(stdout, node->rhs->name->str, node->rhs->name->len);
    printf("\n");
    gen_push("t0");
  } else if (node->kind == NODE_ARROW) {
    gen(node->lhs);
    gen_pop("t0");
    printf("%saddi t0, t0, %d\t\t# member: ", indent, node->rhs->offset);
    print_str_len(stdout, node->rhs->name->str, node->rhs->name->len);
    printf("\n");
    gen_push("t0");
  } else {
    error("左辺値が左辺値ではない！ kind=%d", node->kind);
  }
}

void gen_alloc_stack(local_variable_t *lvar) {
  size_t bytes = calc_total_local_variable_size_on_stack(lvar);
  printf("%saddi sp, sp, -%zd\t\t# stack alloc %zd B\n", indent, bytes, bytes);
}

void gen_free_stack(local_variable_t *lvar) {
  size_t bytes = calc_total_local_variable_size_on_stack(lvar);
  printf("%saddi sp, sp, %zd\t\t# stack free %zd B\n", indent, bytes, bytes);
}

void gen(node_t *node) {
  inc_depth();
  print_node(node);
  fprintf(stderr, "\n");

  if (node->kind == NODE_NUM) {
    printf("%sli t0, %d\n", indent, node->val);
    gen_push("t0");
  } else if (node->kind == NODE_CONST_STRING) {
    printf("%slui t0, %%hi(.L.C%zd)\n", indent, node->const_str->id);
    printf("%saddi t0, t0, %%lo(.L.C%zd)\n", indent, node->const_str->id);
    gen_push("t0");
  } else if (node->kind == NODE_MINUS) {
    gen(node->rhs);
    gen_pop("t0");
    printf("%ssub t0, zero, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_ADD) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");  // rhs
    gen_pop("t1");  // lhs
    if (node->lhs->type->ty == TYPE_POINTER ||
        node->lhs->type->ty == TYPE_ARRAY) {
      printf("%sli t2, %zd\n", indent,
             calc_size_of_type(node->lhs->type->ptr_to));
      printf("%smul t0, t0, t2\n", indent);
    } else if (node->rhs->type->ty == TYPE_POINTER ||
               node->rhs->type->ty == TYPE_ARRAY) {
      printf("%sli t2, %zd\n", indent,
             calc_size_of_type(node->lhs->type->ptr_to));
      printf("%smul t1, t1, t2\n", indent);
    }
    printf("%sadd t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_SUB) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    if (node->lhs->type->ty == TYPE_POINTER ||
        node->lhs->type->ty == TYPE_ARRAY) {
      printf("%sslli t0, t0, 2\n", indent);  // rhs * 4
    } else if (node->rhs->type->ty == TYPE_POINTER ||
               node->rhs->type->ty == TYPE_ARRAY) {
      printf("%sslli t1, t1, 2\n", indent);  // lhs * 4
    }
    printf("%ssub t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_MUL) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%smul t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_DIV) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%sdiv t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_MOD) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%srem t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_LT) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%sslt t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_LE) {
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
  } else if (node->kind == NODE_GT) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%ssgt t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_GE) {
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
  } else if (node->kind == NODE_LOGICAL_AND) {
    int and_end_index = gen_label_index();
    gen(node->lhs);
    gen_pop("t0");
    printf("%sbeqz t0, .L.and.end.%d\t# logical and 1\n", indent,
           and_end_index);
    gen(node->rhs);
    gen_pop("t0");
    printf(".L.and.end.%d:\n", and_end_index);
    gen_push("t0");
  } else if (node->kind == NODE_LOGICAL_OR) {
    int or_end_index = gen_label_index();
    gen(node->lhs);
    gen_pop("t0");
    printf("%sbnez t0, .L.or.end.%d\t# logical or 1\n", indent, or_end_index);
    gen(node->rhs);
    gen_pop("t0");
    printf(".L.or.end.%d:\n", or_end_index);
    gen_push("t0");
  } else if (node->kind == NODE_LOGICAL_NOT) {
    gen(node->rhs);
    gen_pop("t0");
    printf("%sseqz t0, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_EQ) {
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
  } else if (node->kind == NODE_NEQ) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");
    gen_pop("t1");
    printf("%ssub t0, t1, t0\n", indent);
    printf("%ssnez t0, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_BITWISE_AND) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");  // rhs
    gen_pop("t1");  // lhs
    printf("%sand t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_BITWISE_XOR) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");  // rhs
    gen_pop("t1");  // lhs
    printf("%sxor t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_BITWISE_OR) {
    gen(node->lhs);
    gen(node->rhs);
    gen_pop("t0");  // rhs
    gen_pop("t1");  // lhs
    printf("%sor t0, t1, t0\n", indent);
    gen_push("t0");
  } else if (node->kind == NODE_LOCAL_VARIABLE) {
    gen_lval(node);
    if (node->type->ty != TYPE_ARRAY) {
      gen_pop("t0");
      if (calc_size_of_type(node->type) == 4) {
        printf("%slw t0, 0(t0)\n", indent);
      } else if (calc_size_of_type(node->type) == 1) {
        printf("%slb t0, 0(t0)\n", indent);
      } else {
        error("invalid size of type: %zd\n", calc_size_of_type(node->type));
      }
      gen_push("t0");
    }
  } else if (node->kind == NODE_GLOBAL_VARIABLE) {
    gen_lval(node);
    if (node->type->ty != TYPE_ARRAY) {
      gen_pop("t0");
      if (calc_size_of_type(node->type) == 4) {
        printf("%slw t0, 0(t0)\n", indent);
      } else if (calc_size_of_type(node->type) == 1) {
        printf("%slb t0, 0(t0)\n", indent);
      } else {
        error("invalid size of type: %zd\n", calc_size_of_type(node->type));
      }
      gen_push("t0");
    }
  } else if (node->kind == NODE_DOT) {
    gen_lval(node->lhs);
    gen_pop("t0");
    if (node->type->ty != TYPE_ARRAY) {
      printf("%slw t0, %d(t0)\n", indent, node->rhs->offset);
    } else {
      printf("%saddi t0, t0, %d\n", indent, node->rhs->offset);
    }
    gen_push("t0");
  } else if (node->kind == NODE_ARROW) {
    gen(node->lhs);
    gen_pop("t0");
    if (node->type->ty != TYPE_ARRAY) {
      printf("%slw t0, %d(t0)\n", indent, node->rhs->offset);
    } else {
      printf("%saddi t0, t0, %d\n", indent, node->rhs->offset);
    }
    gen_push("t0");
  } else if (node->kind == NODE_ASSIGN) {
    gen(node->rhs);
    gen_lval(node->lhs);
    gen_pop("t1");  // address
    gen_pop("t0");  // value

    if (calc_size_of_type(node->type) == 4) {
      printf("%ssw t0, 0(t1)\n", indent);
    } else if (calc_size_of_type(node->type) == 1) {
      printf("%sandi t0, t0, 0xFF\n", indent);
      printf("%ssb t0, 0(t1)\n", indent);
    } else {
      error("invalid size of type: %zd\n", calc_size_of_type(node->type));
    }
    gen_push("t0");                         // value again
  } else if (node->kind == NODE_VAR_DEC) {  // almost same as NODE_ASSIGN
    if (node->rhs && node->lhs) {
      gen(node->rhs);
      gen_lval(node->lhs);
      gen_pop("t1");  // address
      gen_pop("t0");  // value

      if (calc_size_of_type(node->type) == 4) {
        printf("%ssw t0, 0(t1)\n", indent);
      } else if (calc_size_of_type(node->type) == 1) {
        printf("%sandi t0, t0, 0xFF\n", indent);
        printf("%ssb t0, 0(t1)\n", indent);
      } else {
        error("invalid size of type: %zd\n", calc_size_of_type(node->type));
      }
      // not push value because it is not used
    }
  } else if (node->kind == NODE_RETURN) {
    if (node->rhs) {
      gen(node->rhs);
      gen_pop("a0");
    }
    gen_free_stack(local_variables);
    gen_pop("fp");
    printf("%sret\n", indent);
  } else if (node->kind == NODE_BREAK) {
    printf("%sj .L.loop.end%d\n", indent, last_loop_label_index);
  } else if (node->kind == NODE_CONTINUE) {
    printf("%sj .L.loop.next%d\n", indent, last_loop_label_index);
  } else if (node->kind == NODE_IF) {
    gen(node->cond);
    gen_pop("t0");
    int if_index = gen_label_index();
    printf("%sbeqz t0, .L.else%d\n", indent, if_index);
    gen(node->clause_then);
    printf("%sj .L.if.end%d\n", indent, if_index);
    printf(".L.else%d:\n", if_index);
    if (node->clause_else) {
      gen(node->clause_else);
    }
    printf(".L.if.end%d:\n", if_index);
  } else if (node->kind == NODE_WHILE) {
    int old_loop_label_index = last_loop_label_index;
    int while_index = gen_loop_label_index();
    printf(".L.loop.cond%d: # while loop start\n", while_index);
    gen(node->cond);
    gen_pop("t0");
    printf("%sbeqz t0, .L.loop.end%d\n", indent, while_index);
    gen(node->clause_then);
    printf(".L.loop.next%d: # while loop next (empty)\n",
           while_index);  // empty, but needed for break/continue
    printf("%sj .L.loop.cond%d\n", indent, while_index);
    printf(".L.loop.end%d: # while loop end\n", while_index);
    last_loop_label_index = old_loop_label_index;
  } else if (node->kind == NODE_FOR) {
    int old_loop_label_index = last_loop_label_index;
    int for_index = gen_loop_label_index();
    printf("%s# for start\n", indent);
    if (node->init) {
      printf("%s# for init\n", indent);
      gen(node->init);
    } else {
      printf("%s# for init: empty\n", indent);
    }
    printf(".L.loop.cond%d: # for cond\n", for_index);
    if (node->cond) {
      printf("%s# for cond\n", indent);
      gen(node->cond);
      gen_pop("t0");
      printf("%sbeqz t0, .L.loop.end%d\n", indent, for_index);
    } else {
      printf("%s# for cond: empty\n", indent);
    }
    printf("%s# for body\n", indent);
    gen(node->clause_then);
    printf(".L.loop.next%d: # for next\n", for_index);
    if (node->next) {
      gen(node->next);
    }
    printf("%sj .L.loop.cond%d\n", indent, for_index);
    printf(".L.loop.end%d: # for end\n", for_index);
    last_loop_label_index = old_loop_label_index;
  } else if (node->kind == NODE_BLOCK) {
    for (size_t i = 0; i < node->statement_count; ++i) {
      gen(node->statements[i]);
    }
  } else if (node->kind == NODE_CALL) {
    char *name = calloc(node->name->len + 1, 1);
    memcpy(name, node->name->str, node->name->len);

    for (int i = 0; i < node->args_count; ++i) {
      gen(node->args[node->args_count - 1 - i]);
    }
    for (int i = 0; i < node->args_count; ++i) {
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
  } else if (node->kind == NODE_ADDR) {
    gen_lval(node->rhs);
  } else if (node->kind == NODE_DEREF) {
    gen(node->rhs);
    gen_pop("t0");
    if (calc_size_of_type(node->type) == 4) {
      printf("%slw t0, 0(t0)\n", indent);
    } else if (calc_size_of_type(node->type) == 1) {
      printf("%slb t0, 0(t0)\n", indent);
    } else {
      error("invalid size of type: %zd\n", calc_size_of_type(node->type));
    }
    gen_push("t0");
  } else {
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
  print_str_len(stdout, dec->name->str, dec->name->len);
  printf("\n");
  printf("  .type	    ");
  print_str_len(stdout, dec->name->str, dec->name->len);
  printf(", @function\n");
  print_str_len(stdout, dec->name->str, dec->name->len);
  printf(":\n");
  gen_push("fp");  // save fp

  gen_alloc_stack(local_variables);
  printf("%smv   fp, sp\n", indent);  // update fp

  // push arguments
  for (size_t i = 0; i < dec->func_arg_count; ++i) {
    char reg[] = "a0";
    local_variable_t *var = find_local_variable(dec->func_arg[i]);
    reg[1] = reg[1] + i;
    fprintf(stderr, "push arg %s\n", reg);
    printf("%ssw %s, %d(fp)\n", indent, reg, var->offset);
  }
}

void print_func_epilogue(declaration_t *dec) {}

void gen_declaration(declaration_t *dec) {
  depth = 1;
  if (dec->declaration_type == DECLARATION_GLOBAL_VARIABLE) {
    printf("  .globl  ");
    print_str_len(stdout, dec->name->str, dec->name->len);
    printf("\n");
    printf("  .section  .sdata,\"aw\"\n");
    printf("  .type     ");
    print_str_len(stdout, dec->name->str, dec->name->len);
    printf(", @object\n");

    printf("  .size     ");
    print_str_len(stdout, dec->name->str, dec->name->len);
    printf(", %zd\n", calc_size_of_type(dec->type));

    printf("  .balign    8\n");

    print_str_len(stdout, dec->name->str, dec->name->len);
    printf(":\n");
    if (dec->constant_string) {
      printf("  .word .L.C%zd", dec->constant_string->id);
    } else if (dec->constant_int) {
      assert(calc_size_of_type(dec->type) == 4);
      printf("  .word %d\n", dec->constant_int);
    } else {
      printf("  .zero %zd\n", calc_size_of_type(dec->type));
    }
    printf("\n\n");
  } else if (dec->declaration_type == DECLARATION_FUNCTION) {
    update_indent();
    print_func_prologue(dec);
    for (size_t i = 0; i < dec->func_statement_count; ++i) {
      gen(dec->func_statements[i]);
    }
    print_func_epilogue(dec);
    local_variables = NULL;
  } else if (dec->declaration_type == DECLARATION_TYPEDEF) {
    // do nothing
  } else {
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
    printf("  .section .rodata\n");
    printf("  .balign  4\n");
    printf(".L.C%zd:\n", cur->id);
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
    if (dec) {
      print_declaration(dec);
      gen_declaration(dec);
    }
  }
  print_constant_strings();

  return 0;
}