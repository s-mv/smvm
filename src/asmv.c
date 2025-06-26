#include "asmv.h"

#include "smvm.h"

void asmv_init(asmv *as) {
  as->index = 0;
  as->panic_mode = false;
  listmv_init(&as->output.bytecode, sizeof(u8));
  listmv_init(&as->instructions, sizeof(asmv_inst));
  listmv_init(&as->label_addrs, sizeof(asmv_label));
  listmv_init(&as->label_refs, sizeof(label_reference));
}

u8 asmv_parse_register(asmv *as) {
  u64 backup_index = as->index;

  if (asmv_current(as) == 'i' && asmv_peek(as) == 'p' &&
      !isalnum(asmv_peek2(as))) {
    as->index += 2;
    return (smvm_reg64 << 3) | reg_ip;
  }

  if (asmv_current(as) == 's' && asmv_peek(as) == 'p' &&
      !isalnum(asmv_peek2(as))) {
    as->index += 2;
    return (smvm_reg64 << 3) | reg_sp;
  }

  if (asmv_current(as) == 's' && asmv_peek(as) == 'o' &&
      !isalnum(asmv_peek2(as))) {
    as->index += 2;
    return (smvm_reg64 << 3) | reg_so;
  }

  if (asmv_current(as) == 'a' || asmv_current(as) == 'b' ||
      asmv_current(as) == 'c' || asmv_current(as) == 'd') {
    asmv_skip(as);
    if (asmv_current(as) == '8') {
      asmv_skip(as);
      return (smvm_reg8 << 3) | (reg_a + asmv_prev(as) - 'a');
    }
    if (asmv_current(as) == '1' && asmv_peek(as) == '6') {
      as->index += 2;
      return (smvm_reg16 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }
    if (asmv_current(as) == '3' && asmv_peek(as) == '2') {
      as->index += 2;
      return (smvm_reg32 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }
    if (asmv_current(as) == '6' && asmv_peek(as) == '4') {
      as->index += 2;
      return (smvm_reg64 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }

    if (!isdigit(asmv_current(as)))
      return (smvm_reg64 << 3) | (reg_a + asmv_prev(as) - 'a');
  }

  as->index = backup_index;
  return reg_none;
}

// TODO, floating point numbers
asmv_op_data parse_number(asmv *as) {
  double num = 0.0;
  bool neg = false;
  bool is_float = false;
  double fraction = 0.1;

  if (asmv_current(as) == '-') {
    neg = true;
    asmv_skip(as);
  }

  while (isdigit(asmv_current(as)) || asmv_current(as) == '.' ||
         asmv_current(as) == '_') {
    if (asmv_current(as) == '.') {
      if (is_float) {
        // multiple decimal points (TODO, handle)
        break;
      }
      is_float = true;
      asmv_skip(as);
    } else if (asmv_current(as) == '_') asmv_skip(as);
    else {
      if (!is_float) {
        num = num * 10 + (asmv_current(as) - '0');
      } else {
        num += (asmv_current(as) - '0') * fraction;
        fraction *= 0.1;
      }
      asmv_skip(as);
    }
  }

  asmv_op_data data;
  if (is_float) {
    data.type = asmv_fnum_type;
    data.fnum = neg ? -num : num;
  } else if (neg) {
    data.type = asmv_num_type;
    data.num = -num;
  } else {
    data.type = asmv_unum_type;
    data.unum = num;
  }

  return data;
}

u8 parse_offset(asmv *as) {
  while (isspace(asmv_current(as))) asmv_skip(as);

  // no offset!
  if (asmv_current(as) != '+' && asmv_current(as) != '-') return 0;

  char sign = asmv_current(as);
  asmv_skip(as);

  asmv_op_data offset = parse_number(as);
  // invalid offset, treated as 0
  if (offset.type != asmv_num_type && offset.type != asmv_unum_type) return 0;

  return (sign == '+') ? offset.num : -offset.num;
}

// I hate nesting
asmv_inst asmv_lex_inst(asmv *as) {
  char buffer[512];
  u64 offset = 0;
  asmv_operand op = {0};
  asmv_inst inst = {.error = asmv_all_ok, .eof = false, .label = false};
  char current;

  while (isspace(asmv_current(as))) asmv_skip(as);
  current = asmv_current(as);
  // ignore comments
  if (current == ';' || current == '#')
    while (asmv_next(as) != '\n');
  while (isspace(asmv_current(as))) asmv_skip(as);
  current = asmv_current(as);
  if (current == '\0') return ((asmv_inst){.eof = true});
  // label
  if (current == '.') {
    asmv_skip(as);
    offset = 0;
    while (isalnum(as->code[as->index + offset])) offset++;
    char eof = '\0';
    listmv_init(&inst.str, sizeof(char));
    listmv_push_array(&inst.str, as->code + as->index, offset);
    listmv_push(&inst.str, &eof);
    inst.label = true;
    inst.index = as->index;
    as->index += offset;
    return inst;
  }
  current = asmv_current(as);
  if (!isalpha(current)) return (asmv_inst){.error = asmv_misc_error};

  while (isalpha(as->code[as->index + offset])) offset++;
  strncpy(buffer, as->code + as->index, offset);
  buffer[offset + 1] = '\0';
  as->index += offset;

  for (int i = 0; i < instruction_table_len; i++) {
    if (strncmp(instruction_table[i].name, buffer,
                instruction_table[i].str_size))
      continue;

    inst.code = i;

    for (int j = 0; j < instruction_table[i].num_ops; j++) {
      while (isspace(asmv_current(as))) asmv_skip(as);
      current = asmv_current(as);

      if (current == '-' || isdigit(current)) {
        // immediate mode, handle numbers
        // sub 12 b c makes no sense, 12 = b + c what??
        if (instruction_table[i].num_ops > 1 && j == 0)
          return (asmv_inst){.error = asmv_immediate_x};
        // handle immediate addressing
        op.mode = mode_immediate;
        asmv_op_data data = parse_number(as);
        op.data = data;
        if (data.type == asmv_num_type) {
          op.size = min_space_needed(op.data.num);
          if (op.size != smvm_reg64)
            op.data.num = op.data.num & ((1ull << (8 << op.size)) - 1);
        } else if (data.type == asmv_unum_type) {
          op.size = min_space_neededu(op.data.num);
        } else if (data.type == asmv_fnum_type) {
          op.size = smvm_reg64;
        }
        op.width = op.size;
        op.offset = parse_offset(as);
      } else if (current == '@') {
        asmv_skip(as);
        while (isspace(asmv_current(as))) asmv_skip(as);
        if (isdigit(asmv_current(as))) {
          // if number, direct addressing mode
          // we don't check for - here since addresses are non-negative
          op.mode = mode_direct;
          op.data = parse_number(as);
          op.size = min_space_neededu(op.data.unum);
          if (asmv_current(as) == '>') {
            asmv_skip(as);
            if (asmv_current(as) == '6' && asmv_peek(as) == '4') {
              op.width = smvm_reg64;
              as->index += 2;
            } else if (asmv_current(as) == '3' && asmv_peek(as) == '2') {
              op.width = smvm_reg32;
              as->index += 2;
            } else if (asmv_current(as) == '1' && asmv_peek(as) == '6') {
              op.width = smvm_reg16;
              as->index += 2;
            } else if (asmv_current(as) == '8') {
              op.width = smvm_reg8;
              asmv_skip(as);
            }
          } else op.width = smvm_reg64;
          op.offset = parse_offset(as);
        } else if (asmv_current(as) == 'r') {
          // else if register, indirect addressing mode
          asmv_skip(as);
          smvm_register reg = asmv_parse_register(as);
          // TODO handle reg_none better
          if (reg == reg_none) break;
          op.mode = mode_indirect;
          op.width = reg >> 3;
          op.data.reg = reg & 0b111;
          op.offset = parse_offset(as);
        } else {
          // TODO, handle error
        }
      } else if (current == 'r') {
        asmv_skip(as);
        smvm_register reg = asmv_parse_register(as);  // gives width|reg
        // TODO handle reg_none better mayhaps
        // time will tell
        if (reg == reg_none) continue;
        op.mode = mode_register;
        op.width = reg >> 3;
        op.data.reg = reg & 0b111;
        op.offset = parse_offset(as);
      } else if (current == '"' &&
                 (inst.code == op_puts || inst.code == op_extern ||
                  inst.code == op_scall)) {
        asmv_skip(as);
        listmv_init(&op.data.str, sizeof(char));

        while (asmv_current(as) != '"') {
          current = asmv_current(as);
          // checks for `\n`, etc.
          if (current == '\\' && asmv_peek(as) != '\0') {
            current = asmv_next(as);
            switch (current) {
              case 'n': current = '\n'; break;
              case 't': current = '\t'; break;
              case 'r': current = '\r'; break;
              case '\\':
              case '"': break;
              default: as->index--; current = '\\';
            }
          }
          listmv_push(&op.data.str, &current);
          asmv_skip(as);
        }

        listmv_push(&op.data.str, &(char){'\0'});

        op.data.type = asmv_str_type;
        op.mode = mode_register;
        asmv_skip(as);
      } else if (current == '.') {  // handling labels
        asmv_skip(as);
        offset = 0;
        while (isalnum(as->code[as->index + offset])) offset++;
        op.data.type = asmv_label_type;
        op.mode = mode_immediate;
        char eof = '\0';
        listmv_init(&op.data.str, sizeof(char));
        listmv_push_array(&op.data.str, as->code + as->index, offset);
        listmv_push(&op.data.str, &eof);
        as->code += offset;
      } else {
        // TODO edge cases, error handling
      }
      inst.operands[j] = op;
    }
    break;
  }

  return inst;
}

void asmv_assemble(asmv *as) {
  u64 offset = 0;  // track offset

  // first pass
  while (as->code[as->index] != '\0') {
    // TODO also lex "let"?
    if (asmv_current(as) == 'l' && asmv_peek(as) == 'e' &&
        asmv_peek2(as) == 't' && !isalnum(as->code[as->index])) {}

    asmv_inst inst = asmv_lex_inst(as);
    if (inst.eof) break;
    if (!inst.label) {
      if (instruction_table[inst.code].num_ops == 0) offset++;
      else {
        offset += instruction_table[inst.code].num_ops == 3 ? 4 : 3;
        for (int i = 0; i < instruction_table[inst.code].num_ops; i++) {
          // TODO efficiency for storing labels
          if (inst.operands[i].data.type == asmv_label_type) offset += 8;
          else if (inst.operands[i].data.type == asmv_str_type)
            offset += inst.operands[i].data.str.len;
          else if (inst.operands[i].mode > 1)
            offset += 1 << inst.operands[i].size;
        }
      }
    } else {
      asmv_label label = {.address = offset, .str = inst.str};
      listmv_push(&as->label_addrs, &label);
    }

    listmv_push(&as->instructions, &inst);
    asmv_inst *inst_ref =
        listmv_at(&as->instructions, as->instructions.len - 1);

    for (int i = 0; i < instruction_table[inst.code].num_ops; i++)
      if (inst_ref->operands[i].data.type == asmv_label_type) {
        label_reference ref = {as->instructions.len - 1, i};
        listmv_push(&as->label_refs, &ref);
        continue;
      }
  }

  // check labels
  for (int i = 0; i < as->label_addrs.len; i++) {
    asmv_label label = *(asmv_label *)listmv_at(&as->label_addrs, i);
    bool matched = false;
    for (int j = 0; j < as->label_refs.len; j++) {
      label_reference *ref = listmv_at(&as->label_refs, j);
      asmv_inst *inst = listmv_at(&as->instructions, ref->inst_index);
      asmv_operand *op = &inst->operands[ref->op_index];
      if (op->data.str.cap && !strcmp(label.str.data, op->data.str.data)) {
        listmv_free(&op->data.str);
        op->mode = mode_immediate;
        op->data.unum = label.address;
        op->width = smvm_reg64;
        op->size = smvm_reg64;
        matched = true;
        break;
      }
    }
    if (matched) continue;
  }

  // second pass
  for (int i = 0; i < as->instructions.len; i++) {
    asmv_inst inst = *(asmv_inst *)listmv_at(&as->instructions, i);
    if (inst.eof) break;
    if (inst.label) {
      listmv_free(&inst.str);
      continue;
    }
    if (inst.error != asmv_all_ok) {
      // TODO, better error handling?
      printf("error in assembling: %d\n", inst.error);
      continue;
    }

    const u8 num_ops = instruction_table[inst.code].num_ops;

    if (num_ops == 0) {
      listmv_push(&as->output.bytecode, &inst.code);
      continue;
    }

    u8 primary_bytes[4] = {inst.code, 0, 0, 0};
    // u8 primary_size = 4;
    u8 immediate_bytes[27] = {0};  // data/address bytes
    u64 immediate_size = 0;

    // add "hey, data contains offset!" flags
    if (inst.operands[0].offset) primary_bytes[1] |= 1 << 5;
    if (inst.operands[1].offset) primary_bytes[1] |= 1 << 4;
    if (inst.operands[2].offset) primary_bytes[3] |= 1 << 5;

    // add width bits
    primary_bytes[1] |= inst.operands[0].width << 2;
    primary_bytes[1] |= inst.operands[1].width;
    primary_bytes[3] |= inst.operands[2].width << 3;

    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];

      // set the info and mode bits
      primary_bytes[i] |= op.mode << 6;
      if (op.data.type == asmv_str_type) continue;

      // set the register/data bits
      u8 byte = i == 2 ? 3 : 2;
      u8 offset = i == 1 ? 3 : 0;

      if (op.offset) immediate_bytes[immediate_size++] = op.offset;
      if (op.mode == mode_register || op.mode == mode_indirect) {
        primary_bytes[byte] |= op.data.reg << offset;
      } else if (op.mode == mode_direct || op.mode == mode_immediate) {
        primary_bytes[byte] |= op.size << offset;
        u8 data_size = 1 << op.size;  // ranges from 1 to 8
        immediate_size += data_size;
        for (int n = 0; n < data_size; n++)
          immediate_bytes[immediate_size - n - 1] =
              (op.data.num >> (n * 8)) & 0xff;
      }
    }

    listmv_push_array(&as->output.bytecode, primary_bytes,
                      num_ops == 3 ? 4 : 3);
    listmv_push_array(&as->output.bytecode, immediate_bytes, immediate_size);
    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];
      if (op.data.type != asmv_str_type) continue;
      listmv_push_array(&as->output.bytecode, op.data.str.data,
                        op.data.str.len);
      listmv_free(&op.data.str);
    }
  }

  // append the header now
  as->output.header = (smvm_header){
      .version = smvm_version,
      .header_flags = 0,          // TODO
      .checksum = 0,              // TODO
      .global_variables_len = 0,  // TODO
      .code_len = as->output.bytecode.len,
  };
}

void asmv_free(asmv *as) {
  listmv_free(&as->instructions);
  listmv_free(&as->label_refs);
  listmv_free(&as->label_addrs);
  // bytecode is now owned by a vm
  // so no need to free it
}

char asmv_current(asmv *a) { return a->code[a->index]; }
char asmv_peek(asmv *a) { return a->code[a->index + 1]; }
char asmv_peek2(asmv *a) { return a->code[a->index + 2]; }
char asmv_prev(asmv *a) { return a->code[a->index - 1]; }
char asmv_prev2(asmv *a) { return a->code[a->index - 2]; }
char asmv_next(asmv *a) { return a->code[++a->index]; }
void asmv_skip(asmv *a) { ++a->index; }
