#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsmv.h"
#include "smvm.h"
#include "util.h"

char *readfile(const char *fname);
char *get_file_extension(const char *filename);
void print_usage(const char *program_name);
int write_bytecode(smvm *vm, const char *output_filename);
char *replace_extension(const char *filename, const char *new_extension);

int main(int argc, char **argv) {
  smvm vm;
  char *content;
  char *output;
  int disassemble_mode = 0;
  int bytecode_mode = 0;
  char *bytecode_output = NULL;
  char *filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      disassemble_mode = 1;
    } else if (strcmp(argv[i], "-b") == 0) {
      bytecode_mode = 1;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        bytecode_output = argv[i + 1];
        i++;
      }
    } else if (filename == NULL) {
      filename = argv[i];
    } else {
      printf("Error: Too many arguments!\n");
      print_usage(argv[0]);
      return -1;
    }
  }

  if (filename == NULL) {
    printf("Error: No file name provided!\n");
    print_usage(argv[0]);
    return -1;
  }

  content = readfile(filename);
  if (content == NULL) { return -1; }

  smvm_init(&vm);

  char *extension = get_file_extension(filename);
  if (extension == NULL) {
    printf("Error: File has no extension!\n");
    free(content);
    smvm_free(&vm);
    return -1;
  }

  if (strcmp(extension, "asmv") == 0) {
    smvm_assemble(&vm, content);

    if (bytecode_mode) {
      if (bytecode_output == NULL) {
        bytecode_output = replace_extension(filename, "smvm");
      }

      if (write_bytecode(&vm, bytecode_output) != 0) {
        printf("Error: Failed to write bytecode to file %s\n", bytecode_output);
        if (bytecode_output != NULL &&
            bytecode_output != replace_extension(filename, "smvm")) {
          free(bytecode_output);
        }
        free(content);
        smvm_free(&vm);
        return -1;
      }

      printf("Bytecode written to %s\n", bytecode_output);

      if (bytecode_output != NULL &&
          bytecode_output != replace_extension(filename, "smvm")) {
        free(bytecode_output);
      }

      if (!disassemble_mode) {
        free(content);
        smvm_free(&vm);
        return 0;
      }
    }

    if (disassemble_mode) {
      printf("Error: Disassembly of assembly files is literally redundant.\n");
      free(content);
      smvm_free(&vm);
      return -1;
    }

    smvm_execute(&vm);
  } else if (strcmp(extension, "smvm") == 0) {
    u64 content_size = strlen(content);
    for (u64 i = 0; i < content_size; i++) {
      listmv_push(&vm.bytecode, &content[i]);
    }

    if (disassemble_mode) {
      dsmv disassembler;
      dsmv_init(&disassembler);
      disassembler.bytecode = vm.bytecode;
      dsmv_disassemble(&disassembler);
      printf("Disassembled code:\n------\n%s\n------\n",
             (char *)disassembler.code.data);
      dsmv_free(&disassembler);
    } else {
      smvm_execute(&vm);
    }
  } else {
    printf("Error: Unsupported file extension '%s'\n", extension);
    free(content);
    smvm_free(&vm);
    return -1;
  }

  free(content);
  smvm_free(&vm);
  return 0;
}

int write_bytecode(smvm *vm, const char *output_filename) {
  FILE *fp = fopen(output_filename, "wb");
  if (fp == NULL) {
    perror("Error opening output file");
    return -1;
  }

  size_t bytes_written = fwrite(vm->bytecode.data, 1, vm->bytecode.len, fp);
  if (bytes_written != vm->bytecode.len) {
    perror("Error writing bytecode to file");
    fclose(fp);
    return -1;
  }

  fclose(fp);
  return 0;
}

char *replace_extension(const char *filename, const char *new_extension) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename) {
    size_t len = strlen(filename) + strlen(new_extension) + 2;
    char *result = (char *)malloc(len);
    if (result) { snprintf(result, len, "%s.%s", filename, new_extension); }
    return result;
  }

  size_t base_len = dot - filename;
  size_t result_len = base_len + strlen(new_extension) + 2;
  char *result = (char *)malloc(result_len);
  if (!result) { return NULL; }

  strncpy(result, filename, base_len);
  result[base_len] = '.';
  strcpy(result + base_len + 1, new_extension);
  return result;
}

char *get_file_extension(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename) { return NULL; }
  return (char *)(dot + 1);
}

void print_usage(const char *program_name) {
  printf("Usage: %s [options] filename\n", program_name);
  printf("Options:\n");
  printf(
      "  -d                Disassemble the input file and print the result\n");
  printf("  -b [output_file]  Generate bytecode file after assembly\n");
  printf(
      "                    If no output_file is specified, uses input filename "
      "with .smvm extension\n");
  printf("\n");
  printf("Supported file extensions:\n");
  printf("  .asmv    Assembly source code\n");
  printf("  .smvm    VM bytecode\n");
}

char *readfile(const char *fname) {
  FILE *fp = fopen(fname, "r");
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
