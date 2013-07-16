/* NOT ASSIGNED TO FSF.  COPYRIGHT ALTERA.  */
/* nios2dpx-opc.c -- Ported from nios2-opc.c

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
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "opcode/nios2dpx.h"

/* Register string table */

const struct nios2_reg nios2_builtin_regs[] = {
  {"zero", 0},
  {"at", 1},		        /* assembler temporary */
  {"r2", 2},
  {"r3", 3},
  {"r4", 4},
  {"r5", 5},
  {"r6", 6},
  {"r7", 7},
  {"r8", 8},
  {"r9", 9},
  {"r10", 10},
  {"r11", 11},
  {"r12", 12},
  {"r13", 13},
  {"r14", 14},
  {"r15", 15},
  {"r16", 16},
  {"r17", 17},
  {"r18", 18},
  {"r19", 19},
  {"r20", 20},
  {"r21", 21},
  {"r22", 22},
  {"r23", 23},
  {"r24", 24},
  {"bt", 25},
  {"gp", 26},			/* global pointer */
  {"sp", 27},			/* stack pointer */
  {"fp", 28},			/* frame pointer */
  {"ea", 29},			/* exception return address */
  {"ba", 30},  			/* breakpoint return address */
  {"ra", 31},			/* return address */
  {"r32", 32},
  {"r33", 33},
  {"r34", 34},
  {"r35", 35},
  {"r36", 36},
  {"r37", 37},
  {"r38", 38},
  {"r39", 39},
  {"r40", 40},
  {"r41", 41},
  {"r42", 42},
  {"r43", 43},
  {"r44", 44},
  {"r45", 45},
  {"r46", 46},
  {"r47", 47},
  {"r48", 48},
  {"r49", 49},
  {"r50", 50},
  {"r51", 51},
  {"r52", 52},
  {"r53", 53},
  {"r54", 54},
  {"r55", 55},
  {"r56", 56},
  {"r57", 57},
  {"r58", 58},
  {"r59", 59},
  {"r60", 60},
  {"r61", 61},
  {"r62", 62},
  {"r63", 63},

  /* note that if you add an alias to general purpose reg, you must
     increment CTLREGBASE in include/opcode/nios2dpx.h */

  /* control register names */
  {"ctl0", 0}, /* CTLREGBASE == 64 */
  {"ctl1", 1},
  {"ctl2", 2},
  {"ctl3", 3},
  {"ctl4", 4},
  {"cpuid", 5},
  {"ctl6", 6},
  {"ctl7", 7},
  {"ctl8", 8},
  {"ctl9", 9},
  {"ctl10", 10},
  {"ctl11", 11},
  {"ctl12", 12},
  {"ctl13", 13},
  {"ctl14", 14},
  {"ctl15", 15},
  {"threadnum", 16},
  {"ctl17", 17},
  {"ctl18", 18},
  {"ctl19", 19},
  {"ctl20", 20},
  {"ctl21", 21},
  {"ctl22", 22},
  {"ctl23", 23},
  {"message_flags", 24},
  {"message_user", 25},
  {"message_id0", 26},
  {"message_id1", 27},
  {"ctl28", 28},
  {"ctl29", 29},
  {"ctl30", 30},
  {"ctl31", 31},
  {"ctl32", 32},
  {"ctl33", 33},
  {"ctl34", 34},
  {"ctl35", 35},
  {"ctl36", 36},
  {"ctl37", 37},
  {"ctl38", 38},
  {"ctl39", 39},
  {"ctl40", 40},
  {"ctl41", 41},
  {"ctl42", 42},
  {"ctl43", 43},
  {"ctl44", 44},
  {"ctl45", 45},
  {"ctl46", 46},
  {"ctl47", 47},
  {"ctl48", 48},
  {"ctl49", 49},
  {"ctl50", 50},
  {"ctl51", 51},
  {"ctl52", 52},
  {"ctl53", 53},
  {"ctl54", 54},
  {"ctl55", 55},
  {"ctl56", 56},
  {"ctl57", 57},
  {"ctl58", 58},
  {"ctl59", 59},
  {"ctl60", 60},
  {"ctl61", 61},
  {"ctl62", 62},
  {"ctl63", 63},

  /* alternative names for special registers */
  {"r0", 0},
  {"r1", 1},
  {"et", 24},
  {"r25", 25},
  {"r26", 26},
  {"r27", 27},
  {"r28", 28},
  {"r29", 29},
  {"r30", 30},
  {"sstatus", 30}, /* alias for ba and r30 */
  {"r31", 31},

  /* alternative names for special control registers */
  {"ctl5", 5},
  {"ctl16", 16},
  {"ctl24", 24},
  {"ctl25", 25},
  {"ctl26", 26},
  {"ctl27", 27},

  /* coprocessor register names */
  {"c0", 0},
  {"c1", 1},
  {"c2", 2},
  {"c3", 3},
  {"c4", 4},
  {"c5", 5},
  {"c6", 6},
  {"c7", 7},
  {"c8", 8},
  {"c9", 9},
  {"c10", 10},
  {"c11", 11},
  {"c12", 12},
  {"c13", 13},
  {"c14", 14},
  {"c15", 15},
  {"c16", 16},
  {"c17", 17},
  {"c18", 18},
  {"c19", 19},
  {"c20", 20},
  {"c21", 21},
  {"c22", 22},
  {"c23", 23},
  {"c24", 24},
  {"c25", 25},
  {"c26", 26},
  {"c27", 27},
  {"c28", 28},
  {"c29", 29},
  {"c30", 30},
  {"c31", 31},

  /* RX registers */
  { RX_REG(0), -1},
  { RX_REG(1), -1},
  { RX_REG(2), -1},
  { RX_REG(3), -1},
  { RX_REG(4), -1},
  { RX_REG(5), -1},
  { RX_REG(6), -1},
  { RX_REG(7), -1},
  { RX_REG(8), -1},
  { RX_REG(9), -1},
  { RX_REG(10), -1},
  { RX_REG(11), -1},
  { RX_REG(12), -1},
  { RX_REG(13), -1},
  { RX_REG(14), -1},
  { RX_REG(15), -1},
  { RX_REG(16), -1},
  { RX_REG(17), -1},
  { RX_REG(18), -1},
  { RX_REG(19), -1},
  { RX_REG(20), -1},
  { RX_REG(21), -1},
  { RX_REG(22), -1},
  { RX_REG(23), -1},
  { RX_REG(24), -1},
  { RX_REG(25), -1},
  { RX_REG(26), -1},
  { RX_REG(27), -1},
  { RX_REG(28), -1},
  { RX_REG(29), -1},
  { RX_REG(30), -1},
  { RX_REG(31), -1},

  /* TX registers */
  { TX_REG(0), -1},
  { TX_REG(1), -1},
  { TX_REG(2), -1},
  { TX_REG(3), -1},
  { TX_REG(4), -1},
  { TX_REG(5), -1},
  { TX_REG(6), -1},
  { TX_REG(7), -1},
  { TX_REG(8), -1},
  { TX_REG(9), -1},
  { TX_REG(10), -1},
  { TX_REG(11), -1},
  { TX_REG(12), -1},
  { TX_REG(13), -1},
  { TX_REG(14), -1},
  { TX_REG(15), -1},
  { TX_REG(16), -1},
  { TX_REG(17), -1},
  { TX_REG(18), -1},
  { TX_REG(19), -1},
  { TX_REG(20), -1},
  { TX_REG(21), -1},
  { TX_REG(22), -1},
  { TX_REG(23), -1},
  { TX_REG(24), -1},
  { TX_REG(25), -1},
  { TX_REG(26), -1},
  { TX_REG(27), -1},
  { TX_REG(28), -1},
  { TX_REG(29), -1},
  { TX_REG(30), -1},
  { TX_REG(31), -1},

  /* CR registers */
  { CR_REG(0), -1},
  { CR_REG(1), -1},
  { CR_REG(2), -1},
  { CR_REG(3), -1},
  { CR_REG(4), -1},
  { CR_REG(5), -1},
  { CR_REG(6), -1},
  { CR_REG(7), -1},
  { CR_REG(8), -1},
  { CR_REG(9), -1},
  { CR_REG(10), -1},
  { CR_REG(11), -1},
  { CR_REG(12), -1},
  { CR_REG(13), -1},
  { CR_REG(14), -1},
  { CR_REG(15), -1},
  { CR_REG(16), -1},
  { CR_REG(17), -1},
  { CR_REG(18), -1},
  { CR_REG(19), -1},
  { CR_REG(20), -1},
  { CR_REG(21), -1},
  { CR_REG(22), -1},
  { CR_REG(23), -1},
  { CR_REG(24), -1},
  { CR_REG(25), -1},
  { CR_REG(26), -1},
  { CR_REG(27), -1},
  { CR_REG(28), -1},
  { CR_REG(29), -1},
  { CR_REG(30), -1},
  { CR_REG(31), -1},

  /* CRI registers */
  { CRI_REG(0), -1},
  { CRI_REG(1), -1},
  { CRI_REG(2), -1},
  { CRI_REG(3), -1},
  { CRI_REG(4), -1},
  { CRI_REG(5), -1},
  { CRI_REG(6), -1},
  { CRI_REG(7), -1},
  { CRI_REG(8), -1},
  { CRI_REG(9), -1},
  { CRI_REG(10), -1},
  { CRI_REG(11), -1},
  { CRI_REG(12), -1},
  { CRI_REG(13), -1},
  { CRI_REG(14), -1},
  { CRI_REG(15), -1},
  { CRI_REG(16), -1},
  { CRI_REG(17), -1},
  { CRI_REG(18), -1},
  { CRI_REG(19), -1},
  { CRI_REG(20), -1},
  { CRI_REG(21), -1},
  { CRI_REG(22), -1},
  { CRI_REG(23), -1},
  { CRI_REG(24), -1},
  { CRI_REG(25), -1},
  { CRI_REG(26), -1},
  { CRI_REG(27), -1},
  { CRI_REG(28), -1},
  { CRI_REG(29), -1},
  { CRI_REG(30), -1},
  { CRI_REG(31), -1},

  /* CRO registers */
  { CRO_REG(0), -1},
  { CRO_REG(1), -1},
  { CRO_REG(2), -1},
  { CRO_REG(3), -1},
  { CRO_REG(4), -1},
  { CRO_REG(5), -1},
  { CRO_REG(6), -1},
  { CRO_REG(7), -1},
  { CRO_REG(8), -1},
  { CRO_REG(9), -1},
  { CRO_REG(10), -1},
  { CRO_REG(11), -1},
  { CRO_REG(12), -1},
  { CRO_REG(13), -1},
  { CRO_REG(14), -1},
  { CRO_REG(15), -1},
  { CRO_REG(16), -1},
  { CRO_REG(17), -1},
  { CRO_REG(18), -1},
  { CRO_REG(19), -1},
  { CRO_REG(20), -1},
  { CRO_REG(21), -1},
  { CRO_REG(22), -1},
  { CRO_REG(23), -1},
  { CRO_REG(24), -1},
  { CRO_REG(25), -1},
  { CRO_REG(26), -1},
  { CRO_REG(27), -1},
  { CRO_REG(28), -1},
  { CRO_REG(29), -1},
  { CRO_REG(30), -1},
  { CRO_REG(31), -1}

};

