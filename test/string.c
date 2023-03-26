char* global_variable = "global variable\n";

int main() {
  char* local_variable = "local variable\n";
  puts(global_variable);
  puts("Hello, world!\n");
  puts(local_variable);
  return 0;
}