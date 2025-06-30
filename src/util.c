#include "util.h"

/* listmv */

void listmv_init(listmv *ls, long size) {
  ls->len = 0;
  ls->cap = 1;
  ls->size = size;
  ls->data = calloc(ls->cap, size);
}

void listmv_push(listmv *ls, void *data) {
  if (ls->len >= ls->cap) { ls->cap *= 2; }
  ls->data = realloc(ls->data, ls->cap * ls->size);
  if (ls->data == NULL) {
    fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
    exit(1);
  }
  memcpy((char *)ls->data + ls->len * ls->size, data, ls->size);
  ls->len++;
}

void listmv_push_array(listmv *ls, void *data, size_t num) {
  while (ls->len + num >= ls->cap) {
    ls->cap *= 2;
    ls->data = realloc((char *)ls->data, ls->cap * ls->size);
    if (ls->data == NULL) {
      fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
      exit(1);
    }
  }
  mov_mem((u8 *)ls->data + ls->len * ls->size, (u8 *)data, num * ls->size);
  ls->len += num;
}

void *listmv_pop(listmv *ls) {
  if (ls->len == 0) return NULL;
  ls->len--;
  return (char *)ls->data + ls->len * ls->size;
}

void *listmv_pop_array(listmv *ls, u64 num) {
  if (ls->len < num) return NULL;
  ls->len -= num;
  return (char *)ls->data + ls->len * ls->size;
}

void listmv_grow(listmv *ls, u64 new_cap) {
  if (new_cap <= ls->cap) return;
  ls->cap = new_cap;
  ls->data = realloc(ls->data, ls->cap * ls->size);
  if (ls->data == NULL) {
    fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
    exit(1);
  }
}

void *listmv_at(listmv *ls, u64 index) {
  return (char *)ls->data + index * ls->size;
}

void listmv_free(listmv *ls) {
  free(ls->data);
  ls->cap = 0;
}

/* other utilities */

i64 parse_signed(u64 unum, u8 width) {
  if (width == 8) return unum;
  i64 result;
  u64 mask = (1ull << (width * 8)) - 1;
  u64 sign_bit = 1ull << ((width * 8) - 1);

  unum &= mask;
  if (unum & sign_bit) result = (i64)(unum | ~mask);
  else result = (i64)unum;
  return result;
}

void print_memory(void *arr, int num) {
  unsigned char *p = (unsigned char *)arr;
  for (int i = 0; i < num; i++) {
    unsigned char byte = p[i];
    for (int j = 7; j >= 0; j--) {
      printf("%d", (byte >> j) & 0x01);
      if (j % 8 == 0) printf("|");
      else if (j % 4 == 0) printf(" ");
    }
  }
  printf("\n");
}

u32 checksum_32bit(listmv(u8) memory, listmv(u8) bytecode) {
  u32 checksum = 0;
  for (int i = 0; i < memory.len; i++) {
    checksum += *(u8 *)listmv_at(&memory, i);
  }
  for (int i = 0; i < bytecode.len; i++) {
    checksum += *(u8 *)listmv_at(&bytecode, i);
  }

  return checksum;
}

bool checksum32bit_valid(listmv(u8) memory, listmv(u8) bytecode, u32 checksum) {
  u32 current_checksum = checksum_32bit(memory, bytecode);
  return current_checksum == checksum;
}

/* inline functions */

void mov_mem(u8 *dest, u8 *src, u64 size) { memcpy(dest, src, size); }

void mov_mem_reverse(u8 *dest, u8 *src, u64 size) {
  dest += size - 1;
  while (size--) *dest-- = *src++;
}