#define NIOS2_NUM_REGS \
       ((sizeof nios2_builtin_regs) / (sizeof (nios2_builtin_regs[0])))
const int nios2_num_builtin_regs = NIOS2_NUM_REGS;

/* const removed from the following to allow for dynamic extensions to the
 * built-in instruction set. */
struct nios2_reg *nios2_regs = (struct nios2_reg *) nios2_builtin_regs;
int nios2_num_regs = NIOS2_NUM_REGS;
#undef NIOS2_NUM_REGS


/* R32-R63 Extension Register Mapping Table */
const struct nios2_ext_reg_map nios2_builtin_ext_reg_maps[] = {
 /* RX/TX    CR       CRI/CRO    */
  { {32, 8}, {40,16}, {56, 8} },  /* Config 0 => DEFAULT */
  { {32, 8}, {40, 8}, {48,16} },  /* Config 1 */
  { {32,16}, {48, 8}, {56, 8} },  /* Config 2 */
  { {32,16}, {48,16}, {56, 0} },  /* Config 3 */
  { {32,16}, {48, 0}, {48,16} },  /* Config 4 */
  { {32,32}, {64, 0}, {64, 0} },  /* Config 5 */
  { {32, 8}, {40, 0}, {40, 0} },  /* Config 6 */
  { {32, 8}, {40, 0}, {40, 8} },  /* Config 7 */
  { {32, 8}, {40, 0}, {48,16} },  /* Config 8 */
  { {32, 8}, {40, 8}, {48, 0} },  /* Config 9 */
  { {32, 8}, {40, 8}, {48, 8} },  /* Config 10 */
  { {32, 8}, {48,16}, {64, 0} },  /* Config 11 */
  { {32,16}, {48, 0}, {48, 0} },  /* Config 12 */
  { {32,16}, {48, 0}, {48, 8} },  /* Config 13 */
  { {32,16}, {48, 8}, {56, 0} },  /* Config 14 */
  { {32, 8}, {40,16}, {56, 8} },  /* Config 15 = Config 0 for 11.0, SPR:372359 */
  { {32, 8}, {40, 8}, {48,16} }   /* Config 16 = Config 1 for 11.0, SPR:372359 */
};

