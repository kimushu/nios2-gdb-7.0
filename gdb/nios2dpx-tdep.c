/* Target-machine dependent code for Nios2 DPX, for GDB.
   Copyright (C) 2003-2005
   by Peter Brookes (pbrookes@altera.com)
   and Andrew Draper (adraper@altera.com)

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "trad-frame.h"
#include "dwarf2-frame.h"
#include "symtab.h"
#include "inferior.h"
#include "gdbtypes.h"
#include "gdbcore.h"
#include "gdbcmd.h"
#include "target.h"
#include "dis-asm.h"
#include "regcache.h"
#include "value.h"
#include "symfile.h"
#include "arch-utils.h"
#include "floatformat.h"
#include "gdb_assert.h"
#include "solib-svr4.h"
#include "glibc-tdep.h"
#include "infcall.h"
#include "regset.h"
#include "tramp-frame.h"

#include "elf-bfd.h"
#include "elf/nios2.h"

/* To get entry_point_address.  */
#include "objfiles.h"

/* Nios II ISA specific encodings and macros */
#include "opcode/nios2dpx.h"

/* Macros */

#define Z_REGNUM 0    /* Zero */
#define R2_REGNUM 2    /* used for return value */
#define R3_REGNUM 3    /* used for return value */
/* used for hidden zero argument to store ptr to struct return value */
#define R4_REGNUM 4
#define R7_REGNUM 7
#define GP_REGNUM 26  /* Global Pointer */
#undef SP_REGNUM
#define SP_REGNUM 27  /* Stack Pointer */
#undef FP_REGNUM
#define FP_REGNUM 28  /* Frame Pointer */
#define EA_REGNUM 29  /* Exception address */
#define BA_REGNUM 30  /* Breakpoint return address */
#define RA_REGNUM 31  /* Return address */
#undef PC_REGNUM
#define PC_REGNUM 64

/* Ctrl registers */
#define CPUID_REGNUM 70
#define THREADNUM_REGNUM 81

#define FIRST_ARGREG R4_REGNUM
#define LAST_ARGREG R7_REGNUM
/* Number of all registers */
#define NIOS2_NUM_REGS (129)
/* The maximum register number displayed to the user, */
/* as a result of typing "info reg" at the gdb prompt */
/* dpx has 128 registers, including the control registers*/
#define NIOS2_MAX_REG_DISPLAYED_REGNUM (128)

#define NIOS2_OPCODE_SIZE 4

// we "get" the variable remote_hw_breakpoint_limit from remote.c, then set it to 4 to comply to DPX's maximum hardware breakpoint count
extern int remote_hw_breakpoint_limit;

/* Structures */
struct register_info
{
  int size;
  char *name;
  struct type **type;
};

/* The current value in the register is the value in r[base] at the start of
 * the function + offset ; unless base < 0 in which case it's unknown.
 */
typedef struct
{
  int reg;
  unsigned int offset;

} REG_VALUE;


typedef struct
{
  int basereg;
  CORE_ADDR addr;

} REG_SAVED;

struct nios2_unwind_cache
{
  /* The frame's base, optionally used by the high-level debug info.  */
  CORE_ADDR base;

  /* The previous frame's inner most stack address.  Used as this
     frame ID's stack_addr.  */
  CORE_ADDR cfa;

  /* The address of the first instruction in this function */
  CORE_ADDR pc;

  /* Which register holds the return address for the frame. */
  int return_regnum;

  /* Table indicating what changes have been made to each register */
  REG_VALUE reg_value[NIOS2_NUM_REGS];

  /* Table indicating where each register has been saved.  */
  REG_SAVED reg_saved[NIOS2_NUM_REGS];
};


/* Function prototypes */
CORE_ADDR nios2_saved_pc_after_call (struct frame_info *fi);

/* nios2_register_info_table[i] is the number of bytes of storage in
   GDB's register array occupied by register i.  */
static struct register_info nios2_register_info_table[] = {
  /*  0 */ {4, "zero", 0},
  /*  1 */ {4, "at", 0},
  /*  2 */ {4, "r2", 0},
  /*  3 */ {4, "r3", 0},
  /*  4 */ {4, "r4", 0},
  /*  5 */ {4, "r5", 0},
  /*  6 */ {4, "r6", 0},
  /*  7 */ {4, "r7", 0},
  /*  8 */ {4, "r8", 0},
  /*  9 */ {4, "r9", 0},
  /* 10 */ {4, "r10", 0},
  /* 11 */ {4, "r11", 0},
  /* 12 */ {4, "r12", 0},
  /* 13 */ {4, "r13", 0},
  /* 14 */ {4, "r14", 0},
  /* 15 */ {4, "r15", 0},
  /* 16 */ {4, "r16", 0},
  /* 17 */ {4, "r17", 0},
  /* 18 */ {4, "r18", 0},
  /* 19 */ {4, "r19", 0},
  /* 20 */ {4, "r20", 0},
  /* 21 */ {4, "r21", 0},
  /* 22 */ {4, "r22", 0},
  /* 23 */ {4, "r23", 0},
  /* 24 */ {4, "et", 0},
  /* 25 */ {4, "bt", 0},
  /* 26 */ {4, "gp", 0},
  /* 27 */ {4, "sp", 0},
  /* 28 */ {4, "fp", 0},
  /* 29 */ {4, "ea", 0},
  /* 30 */ {4, "ba", 0},
  /* 31 */ {4, "ra", 0},

  /* 32 */ {4, "r32", 0},
  /* 33 */ {4, "r33", 0},
  /* 34 */ {4, "r34", 0},
  /* 35 */ {4, "r35", 0},
  /* 36 */ {4, "r36", 0},
  /* 37 */ {4, "r37", 0},
  /* 38 */ {4, "r38", 0},
  /* 39 */ {4, "r39", 0},
  /* 40 */ {4, "r40", 0},
  /* 41 */ {4, "r41", 0},
  /* 42 */ {4, "r42", 0},
  /* 43 */ {4, "r43", 0},
  /* 44 */ {4, "r44", 0},
  /* 45 */ {4, "r45", 0},
  /* 46 */ {4, "r46", 0},
  /* 47 */ {4, "r47", 0},
  /* 48 */ {4, "r48", 0},
  /* 49 */ {4, "r49", 0},
  /* 50 */ {4, "r50", 0},
  /* 51 */ {4, "r51", 0},
  /* 52 */ {4, "r52", 0},
  /* 53 */ {4, "r53", 0},
  /* 54 */ {4, "r54", 0},
  /* 55 */ {4, "r55", 0},
  /* 56 */ {4, "r56", 0},
  /* 57 */ {4, "r57", 0},
  /* 58 */ {4, "r58", 0},
  /* 59 */ {4, "r59", 0},
  /* 60 */ {4, "r60", 0},
  /* 61 */ {4, "r61", 0},
  /* 62 */ {4, "r62", 0},
  /* 63 */ {4, "r63", 0},

  /* 64 */ {4, "pc", 0},

/* Begin control register for dpx */
/* index += 65 */
  /*  0 */ {4, "ctl0", 0},
  /*  1 */ {4, "ctl1", 0},
  /*  2 */ {4, "ctl2", 0},
  /*  3 */ {4, "ctl3", 0},
  /*  4 */ {4, "ctl4", 0},
  /*  5 */ {4, "cpuid", 0},
  /*  6 */ {4, "ctl6", 0},
  /*  7 */ {4, "ctl7", 0},
  /*  8 */ {4, "ctl8", 0},
  /*  9 */ {4, "ctl9", 0},
  /* 10 */ {4, "ctl10", 0},
  /* 11 */ {4, "ctl11", 0},
  /* 12 */ {4, "ctl12", 0},
  /* 13 */ {4, "ctl13", 0},
  /* 14 */ {4, "ctl14", 0},
  /* 15 */ {4, "ctl15", 0},
  /* 16 */ {4, "threadnum", 0},
  /* 17 */ {4, "ctl17", 0},
  /* 18 */ {4, "ctl18", 0},
  /* 19 */ {4, "ctl19", 0},
  /* 20 */ {4, "ctl20", 0},
  /* 21 */ {4, "ctl21", 0},
  /* 22 */ {4, "ctl22", 0},
  /* 23 */ {4, "ctl23", 0},
  /* 24 */ {4, "ctl24", 0},
  /* 25 */ {4, "ctl25", 0},
  /* 26 */ {4, "ctl26", 0},
  /* 27 */ {4, "ctl27", 0},
  /* 28 */ {4, "ctl28", 0},
  /* 29 */ {4, "ctl29", 0},
  /* 30 */ {4, "ctl30", 0},
  /* 31 */ {4, "ctl31", 0},
  /* 32 */ {4, "ctl32", 0},
  /* 33 */ {4, "ctl33", 0},
  /* 34 */ {4, "ctl34", 0},
  /* 35 */ {4, "ctl35", 0},
  /* 36 */ {4, "ctl36", 0},
  /* 37 */ {4, "ctl37", 0},
  /* 38 */ {4, "ctl38", 0},
  /* 39 */ {4, "ctl39", 0},
  /* 40 */ {4, "ctl40", 0},
  /* 41 */ {4, "ctl41", 0},
  /* 42 */ {4, "ctl42", 0},
  /* 43 */ {4, "ctl43", 0},
  /* 44 */ {4, "ctl44", 0},
  /* 45 */ {4, "ctl45", 0},
  /* 46 */ {4, "ctl46", 0},
  /* 47 */ {4, "ctl47", 0},
  /* 48 */ {4, "ctl48", 0},
  /* 49 */ {4, "ctl49", 0},
  /* 50 */ {4, "ctl50", 0},
  /* 51 */ {4, "ctl51", 0},
  /* 52 */ {4, "ctl52", 0},
  /* 53 */ {4, "ctl53", 0},
  /* 54 */ {4, "ctl54", 0},
  /* 55 */ {4, "ctl55", 0},
  /* 56 */ {4, "ctl56", 0},
  /* 57 */ {4, "ctl57", 0},
  /* 58 */ {4, "ctl58", 0},
  /* 59 */ {4, "ctl59", 0},
  /* 60 */ {4, "ctl60", 0},
  /* 61 */ {4, "ctl61", 0},
  /* 62 */ {4, "ctl62", 0},
  /* 63 */ {4, "ctl63", 0},
};

