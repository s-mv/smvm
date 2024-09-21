#ifndef smv_smvm_util_h
#define smv_smvm_util_h

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef uint64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef double f64;
typedef float f32;

/*** util ***/
typedef struct listmv {
  void *data;
  u64 len;
  u64 cap;
  long size;
} listmv;

#define listmv(_type) listmv

void listmv_init(listmv *ls, long size);
void listmv_push(listmv *ls, void *data);
void listmv_push_array(listmv *ls, void *data, size_t num);
void *listmv_pop(listmv *ls);
void *listmv_pop_array(listmv *ls, u64 num);
void listmv_grow(listmv *ls, u64 new_cap);
void *listmv_at(listmv *ls, u64 index);
void listmv_free(listmv *ls);
i64 parse_signed(u64 unum, u8 width);
// just a helper for better debugging
void print_memory(void *arr, int num);
u32 checksum_32bit(listmv(u8) memory, listmv(u8) bytecode);
bool checksum32bit_valid(listmv(u8) memory, listmv(u8) bytecode, u32 checksum);
void mov_mem(u8 *dest, u8 *src, u64 size);
void mov_mem_reverse(u8 *dest, u8 *src, u64 size);

#endif
