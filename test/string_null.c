
char *a = "a";
char *b = "b";

void p(char c) {
  putchar(c);
  putchar(c == '\0');
  putchar(c != '\0');
  return;
}
void q0(char *c) {
  putchar(*c == '\0');
  return;
}
void q(char *c) {
  putchar(*c);
  q0(c);
  return;
}

int main() {
  // p(a[1]);
  // p(b[1]);
  // q(&a[1]);
  q(&b[1]);
  putchar(b[1]);
  return 0;
}