/* This array is a mapping from Dwarf-2 register
   numbering to GDB's */
static int nios2_dwarf2gdb_regno_map[] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11,
  12, 13, 14, 15,
  16, 17, 18, 19,
  20, 21, 22, 23,
  24, 25,
  GP_REGNUM,        /* 26 */
  SP_REGNUM,        /* 27 */
  FP_REGNUM,        /* 28 */
  EA_REGNUM,        /* 29 */
  BA_REGNUM,        /* 30 */
  RA_REGNUM,        /* 31 */
  32, 33 , 34,
  35,   /* 35 */
  36,   /* 36 */
  37,   /* 37 */
  38,   /* 38 */
  39,   /* CTL6 */ /* 39 */
  40,   /* 40 */
  41,   /* 41 */
  42,   /* 42 */
  43,   /* 43 */
  44,   /* 44 */
  45,   /* 45 */
  46,   /* 46 */
  47,   /* 47 */
  48,   /* 48 */
  49, 50, 51, 52,
  53, 54, 55, 56,
  57, 58, 59, 60,
  61, 62, 63,
  PC_REGNUM,
  65, 66, 67, 68, 69,
  CPUID_REGNUM, 
  71,
  72,
  73, 74, 75, 76,
  78, 79, 80,
  THREADNUM_REGNUM,
  82, 83, 84, 85,
  86, 87, 88, 89,
  90, 91, 92, 93,
  94, 95, 96, 97,
  98, 99, 100, 101,
  102, 103, 104, 105,
  102, 103, 104, 105,
  106, 107, 108, 109,
  110, 111, 112, 113,
  114, 115, 116, 117,
  118, 119, 120, 121,
  122, 123, 124, 125,
  126, 127, 128
};

/* architecture specific state info */
struct gdbarch_tdep
{
  int extension_register_cfg;
};


/* Dwarf-2 <-> GDB register numbers mapping.  */
int
nios2_dwarf_reg_to_regnum (struct gdbarch *gdbarch, int dw_reg)
{
  if (dw_reg < 0 || dw_reg > NIOS2_NUM_REGS)
    {
      warning ("Dwarf-2 uses unmapped register #%d\n", dw_reg);
      return dw_reg;
    }

  return nios2_dwarf2gdb_regno_map[dw_reg];
}

/* Same as read_memory_u6signed_integer, but don't report an error if
   can't read.  */
int
nios2_read_memory_unsigned_integer (CORE_ADDR memaddr, int len, ULONGEST *ret, struct gdbarch *gdbarch)
{
  char buf[sizeof (ULONGEST)];
  int error;

  error = target_read_memory (memaddr, buf, len);
  if (error != 0)
    return 0;
  else
    {
      *ret = extract_unsigned_integer (buf, len, gdbarch_byte_order (gdbarch));
      return 1;
    }
}

/* Find the name for the specified NIOS2 regno */
static const char *
nios2_register_name (struct gdbarch *gdbarch, int regno)
{
  static char alias[NIOS2_MAX_REG_DISPLAYED_REGNUM][16];
  int ext_reg_cfg = gdbarch_tdep (gdbarch)->extension_register_cfg;      

  /* Don't display any registers after NIOS2_MAX_REG_DISPLAYED_REGNUM */
  if (regno < 0)
    return NULL;
  else if (regno > NIOS2_MAX_REG_DISPLAYED_REGNUM)
    return NULL;

  if (ext_reg_cfg >= 0)
    {
      if (nios2_get_register_alias(alias[regno], 16, nios2_register_info_table[regno].name, ext_reg_cfg, 0) > 0)
        return alias[regno];
    }

  return nios2_register_info_table[regno].name;
}

/* nios2_register_byte_table[i] is the offset into the register file of the
   start of register number i.  We initialize this from
   nios2_register_info_table.  */
int nios2_register_byte_table[NIOS2_NUM_REGS];

/* Index within `registers' of the first byte of the space for register REGNO.  */
int
nios2_register_byte (int regno)
{
  return nios2_register_byte_table[regno];
}

/* Number of bytes of storage in the actual machine representation for
   register 'regno'.  */
static int
nios2_register_raw_size (int regno)
{
  return nios2_register_info_table[regno].size;
}

/* Number of bytes of storage in the program's representation for
   register 'regno'. */
static int
nios2_register_virtual_size (int regno)
{
    return nios2_register_info_table[regno].size;
}

/* Return the GDB type object for the "standard" data type
   of data in register 'regno'.  */
static struct type *
nios2_register_type (struct gdbarch *gdbarch, int regno)
{
/* we're hacking this, but if all the types we have is uint32, we're safe */
return builtin_type(gdbarch)->builtin_uint32;
/*
  return *nios2_register_info_table[regno].type;*/
}

/* Given a return value in `regcache' with a type `valtype',
   extract and copy its value into `valbuf'. */
/* This guy depends on what we have defined for each type.
   eg. we've defined int as 32 bits, TYPE_LENGTH will return 32 for valtype int passed in
   Also, how we extract return value (ie. read from r2 and r3) is the same as Nios II.
*/
void
nios2_extract_return_value (struct type *valtype, struct regcache *regcache,
			    gdb_byte *valbuf)
{
  int len = TYPE_LENGTH (valtype);

  /* pointer types are returned in register r2,
     up to 32-bit types in r2
     up to 64-bit types in r2,r3  */
  if (len <= nios2_register_raw_size (R2_REGNUM)) {
    regcache_cooked_read (regcache, R2_REGNUM, valbuf);
    // we're going to need to fix the endianness
    // iff we get half word.
    if(len == 2) {
    	valbuf[1] = valbuf[3];
    	valbuf[0] = valbuf[2];
    	// we discarded the value of 0 and 1, they're useless to us
    }
  }
  else if (len <= (nios2_register_raw_size (R2_REGNUM)
		  + nios2_register_raw_size (R3_REGNUM)))
    {
      regcache_cooked_read (regcache, R2_REGNUM, valbuf);
      regcache_cooked_read (regcache, R3_REGNUM, valbuf+4);
    }
}

/* Write into appropriate registers a function return value
   of type TYPE, given in virtual format. */
void
nios2_store_return_value (struct type *valtype, struct regcache *regcache,
			  const gdb_byte *valbuf)
{
  int len = TYPE_LENGTH (valtype);

  /* return values of up to 8 bytes are returned in $r2 $r3 */

  if (len <= nios2_register_raw_size (R2_REGNUM))
    regcache_cooked_write (regcache, R2_REGNUM, valbuf);
  else if (len <= (nios2_register_raw_size (R2_REGNUM)
		   + nios2_register_raw_size (R3_REGNUM)))
    {
      regcache_cooked_write (regcache, R2_REGNUM, valbuf);
      regcache_cooked_write (regcache, R3_REGNUM, valbuf+4);
    }
}

