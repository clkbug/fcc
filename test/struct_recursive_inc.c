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
  ++a.val;
  ++b.val;
  ++c.val;
  scan(&a);
  putchar('\n');
  return;
}

void test1() {
  struct list a;
  struct list b;
  struct list c;
  a.next = &b;
  b.next = &c;
  c.next = 0;
  a.val = 1;
  b.val = 2;
  c.val = 3;
  ++a.next->val;
  ++b.next->val;
  ++a.next->next->val;
  putchar('0' + a.next->val);
  putchar('0' + b.next->val);
  putchar('0' + a.next->next->val);
  putchar('\n');
  return;
}

int main() {
  test0();
  test1();
  return 0;
}