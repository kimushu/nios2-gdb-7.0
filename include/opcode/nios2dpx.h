/* NOT ASSIGNED TO FSF.  COPYRIGHT ALTERA.  */
/* nios2dpx.h.  Ported from nios2.h

   Copyright (C) 2010 Altera

This file is part of GDB, GAS, and the GNU binutils.

GDB, GAS, and the GNU binutils are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
1, or (at your option) any later version.

GDB, GAS, and the GNU binutils are distributed in the hope that they
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
*/

#ifndef _NIOS2_DPX_H_
#define _NIOS2_DPX_H_


/****************************************************************************
 * This file contains structures, bit masks and shift counts used
 * by the GNU toolchain to define the New Jersey instruction set and
 * access various opcode fields.
 ****************************************************************************/

/* this enum is used in overflow_msgs in opcode/nios2dpx-opc.c */
enum overflow_type
{
  call_target_overflow = 0,
  branch_target_overflow,
  address_offset_overflow,
  signed_immed16_overflow,
  signed_immed12_overflow,
  unsigned_immed20_overflow,
  unsigned_immed16_overflow,
  unsigned_immed12_overflow,
  unsigned_immed5_overflow,
  custom_opcode_overflow,
  no_overflow
}; 

/*---------------------------------------------------------------------------
   This structure holds information for a particular instruction
  ---------------------------------------------------------------------------*/

/* match When assembling, this
     opcode is modified by the arguments to produce the actual opcode
     that is used.  If pinfo is INSN_MACRO, then this is 0.  */

/* mask If pinfo is not INSN_MACRO, then this is a bit mask for the
     relevant portions of the opcode when disassembling.  If the
     actual opcode anded with the match field equals the opcode field,
     then we have found the correct instruction.  If pinfo is
     INSN_MACRO, then this field is the macro identifier.  */

/* For a macro, this is INSN_MACRO.  Otherwise, it is a collection
     of bits describing the instruction, notably any relevant hazard
     information.  */

struct nios2_opcode
{
  const char *name;		/* The name of the instruction.  */
  const char *args;		/* A string describing the arguments for this 
                                   instruction.  */
  const char *args_test;	/* Like args, but with an extra argument for 
                                   the expected opcode.  */
  unsigned long num_args;	/* The number of arguments the instruction 
                                   takes.  */
  unsigned long match;		/* The basic opcode for the instruction.  */
  unsigned long mask;		/* Mask for the opcode field of the 
                                   instruction.  */
  unsigned long pinfo;		/* Is this a real instruction or instruction 
			           macro?  */
  enum overflow_type overflow_msg;  /* Msg template used to generate informative 
				       message when fixup overflows. */

  const char dest_reg;          /* The destination register char (if this
                                   instruction writes to a register). 0 if there is
                                   no destination */

  const char *restrict_reg;     /* A null terminated char array of reg chars that have extension
                                   register restrictions. i.e. r32-r63 cannot exist in
                                   registers that use this char */
                       
};

/* This value is used in the nios2_opcode.pinfo field to indicate that the 
   instruction is a macro or pseudo-op.  This requires special treatment by 
   the assembler, and is used by the disassembler to determine whether to 
   check for a nop.  */
#define NIOS2_INSN_MACRO 	0x80000000
#define NIOS2_INSN_MACRO_MOV	0x80000001
#define NIOS2_INSN_MACRO_MOVI	0x80000002
#define NIOS2_INSN_MACRO_MOVIA  0x80000004

#define NIOS2_INSN_RELAXABLE	0x40000000
#define NIOS2_INSN_UBRANCH	0x00000010
#define NIOS2_INSN_CBRANCH	0x00000020
#define NIOS2_INSN_CALL		0x00000040

#define NIOS2_INSN_ADDI		0x00000080
#define NIOS2_INSN_ANDI		0x00000100
#define NIOS2_INSN_ORI		0x00000200
#define NIOS2_INSN_XORI		0x00000400
#define NIOS2_INSN_LOAD_STORE	0x00000800
#define NIOS2_INSN_MOVIA        0x00001000