/* This function analyzes the function prologue and tries to work
   out where registers are saved and how long the prologue is.
   The prologue will consist of the following parts:
     1) Optional profiling instrumentation.  The old version uses six
        instructions.  We step over this if there is an exact match.
	  nextpc r8
	  mov	 r9, ra
	  movhi	 r10, %hiadj(.LP2)
	  addi	 r10, r10, %lo(.LP2)
	  call	 mcount
	  mov	 ra, r9
	The new version uses two or three instructions (the last of
	these might get merged in with the STW which saves RA to the
	stack).  We interpret these.
	  mov	 r8, ra
	  call	 mcount
	  mov	 ra, r8

     2) Optional interrupt entry decision.  Again, we step over
        this if there is an exact match.
	  rdctl  et,estatus
	  andi   et,et,1
	  beq    et,zero, <software_exception>
	  rdctl  et,ipending
	  beq    et,zero, <software_exception>

     3) A stack adjustment or stack which, which will be one of:
	  addi   sp, sp, -constant
	or:
	  movi   r8, constant
	  sub    sp, sp, r8
	or
	  movhi  r8, constant
	  addi   r8, r8, constant
	  sub    sp, sp, r8
	or
	  movhi  rx, %hiadj(newstack)
	  addhi  rx, rx, %lo(newstack)
	  stw    sp, constant(rx)
	  mov    sp, rx

     4) An optional stack check, which can take either of these forms:
	  bgeu   sp, rx, +8
	  break  3
	or
	  bltu   sp, rx, .Lstack_overflow
	  ...
	.Lstack_overflow:
	  break  3

     5) Saving any registers which need to be saved.  These will
        normally just be stored onto the stack:
	  stw    rx, constant(sp)
	but in the large frame case will use r8 as an offset back
	to the cfa:
	  add    r8, r8, sp
	  stw    rx, -constant(r8)

	Saving control registers looks slightly different:
	  rdctl  rx, ctlN
	  stw    rx, constant(sp)

     6) An optional FP setup, either if the user has requested a
        frame pointer or if the function calls alloca.
        This is always:
	  mov    fp, sp

    The prologue instructions may be interleaved, and the register
    saves and FP setup can occur in either order.

    To cope with all this variability we decode all the instructions
    from the start of the prologue until we hit a branch, call or
    return.  For each of the instructions mentioned in 3, 4 and 5 we
    handle the limited cases of stores to the stack and operations
    on constant values.
 */

/* TODO:The irq and profiler is NOT DONE*/
typedef struct
{
  unsigned int insn;
  unsigned int mask;
} wild_insn;

static const wild_insn profiler_insn[] =
{
  { 0x30002015, 0x00000000 }, // nextpc r8
  { 0x37c02431, 0x00000000 }, // mov    r9,ra
  { 0xe00a0101, 0x0000FFFF }, // movhi  r10,257
  { 0x228a3308, 0x00000FFF }, // addi   r10,r10,-31992
  { 0x00000001, 0xFFFFF000 }, // call   <mcount>
  { 0x32407c31, 0x00000000 }  // mov    ra,r9
};


/*static const wild_insn irqentry_insn[] =
{
  { 0x0031307a, 0x00000000 }, // rdctl  et,estatus
  { 0xc600004c, 0x00000000 }, // andi   et,et,1
  { 0xc0000026, 0x003FFFC0 }, // beq    et,zero, <software_exception>
  { 0x0031313a, 0x00000000 }, // rdctl  et,ipending
  { 0xc0000026, 0x003FFFC0 }  // beq    et,zero, <software_exception>
};*/

/* this function initializes the cache to it's default values */
static void
nios2_setup_default(struct nios2_unwind_cache *cache)
{
  int i;

  for (i = 0; i < NIOS2_NUM_REGS; i++)
  {
    /* All registers start off holding their previous values */
    cache->reg_value[i].reg    = i;
    cache->reg_value[i].offset = 0;

    /* All registers start off not saved */
    cache->reg_saved[i].basereg = -1;
    cache->reg_saved[i].addr    = 0;
  }
}

static int
nios2_match_sequence(CORE_ADDR start_pc, const wild_insn * sequence, int count, struct frame_info *this_frame)
{
  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  CORE_ADDR pc = start_pc;
  int i;
  unsigned int insn;

  for (i = 0 ; i < count ; i++)
    {
      insn = read_memory_unsigned_integer (pc, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));
      if ((insn & ~sequence[i].mask) != sequence[i].insn)
	return 0;

      pc += NIOS2_OPCODE_SIZE;
    }

  return 1;
}

static unsigned long
nios2_fetch_instruction (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  return read_memory_unsigned_integer (pc, NIOS2_OPCODE_SIZE, byte_order);
}

CORE_ADDR
nios2_analyze_prologue (const CORE_ADDR start_pc, const CORE_ADDR current_pc,
             struct nios2_unwind_cache *cache, struct frame_info *next_frame)
{
  /* Maximum lines of prologue to check */
  /* Note that this number should not be too large, else we can potentially */
  /* end up iterating through unmapped memory */
  CORE_ADDR limit_pc = start_pc + 200;
  int regno;

  /* Does the frame set up the FP register? */
  int base_reg = 0;

  REG_VALUE * value = cache->reg_value;
  REG_VALUE temp_value[NIOS2_NUM_REGS];

  int i;

  /* Save the starting PC so we can correct the pc after running */
  /* through the prolog, using symbol info */
  CORE_ADDR pc = start_pc;

  /* Is this an exception handler? */
  int exception_handler = 0;

  /* What was the original value of SP (or fake original value for
   * functions which switch stacks?
   */
  CORE_ADDR frame_high;

  /* Is this the end of the prologue? */
  int within_prologue = 1;

  CORE_ADDR prologue_end;

  /* Is this the innermost function? */
  int innermost = (frame_relative_level(next_frame) < 0);

#ifdef DEBUG_PRINT
    fprintf_unfiltered (gdb_stdlog,
	    "{ nios2_analyze_prologue start=0x%s, current=0x%s ",
	    paddr_nz (start_pc), paddr_nz (current_pc));
#endif

  /* Set up the default values of the registers. */
  nios2_setup_default(cache);

  /* If the first few instructions are the profile entry then skip over them. */
  /* Newer versions of the compiler use more efficient profiling code. */
  /* Toolchain doesn't support profiling, but in the future we don't want to update gdb to support this*/
  if (nios2_match_sequence(pc, profiler_insn, sizeof(profiler_insn)/sizeof(profiler_insn[0]), next_frame));
    pc += sizeof(profiler_insn)/sizeof(profiler_insn[0]) * NIOS2_OPCODE_SIZE;

  /* If the first few are an interrupt entry then skip over them too
  no we don't have interrupts in dpx */
  /*if (nios2_match_sequence(pc, irqentry_insn, sizeof(irqentry_insn)/sizeof(irqentry_insn[0]), next_frame));
    {
      pc += sizeof(irqentry_insn)/sizeof(irqentry_insn[0]) * NIOS2_OPCODE_SIZE;
      exception_handler = 1;
    }*/

  prologue_end = start_pc;

