void putint(int x) {
  if (x < 0) {
    putchar('-');
    x = -x;
  }
  if (x >= 10) {
    putint(x / 10);
  }
  putchar(x % 10 + '0');
  return;
}

void calc(char* exp) {
  int i = 0;
  int top = 0;
  int stack[100];
  int x;
  int y;
  while (exp[i] != '\0') {
    if ('0' <= exp[i] && exp[i] <= '9') {
      stack[top] = exp[i] - '0';
      top = top + 1;
    } else if (exp[i] == '+') {
      x = stack[top - 1];
      y = stack[top - 2];
      top = top - 2;
      stack[top] = x + y;
      top = top + 1;
    } else if (exp[i] == '-') {
      x = stack[top - 1];
      y = stack[top - 2];
      top = top - 2;
      stack[top] = x - y;
      top = top + 1;
    }
    i = i + 1;
  }
  putint(stack[0]);
  putchar('\n');
  return;
}

int main() {
  calc("1");
  calc("2");
  calc("12+");
  calc("123++");
  calc("12+3+4+5+6-78+-");
  return 0;
}