/* Associates a register name ($6) with a 6-bit index (eg 6).  */
struct nios2_reg
{
  const char *name;
  int index;
};
/* JDS: remove const on index so that it becomes configurable */


/* Maps a set of extenstion register aliases to physical register numbers */
struct nios2_ext_reg_bank_map
{
  const int offset;
  const int num_regs;
};

struct nios2_ext_reg_map
{
  const struct nios2_ext_reg_bank_map rx_tx;
  const struct nios2_ext_reg_bank_map cr;
  const struct nios2_ext_reg_bank_map cri_cro;
};


extern int nios2_get_register_num(const char *reg_name, const char *reg_prefix);
extern int nios2_get_extension_register_index(const char *reg_name, int config_num);
extern int nios2_get_register_alias(char *alias, int alias_size, const char *reg_name, int config_num, int is_dest);



/* -------------------------------------------------------------------------
    Some useful macros for creating register names
   -------------------------------------------------------------------------*/

#define STRINGIFY(s) #s
#define REG_STR(reg_prefix,n) STRINGIFY(reg_prefix)#n
#define R_REG(n)   REG_STR(r,n)
#define C_REG(n)   REG_STR(c,n)
#define CTL_REG(n) REG_STR(ctl,n)
#define RX_REG(n)  REG_STR(rx,n)
#define TX_REG(n)  REG_STR(tx,n)
#define CR_REG(n)  REG_STR(cr,n)
#define CRI_REG(n) REG_STR(cri,n)
#define CRO_REG(n) REG_STR(cro,n)


/* -------------------------------------------------------------------------
    Bitfield masks for New Jersey instructions
   -------------------------------------------------------------------------*/

/* These are bit masks and shift counts to use to access the various
   fields of an instruction.  */