  /* Find the prologue instructions.  */
  /* Fortunately we're in 32bit paradise */
  while (pc < limit_pc && within_prologue)
    {
      /* Present instruction.  */
      unsigned int insn;
      struct gdbarch *gdbarch = get_frame_arch (next_frame);

      int prologue_insn = 0;

      if (pc == current_pc)
      {
	/*
	 * When we reach the current PC we must save the current register
	 * state (for the backtrace) but keep analysing because there might
	 * be more to find out (eg. is this an exception handler).
	 */
	memcpy(temp_value, value, sizeof(temp_value));
	value = temp_value;
#ifdef DEBUG_PRINT
	fprintf_unfiltered (gdb_stdlog, "*");
#endif
      }

      //insn = read_memory_unsigned_integer (pc, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));
      insn = nios2_fetch_instruction(gdbarch, pc);
      pc += NIOS2_OPCODE_SIZE;

#ifdef DEBUG_PRINT
      fprintf_unfiltered (gdb_stdlog, "[%08X]", insn);
#endif

      /* The following instructions can appear in the prologue */

      /* if ((insn & 0x0001FFFF) == 0x0001883A) */
      if( GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_ADD)
	{
	  /* ADD   rc, ra, rb  (also used for MOV) */

	  int ra = GET_IW_A(insn);
	  int rb = GET_IW_B(insn);
	  int rc = GET_IW_C(insn);

	  if (rc == SP_REGNUM && rb == 0 && value[ra].reg == cache->reg_saved[SP_REGNUM].basereg)
	    {
	      /* If the previous value of SP is available somewhere near the new
	       * stack pointer value then this is a stack switch.
	       */

	      /* If any registers were saved on the stack before then we can't backtrace
	       * into them now.
	       */
	      for (i = 0 ; i < NIOS2_NUM_REGS ; i++)
		{
		  if (cache->reg_saved[i].basereg == SP_REGNUM)
		    cache->reg_saved[i].basereg = -1;
		  if (value[i].reg == SP_REGNUM)
		    value[i].reg = -1;
		}

	      /* Create a fake "high water mark" 4 bytes above where SP was stored
	       * and fake up the registers to be consistent with that.
	       */
	      value[SP_REGNUM].reg = SP_REGNUM;
	      value[SP_REGNUM].offset = value[ra].offset - cache->reg_saved[SP_REGNUM].addr - 4;

	      cache->reg_saved[SP_REGNUM].basereg = SP_REGNUM;
	      cache->reg_saved[SP_REGNUM].addr = -4;
	    }

	  else if (rc != 0)
	    {
	      if (value[rb].reg == 0)
		value[rc].reg = value[ra].reg;
	      else if (value[ra].reg == 0)
		value[rc].reg = value[rb].reg;
	      else
		value[rc].reg = -1;

	      value[rc].offset = value[ra].offset + value[rb].offset;
	    }
	  prologue_insn = 1;
	}

      /* else if ((insn & 0x0001FFFF) == 0x0001983A) */
      else if( GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_SUB)
	{
	  /* SUB   rc, ra, rb */

	  int ra = GET_IW_A(insn);
	  int rb = GET_IW_B(insn);
	  int rc = GET_IW_C(insn);

	  if (rc != 0)
	    {
	      if (value[rb].reg == 0)
		value[rc].reg = value[ra].reg;
	      else
		value[rc].reg = -1;

	      value[rc].offset = value[ra].offset - value[rb].offset;
	    }
	}

      /* else if ((insn & 0x0000003F) == 0x00000004) */
      else if( GET_IW_OP(insn) == OP_I12B && GET_IW_I12(insn) == I12B_ADDI)
	{
	  /* ADDI  rb, ra, immed   (also used for MOVI) */
	  short immed = GET_IW_IMM16(insn);
	  int ra = GET_IW_A(insn);
	  int rb = GET_IW_B(insn);

	  if (rb == SP_REGNUM)
	    {
	      /* The first stack adjustment is part of the prologue.  Any subsequent
	         stack adjustments are either down to alloca or the epilogue so stop
		 analysing when we hit them. */
	      if (value[rb].offset != 0 || value[ra].reg != SP_REGNUM)
		break;
	    }

	  if (rb != 0)
	    {
	      value[rb].reg    = value[ra].reg;
	      value[rb].offset = value[ra].offset + immed;
	    }

	  prologue_insn = 1;
	}

     /* else if ((insn & 0x0000003F) == 0x00000034) */
     else if( GET_IW_OP(insn) == OP_ORHI)
	{
	  /* ORHI  rb, ra, immed   (also used for MOVHI) */
	  unsigned int immed = GET_IW_IMM16(insn);
	  int ra = GET_IW_A(insn);
	  int rb = GET_IW_B(insn);

	  if (rb != 0)
	    {
  	      value[rb].reg    = (value[ra].reg == 0) ? 0 : -1;
	      value[rb].offset = value[ra].offset | (immed << 16);
	    }
	}

      /* else if (((insn & IW_OP_MASK) == OP_STW || (insn & IW_OP_MASK) == OP_STWIO)) */
      else if( GET_IW_OP(insn) == OP_I12B && ( GET_IW_I12(insn) == I12B_STW) )
        {
	  /* STW rb, immediate(ra) */

	  short immed16 = GET_IW_IMM16(insn);
	  int ra = GET_IW_A(insn);
	  int rb = GET_IW_B(insn);

	  /* Are we storing the original value of a register?  For exception handlers
	   * the value of EA-4 (return address from interrupts etc) is sometimes stored.
	   */
	  int orig = value[rb].reg;
	  if (orig > 0 && (value[rb].offset == 0 || (orig == EA_REGNUM && value[rb].offset == -4)))
	    {
	      /* We are most interested in stores to the stack, but will also take note
	       * of stores to other places as they might be useful later.
	       */
	      if ((value[ra].reg == SP_REGNUM && cache->reg_saved[orig].basereg != SP_REGNUM) ||
		  cache->reg_saved[orig].basereg == -1)
		{
		  if (pc < current_pc)
		    {
		      /* Save off callee saved registers */
		      cache->reg_saved[orig].basereg = value[ra].reg;
		      cache->reg_saved[orig].addr    = value[ra].offset + GET_IW_IMM16(insn);
		    }

		  prologue_insn = 1;

		  if (orig == EA_REGNUM) // || orig == ESTATUS_REGNUM)
		    exception_handler = 1;
		}
	    }
        }

      /* else if ((insn & 0xFFC1F83F) == 0x0001303A) */
      else if( GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_RDCTL)
	{
	  /* RDCTL rC, ctlN */
/*	  int rc = GET_IW_C(insn);
	  int n = GET_IW_CONTROL_REGNUM(insn);

	  if (rc != 0)
	    {
	      value[rc].reg    = STATUS_REGNUM + n;
	      value[rc].offset = 0;
	    }

	  prologue_insn = 1;*/
        }

      /*else if ((insn & 0x0000003F) == 0 &&
		value[8].reg == RA_REGNUM && value[8].offset == 0 &&
		value[SP_REGNUM].reg == SP_REGNUM && value[SP_REGNUM].offset == 0)*/
		else if( GET_IW_OP(insn) == OP_IX && GET_IW_IX(insn) == IX_CALL
		&&
		value[8].reg == RA_REGNUM && value[8].offset == 0 &&
		value[SP_REGNUM].reg == SP_REGNUM && value[SP_REGNUM].offset == 0)
	{
	  /* A CALL instruction.  This is treated as a call to mcount if ra
	   * has been stored into r8 beforehand and if it's before the stack adjust.
	   * mcount corrupts r2-r3, r9-r15 & ra
	   */
	  for (i = 2 ; i <= 3 ; i++)
	    value[i].reg = -1;
	  for (i = 9 ; i <= 15 ; i++)
	    value[i].reg = -1;
	  value[RA_REGNUM].reg = -1;

	  prologue_insn = 1;
	}

      /* else if ((insn & 0xF83FFFFF) == 0xD800012E) */
      else if ( GET_IW_OP(insn) == OP_I12A && GET_IW_I12(insn) == I12A_BGEU
      && GET_IW_A(insn) == SP_REGNUM /*&& GET_IW_B(insn) == RX_MASK*/ && GET_IW_C(insn) == 8)
	{
	   /* BGEU sp, rx, +8
	    * BREAK 3
	    * This instruction sequence is used in stack checking - we can ignore it
	    */
	  //unsigned int next_insn = read_memory_unsigned_integer(pc, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));
	  unsigned int next_insn = nios2_fetch_instruction(gdbarch, pc);

	  /*if (next_insn != 0x003DA0FA)*/
	  if( GET_IW_OP(insn) != OP_I12A && GET_IW_I12(insn) != I12A_I5 && GET_IW_I5(insn) != I5_BREAK && GET_IW_IMM5(insn) != 3)
	    within_prologue = 0;
	  else
	    pc += NIOS2_OPCODE_SIZE;
	}

      /*else if ((insn & 0xF800003F) == 0xD8000036)*/
      else if( GET_IW_OP(insn) == OP_I12A && GET_IW_I12(insn) == I12A_BLTU)
	{
	   /* BLTU sp, rx, .Lstackoverflow
	    * If the location branched to holds a BREAK 3 instruction then this is
	    * also stack overflow detection.  We can ignore it.
	    */
	  CORE_ADDR target_pc = pc + ((insn & 0x3FFFC0) >> 6);
	  /*unsigned int target_insn = read_memory_unsigned_integer(target_pc, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));*/
	  unsigned int target_insn = nios2_fetch_instruction(gdbarch, target_pc);

	  /*if (target_insn != 0x003DA0FA)*/
	  if( GET_IW_OP(insn) != OP_I12A && GET_IW_I12(insn) != I12A_I5 && GET_IW_I5(insn) != I5_BREAK && GET_IW_IMM5(insn) != 3)
	    within_prologue = 0;
	}

      /* Any other instructions are allowed to be moved up into the prologue.  If we
         reach a branch, call or return then the prologue is considered over */
      /* We also consider a second stack adjustment as terminating the prologue (see
         above) */

      else
	{
	  switch (GET_IW_OP(insn))
	    {
	        switch( GET_IW_I12(insn)) {
	            case I12A_BEQ:
	            case I12A_BGE:
	            case I12A_BGEU:
	            case I12A_BLT:
	            case I12A_BLTU:
	            case I12A_BNE:
                    within_prologue = 0;
                default:
                    break;
	        }
	        switch( GET_IW_IX(insn)) {
	            case IX_CALL:
                    within_prologue = 0;
                default:
                    break;
	        }

	        switch( GET_IW_R3(insn)) {
	            case R3_RET:
	            case R3_ERET:
	            case R3_BRET:
	            case R3_CALLR:
	            case R3_JMP:
                    within_prologue = 0;
                default:
                    break;
	        }
	    default:
	      break;
	    }
	}

      if (prologue_insn)
	prologue_end = pc;
    }

