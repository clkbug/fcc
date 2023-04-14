struct list {
  struct list *next;
  int val;
};

void scan(struct list *p) {
  if (p->next != 0) {
    scan(p->next);
  }
  putchar(p->val + '0');
}

void test0() {
  struct list a;
  struct list b;
  struct list c;
  a.next = &b;
  b.next = &c;
  c.next = 0;
  a.val = 1;
  b.val = 2;
  c.val = 3;
  scan(&a);
  putchar('\n');
  return;
}

int main() {
  test0();
  return 0;
}