/* Macros for getting and setting an instruction field.  */
#define GET_INSN_FIELD(X, i)     (((i) & OP_MASK_##X) >> OP_SH_##X)
#define SET_INSN_FIELD(X, i, j)  ((i) = ((i) &~ (OP_MASK_##X)) | \
                                  (((j) << OP_SH_##X) & (OP_MASK_##X)))


/* We include the auto-generated file nios2-isa.h and define the mask
   and shifts below in terms of those in nios2-isa.h.  This ensures
   that the binutils and hardware are always in sync.  */

#include "nios2dpx-isa.h"

#define OP_MASK_OP              (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_OP                IW_OP_LSB


/* Masks and shifts for I-type instructions.  */

/* The 6b encoding has 4 I variants : I16 IX I12 I5*/
/* The 5b I is just like I16 */

#define OP_MASK_IOP             (IW_OP_MASK << IW_OP_LSB)
#define OP_SH_IOP               IW_OP_LSB

#define OP_MASK_IMM16           (IW_IMM16_MASK << IW_IMM16_LSB)
#define OP_SH_IMM16             IW_IMM16_LSB

#define OP_MASK_IRD             (IW_B_MASK << IW_B_LSB)	
				/* The same as T for I-type.  */
#define OP_SH_IRD               IW_B_LSB

#define OP_MASK_IRT             (IW_B_MASK << IW_B_LSB)
#define OP_SH_IRT               IW_B_LSB

#define OP_MASK_IRS             (IW_A_MASK << IW_A_LSB)
#define OP_SH_IRS               IW_A_LSB

/* specific to I12 instruction format */
#define OP_MASK_II12            (IW_I12_MASK << IW_I12_LSB)
#define OP_SH_II12              IW_I12_LSB

#define OP_MASK_IMM12           (IW_IMM12_MASK << IW_IMM12_LSB)
#define OP_SH_IMM12             IW_IMM12_LSB

/* TODO this value should come from nios2dpx-isa.h */
#define OP_MASK_BMX_LSB_IMM5    (IW_IMM5_MASK << IW_IMM5_LSB)
#define OP_SH_BMX_LSB_IMM5      IW_IMM5_LSB
#define OP_BMX_LSB_IMM5_SIZE	5

/* TODO this value should come from nios2dpx-isa.h */
#define OP_MASK_BMX_MSB_IMM5    (IW_IMM5_MASK << 5)
#define OP_SH_BMX_MSB_IMM5      5
#define OP_BMX_MSB_IMM5_SIZE	5

/* specific to DCACHE instruction format */
#define OP_MASK_IDCACHE         (IW_DCACHE_MASK << IW_DCACHE_LSB)
#define OP_SH_IDCACHE            IW_DCACHE_LSB

/* specific to I5 instruction format */
#define OP_MASK_II5             (IW_I5_MASK << IW_I5_LSB)
#define OP_SH_II5               IW_I5_LSB

#define OP_MASK_IMM5            (IW_IMM5_MASK << IW_IMM5_LSB)
#define OP_SH_IMM5              IW_IMM5_LSB

/* specific to IX instruction format */
#define OP_MASK_IIX             (IW_IX_MASK << IW_IX_LSB)
#define OP_SH_IIX               IW_IX_LSB

#define OP_MASK_IMM26HI         (IW_IMM26_MASK << IW_IMM26_LSB)
#define OP_SH_IMM26HI           IW_IMM26_LSB

#define OP_MASK_IMM20           (IW_IMM20_MASK << IW_IMM20_LSB)
#define OP_SH_IMM20             IW_IMM20_LSB

#define OP_MASK_IMM26LO         OP_MASK_IMM20
#define OP_SH_IMM26LO           OP_SH_IMM20
#define OP_SH_IMM26LO_SIZE      IW_IMM20_SZ

/* Masks and shifts for R-type instructions.  */
#define OP_MASK_RR3   		(IW_R3_MASK << IW_R3_LSB)
#define OP_SH_RR3               IW_R3_LSB

#define OP_MASK_RRD             (IW_C_MASK << IW_C_LSB)
#define OP_SH_RRD               IW_C_LSB

#define OP_MASK_RRT             (IW_B_MASK << IW_B_LSB)
#define OP_SH_RRT               IW_B_LSB

#define OP_MASK_RRS             (IW_A_MASK << IW_A_LSB)
#define OP_SH_RRS               IW_A_LSB

/* This mask/shift decides R3 or custom instruction */
#define OP_MASK_R3_CUSTOM	(IW_IS_CUSTOM_MASK << IW_IS_CUSTOM_LSB)
#define OP_SH_R3_CUSTOM		IW_IS_CUSTOM_LSB

/* 6B encoding does not have a J-type instructions. IX replaces J */

/* Masks and shifts for CTL instructions.  */
/* The RDCTL instruction stores the 6-bit source control register 
   number in the B-field, the A field is 0x0 and the C-field
   contains the destination register number. 
   T (gcc) field == B field */
#define OP_MASK_RCTL            OP_MASK_RRT
#define OP_SH_RCTL              OP_SH_RRT

/* Break instruction imm5 field.  */
/* ToDo: Try to get rid of these */
#define OP_MASK_TRAP_IMM5       OP_MASK_IMM5
#define OP_SH_TRAP_IMM5         OP_SH_IMM5

/* Custom instruction masks.  */
/* these are named in a confusing way. They refer to the
   1 bit value and not the register value */
#define OP_MASK_CUSTOM_A        (IW_CUSTOM_READRA_MASK << IW_CUSTOM_READRA_LSB)
#define OP_SH_CUSTOM_A		IW_CUSTOM_READRA_LSB

#define OP_MASK_CUSTOM_B	(IW_CUSTOM_READRB_MASK << IW_CUSTOM_READRB_LSB)
#define OP_SH_CUSTOM_B		IW_CUSTOM_READRB_LSB

#define OP_MASK_CUSTOM_C	(IW_CUSTOM_WRITERC_MASK << IW_CUSTOM_WRITERC_LSB)
#define OP_SH_CUSTOM_C		IW_CUSTOM_WRITERC_LSB

#define OP_MASK_CUSTOM_N	(IW_CUSTOM_N_MASK << IW_CUSTOM_N_LSB)
#define OP_SH_CUSTOM_N		IW_CUSTOM_N_LSB
#define OP_MAX_CUSTOM_N		((1<<IW_CUSTOM_N_SZ) - 1)

/* The following macros define the opcode matches for each
   instruction code & OP_MASK_INST == OP_MATCH_INST.  */

/* OP instruction matches.  */
#define OP_MATCH_IX             (OP_IX << OP_SH_OP)
#define OP_MATCH_I12A           (OP_I12A << OP_SH_OP)
#define OP_MATCH_I12B           (OP_I12B << OP_SH_OP)

#define OP_MATCH_R3             ((OP_R3 << OP_SH_OP) | (0 << OP_SH_R3_CUSTOM))
#define OP_MASK_R3		(OP_MASK_IOP | OP_MASK_R3_CUSTOM)
#define OP_SH_R3		OP_SH_R3_CUSTOM

#define OP_MATCH_SNDI           (OP_SNDI << OP_SH_OP)
#define OP_MASK_SNDI            OP_MASK_IOP

#define OP_MATCH_RCVI           (OP_RCVI << OP_SH_OP)
#define OP_MASK_RCVI            OP_MASK_IOP

#define OP_MATCH_ANDI           (OP_ANDI << OP_SH_OP)
#define OP_MASK_ANDI            OP_MASK_IOP

#define OP_MATCH_ANDCI		(OP_ANDCI << OP_SH_OP)
#define OP_MASK_ANDCI		OP_MASK_IOP

#define OP_MATCH_ANDCHI		(OP_ANDCHI << OP_SH_OP)
#define OP_MASK_ANDCHI		OP_MASK_IOP

#define OP_MATCH_ORI            (OP_ORI << OP_SH_OP)
#define OP_MASK_ORI             OP_MASK_IOP

#define OP_MATCH_XORI           (OP_XORI << OP_SH_OP)
#define OP_MASK_XORI            OP_MASK_IOP

#define OP_MATCH_ANDHI          (OP_ANDHI << OP_SH_OP)
#define OP_MASK_ANDHI           OP_MASK_IOP

#define OP_MATCH_ORHI           (OP_ORHI << OP_SH_OP)
#define OP_MASK_ORHI            OP_MASK_IOP

#define OP_MATCH_XORHI          (OP_XORHI << OP_SH_OP)
#define OP_MASK_XORHI           OP_MASK_IOP

#define OP_MATCH_CUSTOM         ((OP_CUSTOM << OP_SH_OP) | 1 << OP_SH_R3_CUSTOM)
#define OP_MASK_CUSTOM          (OP_MASK_IOP | OP_MASK_R3_CUSTOM)
#define OP_SH_CUSTOM		OP_SH_R3_CUSTOM

/* IX instruction values.  */
#define OP_MATCH_CALL           ((IX_CALL << OP_SH_IIX) | (OP_MATCH_IX))
#define OP_MASK_CALL            (OP_MASK_IOP | OP_MASK_IIX)

#define OP_MATCH_JMPI           ((IX_JMPI << OP_SH_IIX) | (OP_MATCH_IX))
#define OP_MASK_JMPI            (OP_MASK_IOP | OP_MASK_IIX)

#define OP_MATCH_MOVHI20        ((IX_MOVHI20 << OP_SH_IIX) | (OP_MATCH_IX))
#define OP_MASK_MOVHI20         (OP_MASK_IOP | OP_MASK_IIX)

/* I12A instruction values.  */
#define OP_MATCH_I5             ((I12A_I5 << OP_SH_II12) | (OP_MATCH_I12A))

#define OP_MATCH_CMPGEI         ((I12A_CMPGEI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPGEI          (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_CMPLTI         ((I12A_CMPLTI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPLTI          (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_CMPNEI         ((I12A_CMPNEI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPNEI          (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_CMPEQI         ((I12A_CMPEQI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPEQI          (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_CMPGEUI        ((I12A_CMPGEUI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPGEUI         (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_CMPLTUI        ((I12A_CMPLTUI << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_CMPLTUI         (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_RDPRS          ((I12A_RDPRS << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_RDPRS           (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BGE            ((I12A_BGE << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BGE             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BLT            ((I12A_BLT << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BLT             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BNE            ((I12A_BNE << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BNE             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BEQ            ((I12A_BEQ << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BEQ             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BGEU           ((I12A_BGEU << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BGEU            (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_BLTU           ((I12A_BLTU << OP_SH_II12) | (OP_MATCH_I12A))
#define OP_MASK_BLTU            (OP_MASK_IOP | OP_MASK_II12)

/* I5 instruction values.  */
#define OP_MATCH_ROLI           ((I5_ROLI << OP_SH_II5) | (OP_MATCH_I5))
#define OP_MASK_ROLI            (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_II5)

#define OP_MATCH_SLLI           ((I5_SLLI << OP_SH_II5) | (OP_MATCH_I5))
#define OP_MASK_SLLI            (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_II5)

#define OP_MATCH_SRLI           ((I5_SRLI << OP_SH_II5) | (OP_MATCH_I5))
#define OP_MASK_SRLI            (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_II5)

#define OP_MATCH_TRAP           ((I5_TRAP << OP_SH_II5) | (29 << OP_SH_RRT) | (OP_MATCH_I5))
#define OP_MASK_TRAP            (OP_MASK_IOP | OP_MASK_RRT | OP_MASK_II12 | OP_MASK_II5)

#define OP_MATCH_BREAK          ((I5_BREAK << OP_SH_II5) | (30 << OP_SH_RRT) | (OP_MATCH_I5))
#define OP_MASK_BREAK           (OP_MASK_IOP | OP_MASK_RRT | OP_MASK_II12 | OP_MASK_II5)

#define OP_MATCH_SRAI           ((I5_SRAI << OP_SH_II5) | (OP_MATCH_I5))
#define OP_MASK_SRAI            (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_II5)

/* I12B instruction values.  */
#define OP_MATCH_LDB            ((I12B_LDB << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_LDB             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_LDH            ((I12B_LDH << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_LDH             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_LDW            ((I12B_LDW << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_LDW             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_ADDI           ((I12B_ADDI << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_ADDI            (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_LDBU           ((I12B_LDBU << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_LDBU            (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_LDHU           ((I12B_LDHU << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_LDHU            (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_STB            ((I12B_STB << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_STB             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_STH            ((I12B_STH << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_STH             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_STW            ((I12B_STW << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_STW             (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_MULI           ((I12B_MULI << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_MULI            (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_INSERT         ((I12B_INSERT << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_INSERT          (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_EXTRACT        ((I12B_EXTRACT << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_EXTRACT         (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_MERGE          ((I12B_MERGE << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MASK_MERGE           (OP_MASK_IOP | OP_MASK_II12)

#define OP_MATCH_DCACHE         ((I12B_DCACHE << OP_SH_II12) | (OP_MATCH_I12B))
#define OP_MATCH_DCACHE_INITD	((DCACHE_INITD << IW_DCACHE_LSB) | OP_MATCH_DCACHE)
#define OP_MATCH_DCACHE_INITDA	((DCACHE_INITDA << IW_DCACHE_LSB) | OP_MATCH_DCACHE)
#define OP_MATCH_DCACHE_FLUSHD	((DCACHE_FLUSHD << IW_DCACHE_LSB) | OP_MATCH_DCACHE)
#define OP_MATCH_DCACHE_FLUSHDA	((DCACHE_FLUSHDA << IW_DCACHE_LSB) | OP_MATCH_DCACHE)

/* I12B DCACHE instruction values.  */
#define OP_MATCH_INITD          ((DCACHE_INITD << OP_SH_IDCACHE) | (OP_MATCH_DCACHE_INITD))
#define OP_MASK_INITD           (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_IDCACHE)

#define OP_MATCH_INITDA         ((DCACHE_INITDA << OP_SH_IDCACHE) | (OP_MATCH_DCACHE_INITDA))
#define OP_MASK_INITDA          (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_IDCACHE)

#define OP_MATCH_FLUSHD         ((DCACHE_FLUSHD << OP_SH_IDCACHE) | (OP_MATCH_DCACHE_FLUSHD))
#define OP_MASK_FLUSHD          (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_IDCACHE)

#define OP_MATCH_FLUSHDA        ((DCACHE_FLUSHDA << OP_SH_IDCACHE) | (OP_MATCH_DCACHE_FLUSHDA))
#define OP_MASK_FLUSHDA         (OP_MASK_IOP | OP_MASK_II12 | OP_MASK_IDCACHE)

/* R3 instruction values.  */
// CHA: 30 = sstatus
#define OP_MATCH_ERET           ((R3_ERET << OP_SH_RR3) | (0 << OP_SH_RRD) | (30 << OP_SH_RRT) | (ERETADDR_REGNUM << OP_SH_RRS) | (OP_MATCH_R3))
#define OP_MASK_ERET            (OP_MASK_R3 | OP_MASK_RRS | OP_MASK_RRT | OP_MASK_RRD | OP_MASK_RR3)

#define OP_MATCH_FORK           ((R3_FORK << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_FORK            (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_ROL            ((R3_ROL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_ROL             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_FLUSHP         ((R3_FLUSHP << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_FLUSHP          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_RET            ((R3_RET << OP_SH_RR3) | (0 << OP_SH_RRD) | (0 << OP_SH_RRT) | (RETADDR_REGNUM << OP_SH_RRS) | (OP_MATCH_R3))
#define OP_MASK_RET             (OP_MASK_R3 | OP_MASK_RRS | OP_MASK_RRT | OP_MASK_RRD | OP_MASK_RR3)

#define OP_MATCH_NOR            ((R3_NOR << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_NOR             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_MULXUU         ((R3_MULXUU << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_MULXUU          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPGE          ((R3_CMPGE << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPGE           (OP_MASK_R3 | OP_MASK_RR3)

// TODO get value from isa.h
#define OP_MATCH_BRET           ((R3_BRET << OP_SH_RR3) | (0 << OP_SH_RRD) | (0 << OP_SH_RRT) | (BRETADDR_REGNUM << OP_SH_RRS) | (OP_MATCH_R3))
#define OP_MASK_BRET            (OP_MASK_R3 | OP_MASK_RRD | OP_MASK_RRS | OP_MASK_RRD | OP_MASK_RR3)

#define OP_MATCH_EXIT           ((R3_EXIT << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_EXIT            (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_ROR            ((R3_ROR << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_ROR             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_FLUSHI         ((R3_FLUSHI << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_FLUSHI          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_JMP            ((R3_JMP << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_JMP             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_AND            ((R3_AND << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_AND             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPLT          ((R3_CMPLT << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPLT           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CIDALLOC       ((R3_CIDALLOC << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CIDALLOC        (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SLL            ((R3_SLL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SLL             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_WRPRS          ((R3_WRPRS << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_WRPRS           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_OR             ((R3_OR << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_OR              (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_MULXSU         ((R3_MULXSU << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_MULXSU          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPNE          ((R3_CMPNE << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPNE           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SRL            ((R3_SRL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SRL             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_NEXTPC         ((R3_NEXTPC << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_NEXTPC          (OP_MASK_R3 | OP_MASK_RR3)

// TODO get values from isa.h
#define OP_MATCH_CALLR          ((R3_CALLR << OP_SH_RR3) | (31 << OP_SH_RRD)| (0 << OP_SH_RRT)  | (OP_MATCH_R3))
#define OP_MASK_CALLR           (OP_MASK_R3 | OP_MASK_RRT | OP_MASK_RRD | OP_MASK_RR3)

#define OP_MATCH_XOR            ((R3_XOR << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_XOR             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_MULXSS         ((R3_MULXSS << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_MULXSS          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPEQ          ((R3_CMPEQ << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPEQ           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SND            ((R3_SND << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SND             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_DIVU           ((R3_DIVU << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_DIVU            (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_DIV            ((R3_DIV << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_DIV             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_RDCTL          ((R3_RDCTL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_RDCTL           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_MUL            ((R3_MUL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_MUL             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPGEU         ((R3_CMPGEU << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPGEU          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_INITI          ((R3_INITI << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_INITI           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_RCV            ((R3_RCV << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_RCV             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_WRCTL          ((R3_WRCTL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_WRCTL           (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_CMPLTU         ((R3_CMPLTU << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_CMPLTU          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_ADD            ((R3_ADD << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_ADD             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_TXALLOC        ((R3_TXALLOC << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_TXALLOC         (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_JREL           ((R3_JREL << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_JREL            (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SYNC           ((R3_SYNC << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SYNC            (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SUB            ((R3_SUB << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SUB             (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_RXFREE         ((R3_RXFREE << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_RXFREE          (OP_MASK_R3 | OP_MASK_RR3)

#define OP_MATCH_SRA            ((R3_SRA << OP_SH_RR3) | (OP_MATCH_R3))
#define OP_MASK_SRA             (OP_MASK_R3 | OP_MASK_RR3)

/* ToDo: Remove this and replace it with a pseudo instruction */
#define OP_MATCH_BR OP_MATCH_BEQ
#define OP_MASK_BR OP_MASK_BEQ


#ifndef OP_MASK
#define OP_MASK                         0xffffffff
#endif

/* These are the data structures we use to hold the instruction information.  */

extern const struct nios2_opcode nios2_builtin_opcodes[];
extern const int bfd_nios2_num_builtin_opcodes;
extern struct nios2_opcode *nios2_opcodes;
extern int bfd_nios2_num_opcodes;

/* These are the data structures used to hold the operand parsing information.  */
#if 0
extern const struct nios2_arg_parser nios2_arg_parsers[]; 
extern struct nios2_arg_parser* nios2_arg_parsers;
extern const int nios2_num_builtin_arg_parsers;
extern int nios2_num_arg_parsers;
#endif 

/* These are the data structures used to hold the register information.  */
extern const struct nios2_reg nios2_builtin_regs[];
extern struct nios2_reg *nios2_regs;
extern const int nios2_num_builtin_regs;
extern int nios2_num_regs;


/* These are the data structures used to hold the extenstion register alias information. */
extern const struct nios2_ext_reg_map nios2_builtin_ext_reg_maps[];
extern struct nios2_ext_reg_map *nios2_ext_reg_maps;
extern const int nios2_num_builtin_ext_reg_maps;
extern int nios2_num_ext_reg_maps;


/* Machine-independent macro for number of opcodes.  */

#define NUMOPCODES bfd_nios2_num_opcodes
#define NUMREGISTERS nios2_num_regs;

/* These are used in disassembly to get the correct register names.  */
#define NUMREGNAMES 64
#define NUMCTLREGNAMES 64
#define CTLREGBASE     64

/* This is made extern so that the assembler can use it to find out
   what instruction caused an error.  */
extern const struct nios2_opcode *nios2_find_opcode_hash (unsigned long);

/* Overflow message strings used in the assembler.  */
extern char *overflow_msgs[];


#endif /* _NIOS2_DPX_H */
