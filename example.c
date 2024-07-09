#include <stdio.h>
#include <stdlib.h>

#include "smvm.h"

char *readfile(const char *fname);

int main() {
  smvm vm;
  char *content = readfile("example.asmv");
  smvm_init(&vm);
  smvm_assemble(&vm, content, false);
  smvm_execute(&vm);
  free(content);
  smvm_free(&vm);
  return 0;
}

char *readfile(const char *fname) {
  FILE *fp = fopen(fname, "rb");
  long file_size;

  if (fp == NULL) {
    perror("Error opening file");
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  rewind(fp);

  char *content = (char *)malloc(file_size + 1);

  if (content == NULL) {
    perror("Error allocating memory");
    fclose(fp);
    return NULL;
  }

  size_t bytes_read = fread(content, 1, file_size, fp);

  if (bytes_read != file_size) {
    perror("Error reading file");
    free(content);
    fclose(fp);
    return NULL;
  }

  content[file_size] = '\0';

  fclose(fp);
  return content;
}