  /* Are we within the function epilogue?  If so then we should go back to the
     original register values */
  if (innermost && current_pc > start_pc)
    {
      struct gdbarch *gdbarch = get_frame_arch (next_frame);
      /* First check whether the previous instruction was a stack
         adjustment. */
      /*unsigned int insn = read_memory_unsigned_integer (current_pc - NIOS2_OPCODE_SIZE, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));*/
      unsigned int insn = nios2_fetch_instruction(gdbarch, current_pc - NIOS2_OPCODE_SIZE);

      /* if ((insn & 0xFFC0003C) == 0xDEC00004 ||*/ /* ADDI sp, sp, */
      /* (insn & 0xFFC1FFFF) == 0xDEC1883A ||*/ /* ADD  sp, sp, */
      /* (insn & 0xFFC0003F) == 0xDEC00017)*/  /* LDW  sp, constant(sp) */
	 if( (GET_IW_OP(insn) == OP_I12B && GET_IW_I12(insn) == I12B_ADDI && GET_IW_A(insn) == SP_REGNUM && GET_IW_B(insn) == SP_REGNUM) || /* ADDI sp, sp, */
	 (GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_ADD && GET_IW_A(insn) == SP_REGNUM && GET_IW_B(insn) == SP_REGNUM) || /* ADD  sp, sp, */
	 (GET_IW_OP(insn) == OP_I12B && GET_IW_I12(insn) == I12B_LDW && GET_IW_B(insn) == SP_REGNUM && GET_IW_A(insn) == SP_REGNUM) ) /* LDW  sp, constant(sp) */
	{
	  /* Then check if it's followed by a return or a tail call */
          insn = read_memory_unsigned_integer (current_pc, NIOS2_OPCODE_SIZE, gdbarch_byte_order (gdbarch));

	  /*if (insn == 0xF800283A || */               /* RET */
	  /*    insn == 0xE800083A || */               /* ERET */
	  /*    (insn & 0x07FFFFFF) == 0x0000683A || */ /* JMP */
	  /*    (insn & 0xFFC0003F) == 6) */            /* BR */
	  if((GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_RET) || /* ret */
	  (GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_ERET) || /* eret */
	  (GET_IW_OP(insn) == OP_R3 && GET_IW_R3(insn) == R3_JMP) /* jmp */
	  )
	    {
	      /* Go back to default value of the registers */
	      nios2_setup_default(cache);
	    }
	}
    }

  /* Exception handlers use a different return address register. */
  /* No we don't have exception handler in dpx */
  if (exception_handler)
    cache->return_regnum = EA_REGNUM;

#ifdef DEBUG_PRINT
  fprintf_unfiltered (gdb_stdlog, "\n-> retreg=%d, ", cache->return_regnum);
#endif

  if (cache->reg_value[FP_REGNUM].reg == SP_REGNUM)
    {
      /* If the FP now holds an offset from the CFA then this is a normal
         frame which uses the frame pointer. */
      base_reg = FP_REGNUM;
    }
  else if (cache->reg_value[SP_REGNUM].reg == SP_REGNUM)
    {
      /* FP doesn't hold an offset from the CFA.  If SP still holds an
         offset from the CFA then we might be in a function which omits
	 the frame pointer, or we might be partway through the prologue.
	 In both cases we can find the CFA using SP. */
      base_reg = SP_REGNUM;
    }
  else
    {
      /* Somehow the stack pointer has been corrupted. We can't return. */
#ifdef DEBUG_PRINT
      fprintf_unfiltered (gdb_stdlog, "<can't reach cfa> }\n");
#endif
      return 0;
    }

  if (cache->reg_value[base_reg].offset == 0 ||
      cache->reg_saved[RA_REGNUM].basereg != SP_REGNUM ||
      cache->reg_saved[cache->return_regnum].basereg != SP_REGNUM)
    {
      /*
       * If the frame didn't adjust the stack, didn't save RA or didn't save
       * EA in an exception handler then it must either be a leaf function
       * (doesn't call any other functions) or it can't return.  If it has
       * called another function then it can't be a leaf, so set base == 0
       * to indicate that we can't backtrace past it.
       */

      if (!innermost)
	{
	  /* If it isn't the innermost function then it can't be a leaf, unless
	   * it was interrupted.  Check whether RA for this frame is the same
	   * as PC.  If so then it probably wan't interrupted.
	   */
	  char buf[8];
	  CORE_ADDR ra;
	  struct type *ptr_type = builtin_type (target_gdbarch)->builtin_func_ptr;

	  frame_unwind_register (next_frame, RA_REGNUM, buf);
	  ra = extract_typed_address (buf, ptr_type); /*builtin_type_void_func_ptr);*/

	  if (ra == current_pc)
	    {
#ifdef DEBUG_PRINT
	      fprintf_unfiltered (gdb_stdlog, "<noreturn ADJUST 0x%s, r31@r%d+?>, r%d@r%d+?> }\n",
		                  paddr_nz(cache->reg_value[base_reg].offset),
				  cache->reg_saved[RA_REGNUM].basereg,
				  cache->return_regnum, cache->reg_saved[cache->return_regnum].basereg);
#endif
	      return 0;
	    }
	}
    }

  /* Get the value of whichever register we are using for the base. */
  cache->base = frame_unwind_register_unsigned (next_frame, base_reg);

  /* What was the value of SP at the start of this function (or just
   * after the stack switch).
   */
  frame_high = cache->base - cache->reg_value[base_reg].offset;

  /* Adjust all the saved registers such that they contain addresses
     instead of offsets.  */
  for (i = 0; i < NIOS2_NUM_REGS; i++)
    if (cache->reg_saved[i].basereg == SP_REGNUM)
      {
	cache->reg_saved[i].basereg = Z_REGNUM;
	cache->reg_saved[i].addr += frame_high;
      }

  for (i = 0; i < NIOS2_NUM_REGS; i++)
    if (cache->reg_saved[i].basereg == GP_REGNUM)
      {
	CORE_ADDR gp = frame_unwind_register_unsigned (next_frame, GP_REGNUM);

	for ( ; i < NIOS2_NUM_REGS; i++)
	  if (cache->reg_saved[i].basereg == GP_REGNUM)
	    {
	      cache->reg_saved[i].basereg = Z_REGNUM;
	      cache->reg_saved[i].addr += gp;
	    }
      }

  /* Work out what the value of SP was on the first instruction of this
   * function.  If we didn't switch stacks then this can be trivially
   * computed from the base address.
   */
  if (cache->reg_saved[SP_REGNUM].basereg == Z_REGNUM)
    {
      struct gdbarch *gdbarch = get_frame_arch (next_frame);
      cache->cfa = read_memory_unsigned_integer(cache->reg_saved[SP_REGNUM].addr, 4, gdbarch_byte_order (gdbarch));
    }
  else
    cache->cfa = frame_high;

  /* Exception handlers restore ESTATUS into STATUS. */
  /* No we don't have exception handler in Nios dpx*/
  /*if (exception_handler)
  {
    cache->reg_saved[STATUS_REGNUM] = cache->reg_saved[ESTATUS_REGNUM];
    cache->reg_saved[ESTATUS_REGNUM].basereg = -1;
  }*/

#ifdef DEBUG_PRINT
  fprintf_unfiltered (gdb_stdlog, "cfa=0x%s }\n", paddr_nz(cache->cfa));
#endif

  return prologue_end;
}

struct frame_info *
setup_arbitrary_frame (int argc, CORE_ADDR *argv)
{
  if (argc != 2)
    error ("Nios II frame specifications require two arguments: sp and pc");

  return create_new_frame (argv[0], argv[1]);
}

/* Given a PC value corresponding to the start of a function, return the PC
   of the first instruction after the function prologue.  */
CORE_ADDR
nios2_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR start_pc)
{
  CORE_ADDR func_addr, func_end;
  struct symtab_and_line sal;
  CORE_ADDR pc_after_prologue;

  /* If we have line debugging information, then the end of the prologue
     should the first assembly instruction of the first source line.  */
  if (find_pc_partial_function (start_pc, NULL, &func_addr, &func_end))
    {
      sal = find_pc_line (func_addr, 0);
      if (sal.end > 0 && sal.end < func_end)
	return sal.end;
    }

  return start_pc;
}

const unsigned char*
nios2_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *bp_addr, int *bp_size)
{
  /* break encoding: 31->27  26->22  21->17  16->11 10->6 5->0 */
  /*                 00000   00000   0x1d    0x2d   11111 0x3a */
  /*                 00000   00000   11101   101101 11111 111010 */
  /* In bytes:       00000000 00111011 01101111 11111010 */
  /*                 0x0       0x3b    0x6f     0xfa */

  /* dpx break encoding
    0001 0000 0001 1110 0000 0000 1010 0000
         0x10      0x0e       0x0      0xa0
  */
  static unsigned char breakpoint[] = {0xa0, 0x0, 0x0e, 0x10};
   *bp_size = 4;
   return breakpoint;
}


