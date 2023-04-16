#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void eprintf(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
}

char *read_stdin() {
  int i = 0;

  char *buf = calloc(1, 256 * 1024);
  if (!buf) {
    printf("failed to allocate memory\n");
    exit(1);
  }

  while (i < 256 * 1024) {
    char c = getchar();
    if (c < 32 || c > 126) {
      buf[i] = '\0';
      break;
    }
    if (c == EOF) {
      buf[i] = '\0';
      exit(0);
    }
    putchar(c);
    buf[i++] = c;
  }
  return buf;
}

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) error("cannot open %s: %s", path, strerror(errno));

  // check file size
  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  // read file
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // "\n\0" is added to the end of the file
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}
