char* s = "goodbye, world";

int main() {
  int i;
  printf("hello, world\n");
  for (i = 0; i < 10; ++i) {
    printf("%d\t", i);
  }
  printf("\n");
  printf("%s\n", s);
  return 0;
}