int
gdb_print_insn_nios2 (bfd_vma memaddr, disassemble_info *info)
{
  if (info->endian == BFD_ENDIAN_BIG)
    {

      return print_insn_big_nios2 (memaddr, info);
    }
  else
    {
      return print_insn_little_nios2 (memaddr, info);
    }
}



/* Adjust the address downward (direction of stack growth) so that it
   is correctly aligned for a new stack frame.  */

/* ??? Big endian issues here? */
static CORE_ADDR
nios2_frame_align (struct gdbarch *gdbarch, CORE_ADDR addr)
{
  return align_down (addr, 4);
}



/* We don't convert anything at the moment */
static int
nios2_convert_register_p (struct gdbarch *gdbarch, int regnum, struct type *type)
{
  return 0;
}

static void
nios2_register_to_value (struct frame_info *frame, int regnum,
			 struct type *type, gdb_byte *to)
{
  /* Why do we need to call this function twice? */
  get_frame_register (frame, regnum + 0, (char *) to + 0);
  get_frame_register (frame, regnum + 0, (char *) to + 0);
}

static void
nios2_value_to_register (struct frame_info *frame, int regnum,
			 struct type *type, const gdb_byte *from)
{
  put_frame_register (frame, regnum + 0, (const char *) from + 0);
  put_frame_register (frame, regnum + 0, (const char *) from + 0);
}

/* Determine, for architecture GDBARCH, how a return value of TYPE
   should be returned.  If it is supposed to be returned in registers,
   and READBUF is non-zero, read the appropriate value from REGCACHE,
   and copy it into READBUF.  If WRITEBUF is non-zero, write the value
   from WRITEBUF into REGCACHE.  */

/*
if the readbuf is non-NULL, it reads the type from r2 and r3 (Nios II architecture)
if writebuf is non-NULL, then writes to r2 and r3 the type (Nios II architecture)
*/
static enum return_value_convention
nios2_return_value (struct gdbarch *gdbarch, struct type *func_type,
		    struct type *type,
		    struct regcache *regcache, gdb_byte *readbuf,
		    const gdb_byte *writebuf)
{
  enum type_code code = TYPE_CODE (type);

  if (TYPE_LENGTH (type) > 8)
    return RETURN_VALUE_STRUCT_CONVENTION;


  if (readbuf)
    nios2_extract_return_value (type, regcache, readbuf);
  if (writebuf)
    nios2_store_return_value (type, regcache, writebuf);

  return RETURN_VALUE_REGISTER_CONVENTION;
}

/* Assuming NEXT_FRAME->prev is a dummy, return the frame ID of that
   dummy frame.  The frame ID's base needs to match the TOS value
   saved by save_dummy_frame_tos() and returned from
   nios2_push_dummy_call, and the PC needs to match the dummy frame's
   breakpoint.  */
/*
    Nothing needs to be changed from Nios II to dpx as far as I understand 5/28/10
*/
static struct frame_id
nios2_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  return frame_id_build (
                         get_frame_register_unsigned (this_frame,
						      SP_REGNUM),
			 get_frame_pc (this_frame));
}

/* this function is used to allow gdb to call functions while debugging (dummy calls)
   it actually pushes the args to the registers (and stack) and return the sp for gdb to call the function */
static CORE_ADDR
nios2_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
                       struct regcache *regcache, CORE_ADDR bp_addr,
                       int nargs, struct value **args, CORE_ADDR sp,
                       int struct_return, CORE_ADDR struct_addr)
{
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  int argreg;
  int float_argreg;
  int argnum;
  int len = 0;
  int stack_offset = 0;
  CORE_ADDR func_addr = find_function_addr (function, NULL);

  /* Set the return address register to point to the entry point of
     the program, where a breakpoint lies in wait.  */
  regcache_cooked_write_signed (regcache, RA_REGNUM, bp_addr);

  /* Now make space on the stack for the args.  */
  for (argnum = 0; argnum < nargs; argnum++)
    len += align_up (TYPE_LENGTH (value_type (args[argnum])), 4);
  sp -= len;

  /* Initialize the register pointer.  */
  argreg = FIRST_ARGREG;

  /* The struct_return pointer occupies the first parameter-passing reg.  */
  if (struct_return)
    regcache_cooked_write_unsigned (regcache, argreg++, struct_addr);

  /* Now load as many as possible of the first arguments into
     registers, and push the rest onto the stack.  Loop thru args
     from first to last.  */
  for (argnum = 0; argnum < nargs; argnum++)
    {
      const gdb_byte *val;
      gdb_byte valbuf[MAX_REGISTER_SIZE];
      struct value *arg = args[argnum];
      struct type *arg_type = check_typedef (value_type (arg));
      int len = TYPE_LENGTH (arg_type);
      enum type_code typecode = TYPE_CODE (arg_type);

      val = value_contents (arg);

      /* Copy the argument to general registers or the stack in
         register-sized pieces.  Large arguments are split between
         registers and stack.  */
      while (len > 0)
        {
	  int partial_len = (len < 4 ? len : 4);

	  if (argreg <= LAST_ARGREG)
	    {
	      /* The argument is being passed in a register.  */
	      CORE_ADDR regval = extract_unsigned_integer (val, partial_len, byte_order);
	      regcache_cooked_write_unsigned (regcache, argreg, regval);
	      argreg++;
	    }
	  else
	    {
	      /* The argument is being passed on the stack.  */
	      CORE_ADDR addr = sp + stack_offset;
	      write_memory (addr, val, partial_len);
	      stack_offset += align_up (partial_len, 4);
	    }

	  len -= partial_len;
	  val += partial_len;
	}
    }

  regcache_cooked_write_signed (regcache, SP_REGNUM, sp);

  /* Return adjusted stack pointer.  */
  return sp;
}

static CORE_ADDR
nios2_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  char buf[8];

  frame_unwind_register (next_frame, PC_REGNUM, buf);
  return extract_typed_address (buf, builtin_type (target_gdbarch)->builtin_func_ptr);
}

static CORE_ADDR
nios2_unwind_sp (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  return frame_unwind_register_unsigned (this_frame, SP_REGNUM);
}


/* Frame base handling.  */

struct nios2_unwind_cache *
nios2_frame_unwind_cache (struct frame_info *next_frame,
			 void **this_prologue_cache)
{
  CORE_ADDR current_pc;
  struct nios2_unwind_cache *cache;
  int i;

  if (*this_prologue_cache)
    return *this_prologue_cache;

  cache = FRAME_OBSTACK_ZALLOC (struct nios2_unwind_cache);
  (*this_prologue_cache) = cache;

  /* Zero all fields.  */
  cache->base = 0;
  cache->cfa = 0;
  cache->pc = 0;

  cache->return_regnum = RA_REGNUM;

  nios2_setup_default(cache);

  cache->pc = get_frame_func (next_frame);
  current_pc = get_frame_pc (next_frame);

  /* Prologue analysis does the rest...  */
  if (cache->pc != 0)
    nios2_analyze_prologue (cache->pc, current_pc, cache, next_frame);

  return cache;
}

static void
nios2_frame_this_id (struct frame_info *next_frame, void **this_cache,
		     struct frame_id *this_id)
{
  struct nios2_unwind_cache *cache =
    nios2_frame_unwind_cache (next_frame, this_cache);

  /* This marks the outermost frame.  */
  if (cache->base == 0)
    return;

  (*this_id) = frame_id_build (cache->cfa, cache->pc);
}

static struct value *
nios2_frame_prev_register (struct frame_info *this_frame, void **this_cache,
			   int regnum)
{
  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  struct nios2_unwind_cache *cache =
    nios2_frame_unwind_cache (this_frame, this_cache);

  gdb_assert (regnum >= 0);

  if (regnum == gdbarch_sp_regnum (gdbarch) && cache->cfa)
    return frame_unwind_got_constant (this_frame, regnum, cache->cfa);

  /* If we've worked out where a register is stored then load it from there.
   */
  if (regnum < NIOS2_NUM_REGS && cache->reg_saved[regnum].basereg == Z_REGNUM)
    return frame_unwind_got_memory (this_frame, regnum,
				    cache->reg_saved[regnum].addr);

  return frame_unwind_got_register (this_frame, regnum, regnum);
  }


static int
nios2_frame_sniffer (const struct frame_unwind *self,
			   struct frame_info *this_frame, void **this_cache)
{
  return 1;
}

static const struct frame_unwind nios2_frame_unwind =
{
  NORMAL_FRAME,
  nios2_frame_this_id,
  nios2_frame_prev_register,
  NULL,
  nios2_frame_sniffer
};

static CORE_ADDR
nios2_frame_base_address (struct frame_info *next_frame, void **this_cache)
{
  struct nios2_unwind_cache *info
    = nios2_frame_unwind_cache (next_frame, this_cache);
  return info->base;
}

