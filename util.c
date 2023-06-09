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

char *read_file(char *path) {
  FILE *fp = path ? fopen(path, "r") : stdin;
  size_t size = 128 * 1024 - 2;
  if (!fp) error("cannot open %s: %s", path, strerror(errno));

  // check file size
  // if (fseek(fp, 0, SEEK_END) == -1)
  //   error("%s: fseek: %s", path, strerror(errno));
  // size_t size = ftell(fp);
  // if (fseek(fp, 0, SEEK_SET) == -1)
  //   error("%s: fseek: %s", path, strerror(errno));

  // read file
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // "\n\0" is added to the end of the file
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}
