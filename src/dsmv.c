#include "dsmv.h"

void dsmv_init(dsmv *ds) {
  listmv_init(&ds->code, sizeof(char));
  ds->index = 0;
}

void dsmv_disassemble(dsmv *ds) {
}

/* vm - disassembler (dsmv) - functions */
void dsmv_free(dsmv *ds) {
  listmv_free(&ds->code);
  // ownership of ds->bytecode goes to a vm
  // so no need to free it
}
