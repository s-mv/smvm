#ifndef smv_smvm_dsmv_h
#define smv_smvm_dsmv_h

#include "smvm.h"

typedef struct dsmv {
  listmv(u8) bytecode;  // input
  listmv(char *) code;  // output
  u64 index;
} dsmv;

void dsmv_init(dsmv *ds);
void dsmv_disassemble(dsmv *ds);
void dsmv_free(dsmv *ds);

#endif
