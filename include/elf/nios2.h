/* NOT ASSIGNED TO FSF.  COPYRIGHT ALTERA.  */
/* Altera New Jersey ELF support for BFD

   Copyright (C) 2003
   by Nigel Gray (ngray@altera.com).

   Copyright (C) 2010 Altera

This file is part of BFD, the Binary File Descriptor library.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* This file holds definitions specific to the Altera New Jersey ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_NIOS2_H
#define _ELF_NIOS2_H

#include "elf/reloc-macros.h"

/* the order of these numbers must match the order in
   the elf_nios2_howto_table_rel table for the lookup
   function to work properly */

START_RELOC_NUMBERS (elf_nios2_reloc_type)
  RELOC_NUMBER (R_NIOS2_NONE,	  0)
  RELOC_NUMBER (R_NIOS2_S16,	  1)
  RELOC_NUMBER (R_NIOS2_U16,	  2)
  RELOC_NUMBER (R_NIOS2_PCREL16, 3)
  RELOC_NUMBER (R_NIOS2_CALL26,  4)
  RELOC_NUMBER (R_NIOS2_IMM5,	  5)
  RELOC_NUMBER (R_NIOS2_CACHE_OPX, 6)
  RELOC_NUMBER (R_NIOS2_IMM6,	  7)
  RELOC_NUMBER (R_NIOS2_IMM8,	  8)
  RELOC_NUMBER (R_NIOS2_HI16,	  9)
  RELOC_NUMBER (R_NIOS2_LO16,	  10)
  RELOC_NUMBER (R_NIOS2_HIADJ16, 11)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_32, 12)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_16, 13)
  RELOC_NUMBER (R_NIOS2_BFD_RELOC_8, 14)
  RELOC_NUMBER (R_NIOS2_GPREL, 15)
  RELOC_NUMBER (R_NIOS2_GNU_VTINHERIT, 16)
  RELOC_NUMBER (R_NIOS2_GNU_VTENTRY, 17)
  RELOC_NUMBER (R_NIOS2_UJMP, 18)
  RELOC_NUMBER (R_NIOS2_CJMP, 19)
  RELOC_NUMBER (R_NIOS2_CALLR, 20)
  RELOC_NUMBER (R_NIOS2_ALIGN, 21)
  RELOC_NUMBER (R_NIOS2_GOT16, 22)
  RELOC_NUMBER (R_NIOS2_CALL16, 23)
  RELOC_NUMBER (R_NIOS2_GOTOFF_LO, 24)
  RELOC_NUMBER (R_NIOS2_GOTOFF_HA, 25)
  RELOC_NUMBER (R_NIOS2_PCREL_LO, 26)
  RELOC_NUMBER (R_NIOS2_PCREL_HA, 27)
  RELOC_NUMBER (R_NIOS2_TLS_GD16, 28)
  RELOC_NUMBER (R_NIOS2_TLS_LDM16, 29)
  RELOC_NUMBER (R_NIOS2_TLS_LDO16, 30)
  RELOC_NUMBER (R_NIOS2_TLS_IE16, 31)
  RELOC_NUMBER (R_NIOS2_TLS_LE16, 32)
  RELOC_NUMBER (R_NIOS2_TLS_DTPMOD, 33)
  RELOC_NUMBER (R_NIOS2_TLS_DTPREL, 34)
  RELOC_NUMBER (R_NIOS2_TLS_TPREL, 35)
  RELOC_NUMBER (R_NIOS2_COPY, 36)
  RELOC_NUMBER (R_NIOS2_GLOB_DAT, 37)
  RELOC_NUMBER (R_NIOS2_JUMP_SLOT, 38)
  RELOC_NUMBER (R_NIOS2_RELATIVE, 39)
  RELOC_NUMBER (R_NIOS2_GOTOFF, 40)
  RELOC_NUMBER (R_NIOS2_S12, 41)
  RELOC_NUMBER (R_NIOS2_U12, 42)
  RELOC_NUMBER (R_NIOS2_BMX_LSB, 43)
  RELOC_NUMBER (R_NIOS2_BMX_MSB, 44)
  RELOC_NUMBER (R_NIOS2_PCREL14_S2, 45)
  RELOC_NUMBER (R_NIOS2_U20, 46)
  RELOC_NUMBER (R_NIOS2_HI20, 47)
  RELOC_NUMBER (R_NIOS2_LO12, 48)
  RELOC_NUMBER (R_NIOS2_HIADJ20, 49)
  RELOC_NUMBER (R_NIOS2_ILLEGAL, 50)
END_RELOC_NUMBERS (R_NIOS2_maxext)

/* Processor specific section flags */



/* 4 bit NIOS2 architecture field.  */
#define EF_NIOS2_ARCH		0x0000000f

/* Nios2 (default)  */
#define E_NIOS2_ARCH_1		0x00000000

/* Nios2 DPX Architecture */
#define E_NIOS2_ARCH_DPX        0x00000004


/* 3 bit NIOS2 Encoding field.  */
#define EF_NIOS2_ENCODE          0x00000070

/* Nios2 5B Encoding  */
#define E_NIOS2_ENCODE_5B        0x00000000

/* Nios2 6B Encoding  */
#define E_NIOS2_ENCODE_6B        0x00000010


/* 4 bit NIOS2 Reg Map field.  */
#define EF_NIOS2_REGISTER_MAP    0x00000f00
#define E_NIOS2_REG_MAP_SH 8

#define E_NIOS2_REG_MAP(N)       ((N<<E_NIOS2_REG_MAP_SH) & EF_NIOS2_REGISTER_MAP)
#define E_NIOS2_REG_MAP_GET(EFLAGS) ((EFLAGS & EF_NIOS2_REGISTER_MAP)>>E_NIOS2_REG_MAP_SH) 


/* This is used to mark gp-relative sections */
#define SHF_NIOS2_GPREL	0x10000000

/* Processor specific dynamic array tags.  */

/* Address of _gp.  */
#define DT_NIOS2_GP 0x70000002

#endif //_ELF_NIOS2_H