#define NIOS2_NUM_EXT_REG_MAPS \
  ((sizeof nios2_builtin_ext_reg_maps) / (sizeof (nios2_builtin_ext_reg_maps[0])))
const int nios2_num_builtin_ext_reg_maps = NIOS2_NUM_EXT_REG_MAPS;

struct nios2_ext_reg_map *nios2_ext_reg_maps = (struct nios2_ext_reg_map *) nios2_builtin_ext_reg_maps;
int nios2_num_ext_reg_maps = NIOS2_NUM_EXT_REG_MAPS;
#undef NIOS2_NUM_EXT_REG_MAPS


/* Checks whether the register name has the given prefix
   followed by a number. If yes, return the number. If
   no return -1 */
int
nios2_get_register_num(const char *reg_name, const char *reg_prefix)
{
  assert(reg_name != NULL);

  if (strncmp (reg_name, reg_prefix, strlen (reg_prefix)) != 0)
    return -1;
  
  const char *reg_index_ptr = &reg_name[strlen(reg_prefix)];

  if (!isdigit(*reg_index_ptr))
    return -1;

  int reg_num = 0;
  do
    {
      reg_num *= 10;
      reg_num += *reg_index_ptr - '0';
      ++reg_index_ptr;
    }
  while (isdigit(*reg_index_ptr));

  return reg_num;   
}


/* Given an extension register "reg_name" and config_num (0-14) mapping
     EXAMPLES: rx0, tx0, cr0, cri0, cro0
    -> return -1 if the register does not exist
    -> return the general purpose register index (r32 to r63) if 
       register exists */