static const struct frame_base nios2_frame_base =
{
  &nios2_frame_unwind,
  nios2_frame_base_address,
  nios2_frame_base_address,
  nios2_frame_base_address
};
/* --------------- FRAME BASE HANDLING ENDS --------------- */

/* --------------- BEGIN FRAME UNWINDER STUB -------------- */

static struct trad_frame_cache *
nios2_stub_frame_cache (struct frame_info *next_frame, void **this_cache)
{
  struct gdbarch *gdbarch = get_frame_arch (next_frame);
  CORE_ADDR pc;
  CORE_ADDR start_addr;
  CORE_ADDR stack_addr;
  struct trad_frame_cache *this_trad_cache;

  if ((*this_cache) != NULL)
    return (*this_cache);
  this_trad_cache = trad_frame_cache_zalloc (next_frame);
  (*this_cache) = this_trad_cache;

  /* The return address is in the link register.  */
  trad_frame_set_reg_realreg (this_trad_cache,
                              gdbarch_pc_regnum (gdbarch),
                              RA_REGNUM);

  /* Frame ID, since it's a frameless / stackless function, no stack
     space is allocated and SP on entry is the current SP.  */
  pc = get_frame_pc (next_frame);
  find_pc_partial_function (pc, NULL, &start_addr, NULL);
  stack_addr = get_frame_register_signed (next_frame, SP_REGNUM); //frame_unwind_register_signed (next_frame, SP_REGNUM);
  trad_frame_set_id (this_trad_cache, frame_id_build (stack_addr, start_addr));
  /* Assume that the frame's base is the same as the
     stack-pointer.  */
  trad_frame_set_this_base (this_trad_cache, stack_addr);

  return this_trad_cache;
}

static void
nios2_stub_frame_this_id (struct frame_info *next_frame, void **this_cache,
                          struct frame_id *this_id)
{
  struct trad_frame_cache *this_trad_cache
    = nios2_stub_frame_cache (next_frame, this_cache);
  trad_frame_get_id (this_trad_cache, this_id);
}

static struct value *
nios2_stub_frame_prev_register (struct frame_info *next_frame,
			        void **this_cache,
			        int regnum)
{
  struct trad_frame_cache *this_trad_cache
    = nios2_stub_frame_cache (next_frame, this_cache);
  return trad_frame_get_register (this_trad_cache, next_frame, regnum);
}

static int
nios2_stub_frame_sniffer (const struct frame_unwind *self,
			 struct frame_info *this_frame, void **this_cache)
{
  struct obj_section *s;
  CORE_ADDR pc = get_frame_address_in_block (this_frame);

  if (in_plt_section (pc, NULL))
    return 1;

  return 0;
}

static const struct frame_unwind nios2_stub_frame_unwind =
{
  NORMAL_FRAME,
  nios2_stub_frame_this_id,
  nios2_stub_frame_prev_register,
  NULL,
  nios2_frame_sniffer
};


#define SIGN_EXTEND12(x) (((signed)x<<20)>>20)


/* Determine where to set a single step breakpoint while considering
   branch prediction.  */
