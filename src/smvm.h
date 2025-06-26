#ifndef smv_smvm_smvm_h
#define smv_smvm_smvm_h

#include "util.h"

// version -> ff.fff.fff
// current version -> 0.1.0
#define smvm_version (0x00001000)
#define smvm_register_num (8)

typedef struct smvm_header {
  u16 version;
  u16 header_flags;  // enum header_flag
  u32 checksum;      // checksum of data + code
  u64 global_variables_len;
  u64 code_len;
} smvm_header;

typedef struct smvm {
  listmv(u8) memory;
  listmv(u8) bytecode;
  listmv(u8) stack;
  listmv(smvm_syscall) syscalls;  // natives
  smvm_header header;
  i64 registers[smvm_register_num];
  u8 flags;
  bool little_endian;
  // this is all cache, maybe I should make another struct
  struct cache {
    i64 *pointers[3];
    i64 data[3];
    u8 widths[3];
    u8 offset;
  } cache;
} smvm;

typedef struct smvm_syscall {
  u64 id;
  char *name;
  void *function;
} smvm_syscall;

typedef enum smvm_opcode : u8 {
  op_halt = 0b000000,
  op_mov = 0b000001,
  op_movu = 0b000010,
  op_movf = 0b000011,
  op_swap = 0b000100,
  op_lea = 0b000101,
  op_add = 0b000110,
  op_addu = 0b000111,
  op_addf = 0b001000,
  op_sub = 0b001001,
  op_subu = 0b001010,
  op_subf = 0b001011,
  op_mul = 0b001100,
  op_mulu = 0b001101,
  op_mulf = 0b001110,
  op_div = 0b001111,
  op_divu = 0b10000,
  op_divf = 0b10001,
  op_inc = 0b10010,
  op_dec = 0b10011,
  op_and = 0b10100,
  op_or = 0b10101,
  op_xor = 0b10110,
  op_shl = 0b10111,
  op_shr = 0b11000,
  op_shli = 0b11001,
  op_shri = 0b11010,
  op_slc = 0b11011,
  op_src = 0b11100,
  op_jmp = 0b11101,
  op_je = 0b11110,
  op_jne = 0b11111,
  op_jl = 0b100000,
  op_loop = 0b100001,
  op_call = 0b100010,
  op_ret = 0b100011,
  op_push = 0b100100,
  op_pop = 0b100101,
  op_extern = 0b100110,
  op_scall = 0b100111,
  op_getu = 0b101000,
  op_puti = 0b101001,
  op_putu = 0b101010,
  op_putf = 0b101011,
  op_puts = 0b101100,
} smvm_opcode;

typedef enum smvm_register : u8 {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_fl = 0b100,  // flag register
  reg_sp = 0b101,  // stack pointer
  reg_ip = 0b110,  // instruction pointer
  reg_so = 0b111,  // syscall offset register (a lifesaver)
  reg_none = 0b1000,
} smvm_register;

typedef enum smvm_data_width : u8 {
  smvm_reg8 = 0b00,
  smvm_reg16 = 0b01,
  smvm_reg32 = 0b10,
  smvm_reg64 = 0b11,
} smvm_data_width;

typedef enum smvm_flag : u8 {
  flag_o = 1,       // overflow
  flag_d = 1 << 1,  // direction
  flag_t = 1 << 2,  // trap ~TODO add errors~
  flag_s = 1 << 3,  // sign
  flag_z = 1 << 4,  // zero
} smvm_flag;

typedef enum smvm_header_flag {
  flag_interface = 1,
} smvm_header_flag;

typedef enum smvm_mode {
  // mode takes 3 bits however only 2 bits are present here and the reason is
  // that the 3rd bit just indicates that there's an offset appended
  // 0th bit indicates if data is appended   (1 = appended)
  // 1st bit indicates if register is needed (1 = needed)
  // 2nd bit indicates if offset is stored   (1 = yes)
  mode_register = 0b00,   // r
  mode_indirect = 0b01,   // x
  mode_immediate = 0b10,  // i
  mode_direct = 0b11,     // d
  // implicit type is quite literally, implicit; implicit instructions are
  // simply run without checking the "mode" first, unlike other instructions
  // mode_implicit = 0b111,  // TODO remove?
} smvm_mode;

void smvm_init(smvm *vm);
void smvm_link_call(smvm *vm, void (*fn)(smvm *), const char *name);
void smvm_assemble(smvm *vm, char *code);
void smvm_execute(smvm *vm);
void smvm_disassemble(smvm *vm, char *code);
void smvm_free(smvm *vm);

/* some helpers */
void smvm_push(smvm *vm, u8 *value, u64 width);
u8 *smvm_pop(smvm *vm, u64 width);
bool is_little_endian();

/* helpers */

void update_stack_pointer(smvm *vm);
smvm_data_width min_space_neededu(u64 data);
smvm_data_width min_space_needed(i64 data);
void smvm_ip_inc(smvm *vm, u64 inc);
u8 *smvm_fetch_inst_addr(smvm *vm);
u16 smvm_get_flag(smvm *vm, smvm_flag flag);
void smvm_set_flag(smvm *vm, smvm_flag flag);
void smvm_reset_flag(smvm *vm, smvm_flag flag);

/* operation function declarations */
// this feels so redundant, if only I wasn't lazy and wrote a macro or
// something...
void trap_fn(smvm *vm);
void mov_fn(smvm *vm);
void movu_fn(smvm *vm);
void movf_fn(smvm *vm);
void swap_fn(smvm *vm);
void lea_fn(smvm *vm);
void add_fn(smvm *vm);
void addu_fn(smvm *vm);
void addf_fn(smvm *vm);
void sub_fn(smvm *vm);
void subu_fn(smvm *vm);
void subf_fn(smvm *vm);
void mul_fn(smvm *vm);
void mulu_fn(smvm *vm);
void mulf_fn(smvm *vm);
void div_fn(smvm *vm);
void divu_fn(smvm *vm);
void divf_fn(smvm *vm);
void inc_fn(smvm *vm);
void dec_fn(smvm *vm);
void and_fn(smvm *vm);
void or_fn(smvm *vm);
void xor_fn(smvm *vm);
void shl_fn(smvm *vm);
void shr_fn(smvm *vm);
void shli_fn(smvm *vm);
void shri_fn(smvm *vm);
void slc_fn(smvm *vm);
void src_fn(smvm *vm);
void jmp_fn(smvm *vm);
void je_fn(smvm *vm);
void jne_fn(smvm *vm);
void jl_fn(smvm *vm);
void loop_fn(smvm *vm);
void call_fn(smvm *vm);
void ret_fn(smvm *vm);
void push_fn(smvm *vm);
void pop_fn(smvm *vm);
void extern_fn(smvm *vm);
void scall_fn(smvm *vm);
void getu_fn(smvm *vm);
void puti_fn(smvm *vm);
void putu_fn(smvm *vm);
void putf_fn(smvm *vm);
void puts_fn(smvm *vm);

// TODO, implement hashing or something
typedef struct instruction_info {
  char *name;
  u8 str_size : 4;
  u8 num_ops : 4;
  void (*fn)(smvm *);
} instruction_info;

#define instruction_table_len (45)
extern instruction_info instruction_table[instruction_table_len];

#endif
