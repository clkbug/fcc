char* a = "hello, world\n";
char* b = "hello, world\n";
// char* b = "madam, i am adam\n";
// char* c = "no melon, no lemon\n";

void print(char* c) {
  while (*c != '\0') {
    putchar(*c);
    c = c + 1;
  }
  return;
}

int main() {
  print(a);
  print(b);
  //   print(c);
  return 0;
}