int
nios2_get_extension_register_index(const char *reg_name, int config_num)
{

  assert(config_num < nios2_num_ext_reg_maps);
  const struct nios2_ext_reg_map ext_reg_map = nios2_builtin_ext_reg_maps[config_num];

  int reg_num;
 
  if ( ((reg_num = nios2_get_register_num(reg_name,RX_REG())) != -1) 
       || ((reg_num = nios2_get_register_num(reg_name,TX_REG())) != -1))
    {
      if (reg_num < ext_reg_map.rx_tx.num_regs)
        return reg_num + ext_reg_map.rx_tx.offset;
    }
    
  else if ( ((reg_num = nios2_get_register_num(reg_name,CR_REG())) != -1) )
    {
      if (reg_num < ext_reg_map.cr.num_regs)
        return reg_num + ext_reg_map.cr.offset;
    }

  else if ( ((reg_num = nios2_get_register_num(reg_name,CRI_REG())) != -1) 
       || ((reg_num = nios2_get_register_num(reg_name,CRO_REG())) != -1))
    {
      if (reg_num < ext_reg_map.cri_cro.num_regs)
        return reg_num + ext_reg_map.cri_cro.offset;
    }

  return -1;

}

/*
On success, the total number of characters written is returned. This count does not include the additional null-character automatically appended at the end of the string.
On failure, a negative number is returned.
*/
int
nios2_get_register_alias(char *alias, int alias_size, const char *reg_name, int config_num, int is_dest) 
{

  int gp_reg_num = nios2_get_register_num(reg_name, R_REG());

  if (gp_reg_num == -1)
    return -1;

  const struct nios2_ext_reg_map ext_reg_map = nios2_builtin_ext_reg_maps[config_num];

  if (ext_reg_map.rx_tx.num_regs > 0 )
    {
      int r_offset = gp_reg_num - ext_reg_map.rx_tx.offset;  
      if (r_offset >= 0 && r_offset < ext_reg_map.rx_tx.num_regs)
        {
          if (is_dest) 
            {
              if (alias_size > (int)(strlen(TX_REG()) + 3))     
                return sprintf(alias,TX_REG(%d),r_offset);
            }
          else
            {
              if (alias_size > (int)(strlen(RX_REG()) + 3))
                return sprintf(alias,RX_REG(%d),r_offset);
            }
        }
    }

  if (ext_reg_map.cr.num_regs > 0 )
    {
      int r_offset = gp_reg_num - ext_reg_map.cr.offset;  
      if (r_offset >= 0 && r_offset < ext_reg_map.cr.num_regs)
        {
          if (alias_size > (int)(strlen(CR_REG()) + 3))     
            return sprintf(alias,CR_REG(%d),r_offset);
        }
    }

  if (ext_reg_map.cri_cro.num_regs > 0 )
    {
      int r_offset = gp_reg_num - ext_reg_map.cri_cro.offset;  
      if (r_offset >= 0 && r_offset < ext_reg_map.cri_cro.num_regs)
        {
          if (is_dest) 
            {
              if (alias_size > (int)(strlen(CRO_REG()) + 3))     
                return sprintf(alias,CRO_REG(%d),r_offset);
            }
          else
            {
              if (alias_size > (int)(strlen(CRI_REG()) + 3))
                return sprintf(alias,CRI_REG(%d),r_offset);
            }
        }
    }

  return -1;
}

/* overflow message string templates */

char *overflow_msgs[] = {
  "call target address 0x%08x out of range 0x%08x to 0x%08x",
  "branch offset %d out of range %d to %d",
  "%s offset %d out of range %d to %d",
  "immediate value %d out of range %d to %d",
  "immediate value %d out of range %d to %d",
  "immediate value %u out of range %u to %u",
  "immediate value %u out of range %u to %u",
  "immediate value %u out of range %u to %u",
  "immediate value %u out of range %u to %u",
  "custom instruction opcode %u out of range %u to %u",
};




/*--------------------------------------------------------------------------------
   This is the opcode table used by the New Jersey GNU as, disassembler and GDB
  --------------------------------------------------------------------------------*/

/*
       The following letters can appear in the args field of the nios2_opcode
       structure:

       c - a 5/6-bit control register index or break opcode
       d - a 5/6-bit destination register index (c register)
       s - a 5/6-bit left source register index (a register)
       t - a 5/6-bit right source register index (b register)

       i - a 16-bit signed immediate
       u - a 16-bit unsigned immediate

       h - a 5-bit unsigned immediate (BMX upper 5 bits)
       j - a 5-bit unsigned immediate (I5 & BMX lower 5 bits)
       k - a 6-bit unsigned immediate --> NOT Used with 6B
       l - an 8-bit unsigned immediate (used by custom instuction)
       m - a 26-bit unsigned immediate

       n - a 12-bit signed immediate
       p - a 12-bit unsigned immediate
       r - a 20-bit unsigned immediate

       o - movia uses o! where is o defined? I assume it's any 32 bit value
       q - a 14-bit signed immediate, word aligned (2 LSBs are ignored)

       b - break and trap use b! what is b? where is b defined? It should be:
           an optional 5 bit unsigned immediate. Kindof like j, but optional
*/

