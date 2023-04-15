struct tree_node {
  int value;
  struct tree_node* child[2];
};

struct tree_node* new_tree(int value) {
  struct tree_node* node = calloc(1, sizeof(struct tree_node));
  node->value = value;
  return node;
}

void dfs(struct tree_node* node) {
  if (!node) {
    return;
  }
  putchar(node->value + '0');
  dfs(node->child[0]);
  dfs(node->child[1]);
  return;
}
void dfs2(struct tree_node* node) {
  if (!node) {
    return;
  }
  dfs(node->child[0]);
  putchar(node->value + '0');
  dfs(node->child[1]);
  return;
}
void dfs3(struct tree_node* node) {
  if (!node) {
    return;
  }
  dfs(node->child[0]);
  dfs(node->child[1]);
  putchar(node->value + '0');
  return;
}

int main() {
  struct tree_node* tree[16];
  int i;
  for (i = 0; i < 16; ++i) {
    tree[i] = new_tree(i);
  }
  for (i = 0; i < 16; ++i) {
    putchar(tree[i]->value + '0');
  }
  putchar('\n');
  for (i = 0; i < 16; ++i) {
    if (2 * i + 1 < 16) {
      tree[i]->child[0] = tree[2 * i + 1];
    }
    if (2 * i + 2 < 16) {
      tree[i]->child[1] = tree[2 * i + 2];
    }
  }
  dfs(tree[0]);
  putchar('\n');
  dfs2(tree[0]);
  putchar('\n');
  dfs3(tree[0]);
  putchar('\n');

  return 0;
}