static CORE_ADDR
nios2_get_next_pc (struct frame_info *frame, CORE_ADDR pc)
{
  unsigned long inst;
  int op;
  /*int imm16;*/
  int imm12;
  int ra;
  int rb;
  int ras;
  int rbs;
  unsigned int rau;
  unsigned int rbu;

  struct gdbarch *gdbarch = get_frame_arch (frame);

  inst = nios2_fetch_instruction (gdbarch, pc);
  pc += NIOS2_OPCODE_SIZE;

  imm12 = (short) GET_IW_IMM12(inst);
  ra = GET_IW_A(inst);
  rb = GET_IW_B(inst);
  ras = get_frame_register_signed (frame, ra);
  rbs = get_frame_register_signed (frame, rb);
  rau = get_frame_register_unsigned (frame, ra);
  rbu = get_frame_register_unsigned (frame, rb);

  switch (GET_IW_OP(inst))
    {
        case OP_I12A:
            switch(GET_IW_I12(inst)) {
                case I12A_BEQ:
                  if (ras == rbs)
                  {
                    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

                case I12A_BGE:
                  if (ras >= rbs)
                  {
                    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

                case I12A_BGEU:
                  if (rau >= rbu)
                  {
                    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

                case I12A_BLT:
                  if (ras < rbs)
                  {
                    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

                case I12A_BLTU:
                  if (rau < rbu)
                  {
                    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

                case I12A_BNE:
                  if (ras != rbs)
		  {
		    imm12 = SIGN_EXTEND12(imm12);
                    pc += (imm12 * 4);
                  }
                  break;

/*                case I12A_BR:
                  pc += imm12;
                  break;*/
            }
            break;
        case OP_IX:
            switch(GET_IW_IX(inst)) {
                case IX_JMPI:
                case IX_CALL:
                  //pc = ((pc & 0xf0000000) | (GET_IW_IMM26(inst))) << 2;
                	pc = (((inst & OP_MASK_IMM26LO) >> OP_SH_IMM26LO) | ((inst & OP_MASK_IMM26HI) >> (OP_SH_IMM26HI - OP_SH_IMM26LO_SIZE))) << 2;
                  break;
            }
            break;
        case OP_R3:
            switch(GET_IW_R3(inst)) {
                case R3_JMP:
                case R3_CALLR:
                case R3_RET:
                  pc = ras;
                  break;
                default:
                  break;
            }
            break;
        default:
            break;
    }
    return pc;
}


/* nios2_software_single_step() is called just before we want to resume the
   inferior, if we want to single-step it but there is no hardware or kernel
   single-step support.  We find the target of the coming instruction
   and breakpoint it.  */
int
nios2_software_single_step (struct frame_info *frame)
{
  struct gdbarch *gdbarch = get_frame_arch (frame);

  CORE_ADDR next_pc = nios2_get_next_pc (frame, get_frame_pc (frame));
  insert_single_step_breakpoint (gdbarch, next_pc);

  return 1;
}

/* Core file and register set support.  */
/* Core file is removed, we don't support this */
#ifdef ENABLE_CORE_FILE
static const int reg_offsets[NIOS2_NUM_REGS] =
  {-1,  8,  9, 10, 11, 12, 13, 14,
    0,  1,  2,  3,  4,  5,  6,  7,
   23, 24, 25, 26, 27, 28, 29, 30,
   -1, -1, 19, 18, 17, 21, -1, 16,
   21, -1, 20, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1};

void
nios2_supply_gregset (const struct regset *regset,
                      struct regcache *regcache,
                      int regnum, const void *gregs_buf, size_t len)
{
  const gdb_byte *gregs = gregs_buf;
  int regno;
  static const gdb_byte zero_buf[4] = {0, 0, 0, 0};

  for (regno = Z_REGNUM; regno <= MPUACC_REGNUM; regno++)
    if (regnum == -1 || regnum == regno)
      {
	if (reg_offsets[regno] != -1)
	  regcache_raw_supply (regcache, regno,
			       gregs + 4 * reg_offsets[regno]);
	else
	  regcache_raw_supply (regcache, regno, zero_buf);
      }
}

static struct regset nios2_regset =
  {
    NULL,
    nios2_supply_gregset
  };
#endif /* ENABLE_CORE_FILE */

/* Return the appropriate register set for the core section identified
   by SECT_NAME and SECT_SIZE.  */
/*
Unknown, needs confirmation on difference between Nios II and dpx
*/
/*static const struct regset *
nios2_regset_from_core_section (struct gdbarch *gdbarch,
                                const char *sect_name, size_t sect_size)
{
  if (strcmp (sect_name, ".reg") == 0)
    return &nios2_regset;

  return NULL;
}
*/
/* Initialize a trad-frame cache corresponding to the tramp-frame.
   FUNC is the address of the instruction TRAMP[0] in memory.  */
static void
nios2_linux_sigreturn_init (const struct tramp_frame *self,
			    struct frame_info *next_frame,
			    struct trad_frame_cache *this_cache,
			    CORE_ADDR func)
{
  CORE_ADDR base = func + 16;
  int i;

  for (i = 0; i < 8; i++)
    trad_frame_set_reg_addr (this_cache, i+8, base + i * 4);
  for (i = 0; i < 7; i++)
    trad_frame_set_reg_addr (this_cache, i+1, base + (i+8)*4);
  trad_frame_set_reg_addr (this_cache, RA_REGNUM, base + 16*4);
  trad_frame_set_reg_addr (this_cache, FP_REGNUM, base + 17*4);
  trad_frame_set_reg_addr (this_cache, SP_REGNUM, base + 18*4);
  trad_frame_set_reg_addr (this_cache, GP_REGNUM, base + 19*4);
/*  trad_frame_set_reg_addr (this_cache, ESTATUS_REGNUM, base + 20*4);*/
  trad_frame_set_reg_addr (this_cache, PC_REGNUM, base + 21*4);

  /* Save a frame ID.  */
  trad_frame_set_id (this_cache, frame_id_build (base, func));
}

/* Initialize a trad-frame cache corresponding to the tramp-frame.
   FUNC is the address of the instruction TRAMP[0] in memory.  */
static void
nios2_linux_rt_sigreturn_init (const struct tramp_frame *self,
			       struct frame_info *next_frame,
			       struct trad_frame_cache *this_cache,
			       CORE_ADDR func)
{
  CORE_ADDR sp = frame_unwind_register_unsigned (next_frame, SP_REGNUM);
  CORE_ADDR base = func + 41*4;
  int i;

  for (i = 0; i < 23; i++)
    trad_frame_set_reg_addr (this_cache, i+1, base + i * 4);
  trad_frame_set_reg_addr (this_cache, RA_REGNUM, base + 23*4);
  trad_frame_set_reg_addr (this_cache, FP_REGNUM, base + 24*4);
  trad_frame_set_reg_addr (this_cache, GP_REGNUM, base + 25*4);
  trad_frame_set_reg_addr (this_cache, PC_REGNUM, base + 27*4);
  trad_frame_set_reg_addr (this_cache, SP_REGNUM, base + 28*4);

  /* Save a frame ID.  */
  trad_frame_set_id (this_cache, frame_id_build (sp, func));
}

static struct tramp_frame nios2_linux_sigreturn_tramp_frame = {
  SIGTRAMP_FRAME,
  4,
  {
	{ GET_I12_FORMAT_IW(0x02, 0x0, 0x02, 0x03, 119), -1 }, /* movi r2, __NR_sigreturn */
    //{ 0x00800004 | (__NR_sigreturn << 6), -1 },  /* movi r2,__NR_sigreturn */
    { 0x003b683a, -1 },               /* trap */
    { TRAMP_SENTINEL_INSN }
  },
  nios2_linux_sigreturn_init
};

static struct tramp_frame nios2_linux_rt_sigreturn_tramp_frame = {
  SIGTRAMP_FRAME,
  4,
  {
	{ GET_I12_FORMAT_IW(0x02, 0x0, 0x02, 0x03, 173), -1 }, /* movi r2, __NR_rt_sigreturn */
    //{ NIOS2_INSN_MACRO_MOVI | (__NR_rt_sigreturn << 6), -1 },  /* movi r2,__NR_rt_sigreturn */
    { 0x003b683a, -1 },               /* trap */
    { TRAMP_SENTINEL_INSN }
  },
  nios2_linux_rt_sigreturn_init
};


static struct gdbarch *
nios2_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch_tdep *tdep = NULL;
  struct gdbarch *gdbarch;
  int register_bytes, i;
  int regLoop;

  remote_hw_breakpoint_limit = 4;
  /* Find a candidate among the list of pre-declared architectures. */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != NULL)
    {   
      tdep = gdbarch_tdep (arches->gdbarch);     
      if (tdep->extension_register_cfg == -1 && info.abfd) 
        {
          unsigned int elf_flags = elf_elfheader (info.abfd)->e_flags;          
          tdep->extension_register_cfg = E_NIOS2_REG_MAP_GET(elf_flags);
        }
      
      return arches->gdbarch;
    }
  
  /* No gdbarch found, create a new architecture from the information
     provided. */

  tdep = xmalloc (sizeof (struct gdbarch_tdep));
  if (info.abfd) 
    {
      unsigned int elf_flags = elf_elfheader (info.abfd)->e_flags;          
      tdep->extension_register_cfg = E_NIOS2_REG_MAP_GET(elf_flags);
    }
  else
    tdep->extension_register_cfg = -1;

  gdbarch = gdbarch_alloc (&info, tdep);


  /* Data type sizes.  */
  set_gdbarch_ptr_bit (gdbarch, 32);
  set_gdbarch_addr_bit (gdbarch, 32);
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);


  set_gdbarch_float_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_double_format (gdbarch, floatformats_ieee_double);

  /* The register set.  */
  set_gdbarch_num_regs (gdbarch, NIOS2_NUM_REGS);
  set_gdbarch_sp_regnum (gdbarch, SP_REGNUM);
  set_gdbarch_pc_regnum (gdbarch, PC_REGNUM);	/* Pseudo register PC */

  set_gdbarch_register_name (gdbarch, nios2_register_name);
  set_gdbarch_register_type (gdbarch, nios2_register_type); /* TODO: we're currently hacking here, all types are uint32, but it should work for now */

   /* The "default" register numbering scheme for AMD64 is referred to
   as the "DWARF Register Number Mapping" in the System V psABI.
   The preferred debugging format for all known Nios II targets is
   actually DWARF2, and GCC doesn't seem to support DWARF (that is
   DWARF-1), but we provide the same mapping just in case.  This
   mapping is also used for stabs, which GCC does support.  */
  set_gdbarch_stab_reg_to_regnum (gdbarch, nios2_dwarf_reg_to_regnum);

  /* is dwarf no longer supported? */
  /*set_gdbarch_dwarf_reg_to_regnum (gdbarch, nios2_dwarf_reg_to_regnum);*/
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, nios2_dwarf_reg_to_regnum);

  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);

  /* Call dummy code.  */
  set_gdbarch_frame_align (gdbarch, nios2_frame_align);

  set_gdbarch_convert_register_p (gdbarch, nios2_convert_register_p);
  set_gdbarch_register_to_value (gdbarch, nios2_register_to_value);
  set_gdbarch_value_to_register (gdbarch, nios2_value_to_register);

  set_gdbarch_return_value (gdbarch, nios2_return_value);

  set_gdbarch_skip_prologue (gdbarch, nios2_skip_prologue);
  set_gdbarch_breakpoint_from_pc (gdbarch, nios2_breakpoint_from_pc);

  /* unknown, might not need changes in dpx */
  set_gdbarch_unwind_pc (gdbarch, nios2_unwind_pc);
  set_gdbarch_unwind_sp (gdbarch, nios2_unwind_sp);
  set_gdbarch_dummy_id (gdbarch, nios2_unwind_dummy_id);

  /* The dwarf2 unwinder will normally produce the best results if the
  debug information is available, so register it first. */
  /* dwarf2_frame_sniffer is replaced with dwarf2_append_unwinders() */
  /*frame_unwind_append_unwinder (gdbarch, &dwarf2_frame_sniffer);*/
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &nios2_stub_frame_unwind);

  /* Shared library handling.  */
  set_gdbarch_skip_trampoline_code (gdbarch, find_solib_trampoline_target);
  set_gdbarch_skip_solib_resolver (gdbarch, glibc_skip_solib_resolver);

  set_solib_svr4_fetch_link_map_offsets (gdbarch,
					 svr4_ilp32_fetch_link_map_offsets);

  frame_unwind_append_unwinder(gdbarch, &nios2_frame_unwind);
  /* frame_unwind_append_sniffer (gdbarch, nios2_frame_sniffer); */

  frame_base_set_default (gdbarch, &nios2_frame_base);

  /* Enable TLS support.  */
  set_gdbarch_fetch_tls_load_module_address (gdbarch,
                                             svr4_fetch_objfile_link_map);

  /* Disassembler printing */
  set_gdbarch_print_insn (gdbarch, gdb_print_insn_nios2);

  /* Enable inferior call support.  */
  set_gdbarch_push_dummy_call (gdbarch, nios2_push_dummy_call);

  /* Core file support.
     no we don't support core file
     */

  /* set_gdbarch_regset_from_core_section (gdbarch,
                                        nios2_regset_from_core_section);*/

#ifdef NIOS2_LINUX
  tramp_frame_prepend_unwinder (gdbarch,
                                &nios2_linux_sigreturn_tramp_frame);
  tramp_frame_prepend_unwinder (gdbarch,
                                &nios2_linux_rt_sigreturn_tramp_frame);
#endif

  return gdbarch;
}

static void
nios2_dump_tdep (struct gdbarch *gdbarch, struct ui_file *file)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (tdep == NULL)
    return;

  fprintf_unfiltered (file, _("nios2_dump_tdep: Extension Register Config = %d"),
 		      tdep->extension_register_cfg);
}


void
_initialize_nios2_tdep (void)
{
  int i, offset = 0;

  gdbarch_register (bfd_arch_nios2, nios2_gdbarch_init, nios2_dump_tdep);

  /* Do not display anything after NIOS2_MAX_REG_DISPLAYED_REGNUM */
  for (i = 0; i <= NIOS2_MAX_REG_DISPLAYED_REGNUM; i++)
    {
       nios2_register_byte_table[i] = offset;
       offset += nios2_register_info_table[i].size;
    }
  
  for (i = NIOS2_MAX_REG_DISPLAYED_REGNUM + 1; i < NIOS2_NUM_REGS; i++)
    {
      nios2_register_byte_table[i] = -1;
    }
  
}