/* *INDENT-OFF* */
/* FIXME: Re-format for GNU standards */
const struct nios2_opcode nios2_builtin_opcodes[] =
{
/* name,           args,           args_test      num_args,  match,                  mask,                                   pinfo,                             overflow_msg,           dest_reg   reg restrictions */
  {"add",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_ADD,           OP_MASK_ADD,                            0,	                                no_overflow,               'd',        "t" },
  {"addi",         "t,s,n",        "t,s,n,E",     3,         OP_MATCH_ADDI,          OP_MASK_ADDI,                           NIOS2_INSN_ADDI,                   signed_immed12_overflow,   't',        ""  },
  {"and",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_AND,           OP_MASK_AND,                            0,                                 no_overflow,               'd',        "t" },
  {"andhi",        "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ANDHI,         OP_MASK_ANDHI,                          0,					unsigned_immed16_overflow, 't',        ""  },
  {"andi",         "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ANDI,          OP_MASK_ANDI,                           NIOS2_INSN_ANDI,			unsigned_immed16_overflow, 't',        ""  },
  {"andci",        "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ANDCI,         OP_MASK_ANDCI,                          0,					unsigned_immed16_overflow, 't',        ""  },
  {"andchi",       "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ANDCHI,        OP_MASK_ANDCHI,                         0,					unsigned_immed16_overflow, 't',        ""  },
  {"beq",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BEQ,           OP_MASK_BEQ,                            NIOS2_INSN_CBRANCH,  	        branch_target_overflow,    0,          "t" },
  {"bge",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BGE,           OP_MASK_BGE,                            NIOS2_INSN_CBRANCH,	        branch_target_overflow,    0,          "t" },
  {"bgeu",         "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BGEU,          OP_MASK_BGEU,                           NIOS2_INSN_CBRANCH,	        branch_target_overflow,    0,          "t" },
  {"bgt",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BLT,           OP_MASK_BLT,                            NIOS2_INSN_MACRO|NIOS2_INSN_CBRANCH, branch_target_overflow,  0,          "t" },
  {"bgtu",         "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BLTU,          OP_MASK_BLTU,                           NIOS2_INSN_MACRO|NIOS2_INSN_CBRANCH, branch_target_overflow,  0,          "t" },
  {"ble",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BGE,           OP_MASK_BGE,                            NIOS2_INSN_MACRO|NIOS2_INSN_CBRANCH, branch_target_overflow,  0,          "t" },
  {"bleu",         "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BGEU,          OP_MASK_BGEU,                           NIOS2_INSN_MACRO|NIOS2_INSN_CBRANCH, branch_target_overflow,  0,          "t" },
  {"blt",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BLT,           OP_MASK_BLT,                            NIOS2_INSN_CBRANCH,                branch_target_overflow,    0,          "t" },
  {"bltu",         "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BLTU,          OP_MASK_BLTU,                           NIOS2_INSN_CBRANCH,		branch_target_overflow,    0,          "t" },
  {"bne",          "s,t,q",        "s,t,q,E",     3,         OP_MATCH_BNE,           OP_MASK_BNE,                            NIOS2_INSN_CBRANCH,		branch_target_overflow,    0,          "t" },
  {"br",           "q",            "q,E",         1,         OP_MATCH_BR,            OP_MASK_BR,                             NIOS2_INSN_MACRO|NIOS2_INSN_CBRANCH, branch_target_overflow,  0,          ""  },
  {"break",        "b",            "b,E",         1,         OP_MATCH_BREAK,         OP_MASK_BREAK,                          0,	                                unsigned_immed5_overflow,  0,          ""  },
  {"bret",         "",             "E",           0,         OP_MATCH_BRET,          OP_MASK_BRET,                           0, 			        no_overflow,               0,          ""  },
  {"call",         "m",            "m,E",         1,         OP_MATCH_CALL,          OP_MASK_CALL,                           NIOS2_INSN_CALL,			call_target_overflow,      0,          ""  },
  {"callr",        "s",            "s,E",         1,         OP_MATCH_CALLR,         OP_MASK_CALLR,                          0, 				no_overflow,               0,          ""  },
  {"cidalloc",     "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CIDALLOC,      OP_MASK_CIDALLOC,                       0,					no_overflow,               0,          ""  },
  {"cmpeq",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPEQ,         OP_MASK_CMPEQ,              	     0,                                 no_overflow,               'd',        "t" },
  {"cmpeqi",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPEQI,        OP_MASK_CMPEQI,                         0,					signed_immed12_overflow,   't',        ""  },
  {"cmpge",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPGE,         OP_MASK_CMPGE,                          0, 				no_overflow,               'd',        "t" },
  {"cmpgei",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPGEI,        OP_MASK_CMPGEI,                         0,					signed_immed12_overflow,   't',        ""  },
  {"cmpgeu",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPGEU,        OP_MASK_CMPGEU,                         0, 				no_overflow,               'd',        "t" },
  {"cmpgeui",      "t,s,p",        "t,s,p,E",     3,         OP_MATCH_CMPGEUI,       OP_MASK_CMPGEUI,                        0,					unsigned_immed12_overflow, 't',        ""  },
  {"cmpgt",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPLT,         OP_MASK_CMPLT,                          NIOS2_INSN_MACRO, 			no_overflow,               'd',        "s" },
  {"cmpgti",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPGEI,        OP_MASK_CMPGEI,                         NIOS2_INSN_MACRO,			signed_immed12_overflow,   't',        ""  },
  {"cmpgtu",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPLTU,        OP_MASK_CMPLTU,                         NIOS2_INSN_MACRO, 			no_overflow,               'd',        "s" },
  {"cmpgtui",      "t,s,p",        "t,s,p,E",     3,         OP_MATCH_CMPGEUI,       OP_MASK_CMPGEUI,                        NIOS2_INSN_MACRO,			unsigned_immed12_overflow, 't',        ""  },
  {"cmple",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPGE,         OP_MASK_CMPGE,                          NIOS2_INSN_MACRO, 			no_overflow,               'd',        "s" },
  {"cmplei",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPLTI,        OP_MASK_CMPLTI,                         NIOS2_INSN_MACRO,			signed_immed12_overflow,   't',        ""  },
  {"cmpleu",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPGEU,        OP_MASK_CMPGEU,                         NIOS2_INSN_MACRO, 			no_overflow,               'd',        "s" },
  {"cmpleui",      "t,s,p",        "t,s,p,E",     3,         OP_MATCH_CMPLTUI,       OP_MASK_CMPLTUI,                        NIOS2_INSN_MACRO,			unsigned_immed12_overflow, 't',        ""  },
  {"cmplt",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPLT,         OP_MASK_CMPLT,                          0,					no_overflow,               'd',        "t" },
  {"cmplti",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPLTI,        OP_MASK_CMPLTI,                         0,					signed_immed12_overflow,   't',        ""  },
  {"cmpltu",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPLTU,        OP_MASK_CMPLTU,                         0,					no_overflow,               'd',        "t" },
  {"cmpltui",      "t,s,p",        "t,s,p,E",     3,         OP_MATCH_CMPLTUI,       OP_MASK_CMPLTUI,                        0,				        unsigned_immed12_overflow, 't',        ""  },
  {"cmpne",        "d,s,t",        "d,s,t,E",     3,         OP_MATCH_CMPNE,         OP_MASK_CMPNE,                          0, 				no_overflow,               'd',        "t" },
  {"cmpnei",       "t,s,n",        "t,s,n,E",     3,         OP_MATCH_CMPNEI,        OP_MASK_CMPNEI,                         0,					signed_immed12_overflow,   't',        ""  },
  {"custom",       "l,d,s,t",      "l,d,s,t,E",   4,         OP_MATCH_CUSTOM,        OP_MASK_CUSTOM,                         0,	 				custom_opcode_overflow,    'd',        "dst" },
  {"exit",         "",             "E",           0,         OP_MATCH_EXIT,          OP_MASK_EXIT,                           0, 				no_overflow,               0,          ""  },
  {"eret",         "",             "E",           0,         OP_MATCH_ERET,          OP_MASK_ERET,                           0,					no_overflow,               0,          ""  },
  {"extract",      "t,s,h,j",      "t,s,h,j,E",   4,         OP_MATCH_EXTRACT,       OP_MASK_EXTRACT,                        0, 				unsigned_immed5_overflow,  't',        ""  },
  {"flushd",       "n(s)",         "n(s)E",       2,         OP_MATCH_FLUSHD,        OP_MASK_FLUSHD,                         0,					signed_immed12_overflow,   0,          ""  },
  {"flushda",      "n(s)",         "n(s)E",       2,         OP_MATCH_FLUSHDA,       OP_MASK_FLUSHDA,                        0,					signed_immed12_overflow,   0,          ""  },
  {"fork",         "d,s,t",        "d,s,t,E",     3,         OP_MATCH_FORK,          OP_MASK_FORK,                           0, 				no_overflow,               0,          ""  },
  {"initd",        "n(s)",         "n(s)E",       2,         OP_MATCH_INITD,         OP_MASK_INITD,                          0,					signed_immed12_overflow,   0,          ""  },
  {"initda",       "n(s)",         "n(s)E",       2,         OP_MATCH_INITDA,        OP_MASK_INITDA,                         0,                                 signed_immed12_overflow,   0,          ""  },
  {"insert",       "t,s,h,j",      "t,s,h,j,E",   4,         OP_MATCH_INSERT,        OP_MASK_INSERT,                         0, 			        unsigned_immed5_overflow,  't',        "t" },
  {"jmp",          "s",            "s,E",         1,         OP_MATCH_JMP,           OP_MASK_JMP,                            0, 				no_overflow,               0,          ""  },
  {"jmpi",         "m",            "m,E",         1,         OP_MATCH_JMPI,          OP_MASK_JMPI,                           0,					no_overflow,               0,          ""  },
  {"jrel",         "s",            "s,E",         1,         OP_MATCH_JREL,          OP_MASK_JREL,                           0,					no_overflow,               0,          ""  },
  {"ldb",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDB,           OP_MASK_LDB,                            0,					address_offset_overflow,   's',        "t" },
  {"ldbio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDB,           OP_MASK_LDB,                            NIOS2_INSN_LOAD_STORE,             address_offset_overflow,   's',        "t" },
  {"ldbu",         "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDBU,          OP_MASK_LDBU,                           0,					address_offset_overflow,   's',        "t" },
  {"ldbuio",       "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDBU,          OP_MASK_LDBU,                           NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   's',        "t" },
  {"ldh",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDH,           OP_MASK_LDH,                            0,					address_offset_overflow,   's',        "t" },
  {"ldhio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDH,           OP_MASK_LDH,                            NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   's',        "t" },
  {"ldhu",         "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDHU,          OP_MASK_LDHU,                           0,					address_offset_overflow,   's',        "t" },
  {"ldhuio",       "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDHU,          OP_MASK_LDHU,                           NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   's',        "t" },
  {"ldw",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDW,           OP_MASK_LDW,                            0,					address_offset_overflow,   's',        "t" },
  {"ldwio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_LDW,           OP_MASK_LDW,                            NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   's',        "t" },
  {"merge",        "s,t,h,j",      "s,t,h,j,E",   4,         OP_MATCH_MERGE,         OP_MASK_MERGE,                          0, 				unsigned_immed5_overflow,  's',        ""  },
  {"mov",          "d,s",          "d,s,E",       2,         OP_MATCH_ADD,           OP_MASK_RRT|OP_MASK_ADD,                NIOS2_INSN_MACRO_MOV, 		no_overflow,               'd',        ""  },
  {"movhi",        "t,u",          "t,u,E",       2,         OP_MATCH_ORHI,          OP_MASK_IRS|OP_MASK_ORHI,               NIOS2_INSN_MACRO_MOVI,		unsigned_immed16_overflow, 't',        ""  },
  {"movhi20",      "s,r",          "s,r,E",       2,         OP_MATCH_MOVHI20,       OP_MASK_MOVHI20,                        0,	                                unsigned_immed20_overflow, 's',        ""  },
  /* movi expands to addi t,zero,n, n is a signed 12 bit value */
  {"movi",         "t,n",          "t,n,E",       2,         OP_MATCH_ADDI,          OP_MASK_IRS|OP_MASK_ADDI,               NIOS2_INSN_MACRO_MOVI,		signed_immed12_overflow,   't',        ""  },
  /* movia expands to two instructions so there is no mask or match */
  {"movia",        "s,o",          "s,o,E",       2,	     OP_MATCH_MOVHI20,       OP_MASK_MOVHI20,                        NIOS2_INSN_MOVIA,                  no_overflow,               's',        ""  },
  {"movui",        "t,u",          "t,u,E",       2,         OP_MATCH_ORI,           OP_MASK_IRS|OP_MASK_ORI,                NIOS2_INSN_MACRO_MOVI,		unsigned_immed16_overflow, 't',        ""  },
  {"mul",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_MUL,           OP_MASK_MUL,                            0, 				no_overflow,               'd',        "t" },
  {"muli",         "t,s,n",        "t,s,n,E",     3,         OP_MATCH_MULI,          OP_MASK_MULI,                           0,					signed_immed12_overflow,   't',        ""  },
  {"mulxss",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_MULXSS,        OP_MASK_MULXSS,                         0, 				no_overflow,               'd',        "t" },
  {"mulxsu",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_MULXSU,        OP_MASK_MULXSU,                         0, 				no_overflow,               'd',        "t" },
  {"mulxuu",       "d,s,t",        "d,s,t,E",     3 ,        OP_MATCH_MULXUU,        OP_MASK_MULXUU,                         0, 				no_overflow,               'd',        "t" },
  {"nextpc",       "d",            "d,E",         1,         OP_MATCH_NEXTPC,        OP_MASK_NEXTPC,                         0, 				no_overflow,               'd',        ""  },
  {"nop",          "",             "E",           0,         OP_MATCH_ADD,           OP_MASK,                                NIOS2_INSN_MACRO_MOV,		no_overflow,                0 ,        ""  },
  {"nor",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_NOR,           OP_MASK_NOR,                            0,					no_overflow,               'd',        "t" },
  {"or",           "d,s,t",        "d,s,t,E",     3,         OP_MATCH_OR,            OP_MASK_OR,                             0,					no_overflow,               'd',        "t" },
  {"orhi",         "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ORHI,          OP_MASK_ORHI,                           0,					unsigned_immed16_overflow, 't',        ""  },
  {"ori",          "t,s,u",        "t,s,u,E",     3,         OP_MATCH_ORI,           OP_MASK_ORI,                            NIOS2_INSN_ORI,			unsigned_immed16_overflow, 't',        ""  },
  {"rcv",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_RCV,           OP_MASK_RCV,                            0, 				no_overflow,               0,          ""  },
  {"rcvi",         "t,s,u",        "t,s,u,E",     3,         OP_MATCH_RCVI,          OP_MASK_RCVI,                           0, 				unsigned_immed16_overflow, 0,          ""  },
  {"rdctl",        "d,c",          "d,c,E",       2,         OP_MATCH_RDCTL,         OP_MASK_RDCTL,                          0, 				no_overflow,               'd',        ""  },
  {"ret",          "",             "E",           0,         OP_MATCH_RET,           OP_MASK_RET,                            0, 				no_overflow,               0,          ""  },
  {"rol",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_ROL,           OP_MASK_ROL,                            0, 				no_overflow,               'd',        "t" },
  {"roli",         "t,s,j",        "t,s,j,E",     3,         OP_MATCH_ROLI,          OP_MASK_ROLI,                           0, 				unsigned_immed5_overflow,  't',        ""  },
  {"ror",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_ROR,           OP_MASK_ROR,                            0, 				no_overflow,               'd',        "t" },
  {"rxfree",       "d,s,t",        "d,s,t,E",     3,         OP_MATCH_RXFREE,        OP_MASK_RXFREE,                         0, 				no_overflow,               0,          ""  },
  {"sll",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_SLL,           OP_MASK_SLL,                            0, 				no_overflow,               'd',        "t" },
  {"slli",         "t,s,j",        "t,s,j,E",     3,         OP_MATCH_SLLI,          OP_MASK_SLLI,                           0, 				unsigned_immed5_overflow,  't',        ""  },
  {"snd",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_SND,           OP_MASK_SND,                            0, 				no_overflow,               'd',          "t"  },
  {"sndi",         "t,s,u",        "t,s,u,E",     3,         OP_MATCH_SNDI,          OP_MASK_SNDI,                           0, 				unsigned_immed16_overflow, 't',          ""  },
  {"sra",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_SRA,           OP_MASK_SRA,                            0, 				no_overflow,               'd',        "t" },
  {"srai",         "t,s,j",        "t,s,j,E",     3,         OP_MATCH_SRAI,          OP_MASK_SRAI,                           0, 				unsigned_immed5_overflow,  't',        ""  },
  {"srl",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_SRL,           OP_MASK_SRL,                            0,                                 no_overflow,               'd',        "t" },
  {"srli",         "t,s,j",        "t,s,j,E",     3,         OP_MATCH_SRLI,          OP_MASK_SRLI,                           0, 				unsigned_immed5_overflow,  't',        ""  },
  {"stb",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STB,           OP_MASK_STB,                            0,					address_offset_overflow,   0,          "t" },
  {"stbio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STB,           OP_MASK_STB,                            NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   0,          "t" },
  {"sth",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STH,           OP_MASK_STH,                            0,					address_offset_overflow,   0,          "t" },
  {"sthio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STH,           OP_MASK_STH,                            NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   0,          "t" },
  {"stw",          "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STW,           OP_MASK_STW,                            0,					address_offset_overflow,   0,          "t" },
  {"stwio",        "s,n(t)",       "s,n(t)E",     3,         OP_MATCH_STW,           OP_MASK_STW,                            NIOS2_INSN_LOAD_STORE,	        address_offset_overflow,   0,          "t" },
  {"sub",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_SUB,           OP_MASK_SUB,                            0, 				no_overflow,               'd',        "t" },
  {"subi",         "t,s,n",        "t,s,n,E",     3,         OP_MATCH_ADDI,          OP_MASK_ADDI,                           NIOS2_INSN_MACRO,                  signed_immed12_overflow,   't',        ""  },
  {"trap",         "b",             "b,E",        0,         OP_MATCH_TRAP,          OP_MASK_TRAP,                           0, 				unsigned_immed5_overflow,  0,          ""  },
  {"txalloc",      "d,s,t",        "d,s,t,E",     3,         OP_MATCH_TXALLOC,       OP_MASK_TXALLOC,                        0, 				no_overflow,               0,          ""  },
  {"wrctl",        "c,s",          "c,s,E",       2,         OP_MATCH_WRCTL,         OP_MASK_WRCTL,                          0, 				no_overflow,               'c',        ""  },
  {"xor",          "d,s,t",        "d,s,t,E",     3,         OP_MATCH_XOR,           OP_MASK_XOR,                            0, 				no_overflow,               'd',        "t" },
  {"xorhi",        "t,s,u",        "t,s,u,E",     3,         OP_MATCH_XORHI,         OP_MASK_XORHI,                          0,					unsigned_immed16_overflow, 't',        ""  },
  {"xori",         "t,s,u",        "t,s,u,E",     3,         OP_MATCH_XORI,          OP_MASK_XORI,                           NIOS2_INSN_XORI,			unsigned_immed16_overflow, 't',        ""  }
};
/* *INDENT-ON* */

#define NIOS2_NUM_OPCODES \
       ((sizeof nios2_builtin_opcodes) / (sizeof (nios2_builtin_opcodes[0])))
const int bfd_nios2_num_builtin_opcodes = NIOS2_NUM_OPCODES;

/* const removed from the following to allow for dynamic extensions to the
 * built-in instruction set. */
struct nios2_opcode *nios2_opcodes =
  (struct nios2_opcode *) nios2_builtin_opcodes;
int bfd_nios2_num_opcodes = NIOS2_NUM_OPCODES;
#undef NIOS2_NUM_OPCODES
