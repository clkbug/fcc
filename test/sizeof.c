struct list {
  struct list *next;
  int val;
};

typedef struct list list_alias;

int main() {
  putchar('0' + sizeof(char));
  putchar('0' + sizeof(int));
  putchar('0' + sizeof(list_alias));
  //   putchar('0' + sizeof(struct list));
  //   putchar('0' + sizeof(bool));
  //   putchar('0' + sizeof(size_t));
  return 0;
}