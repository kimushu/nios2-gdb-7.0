/**
 * @file interp.c
 * @brief NiosII instruction interpreter
 * @author kimu_shu
 */

#include "config.h"
#include "gdb/callback.h"
#include "gdb/signals.h"
#include "libiberty.h"
#include "gdb/remote-sim.h"
#include "sim-config.h"
#include "sim-types.h"
#include "sim-utils.h"
#include "sim-nios2.h"
#include "sim-main.h"

typedef unsigned32 uint32_t;
#include "asm_nios2.h"

nios2_cpu_t cpu;

int
nios2_reset(void)
{
  int i;

  /* initialize general-purpose registers */
  for (i = 0; i < 32; ++i)
    cpu.regs.array[i] = 0xdeadbeef;
  cpu.regs.zero = 0;

  /* initialize control registers */
  cpu.regs.status = NIOS2_STATUS_RSIE;
  cpu.regs.estatus = 0;
  cpu.regs.bstatus = 0;
  cpu.regs.ienable = 0;
  cpu.regs.ipending = 0;
  cpu.regs.cpuid = cpu.features.cpuid;
  cpu.regs.ctl6 = 0xffffffff;
  cpu.regs.exception = 0;
  if (cpu.features.mmu)
    {
      cpu.regs.pteaddr = 0;
      cpu.regs.tlbacc = 0;
      cpu.regs.tlbmisc = 0;
    }
  else
    {
      cpu.regs.pteaddr = 0xffffffff;
      cpu.regs.tlbacc = 0xffffffff;
      cpu.regs.tlbmisc = 0xffffffff;
    }
  cpu.regs.ctl11 = 0xffffffff;
  cpu.regs.badaddr = 0;
  if (cpu.features.mpu)
    {
      cpu.regs.config = 0;
      cpu.regs.mpubase = 0;
      cpu.regs.mpuacc = 0;
    }
  else
    {
      cpu.regs.config = 0xffffffff;
      cpu.regs.mpubase = 0xffffffff;
      cpu.regs.mpuacc = 0xffffffff;
    }

  /* jump to reset vector */
  cpu.regs.pc = cpu.features.reset_addr;

  return 1;
}

int
nios2_interpret(int step)
{
  unsigned32 i, a, b, c;
  unsigned32 opcode;
  unsigned32 nextpc;
  int flags;
  int magic;
  union {
    unsigned32  wu;
    signed32    w;
    unsigned16  hu;
    signed16    h;
    unsigned8   bu;
    signed8     b;
  } buf;

  if (step)
    {
      cpu.state = sim_stopped;
      cpu.signal = TARGET_SIGNAL_TRAP;
    }
  else
    cpu.state = sim_running;

  magic = 0;

  do
    {
      nextpc = cpu.regs.pc;

      if (avm_read(nextpc, (unsigned char *) &i, 4, AVM_INSTRUCTION) != 4)
        {
          sim_printf("instruction fetch failed (@%08x)\n", nextpc);
          return 0; /* instruction fetch failed */
        }

      i = LE2H4(i);

      opcode = NIOS2_OP(NIOS2_GET_OP(i));
      if (opcode == NIOS2_OP(0x3a))
        opcode |= NIOS2_OPX(NIOS2_GET_OPX(i));

      // sim_printf("@%08x (%08x) op=0x%02x, opx=0x%02x\n", nextpc, i, NIOS2_GET_OP(opcode), NIOS2_GET_OPX(opcode));

      nextpc += 4;
      switch(opcode)
        {
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x31):  /* add c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] + cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          case NIOS2_OP(0x04):                  /* addi b,a,sv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] + (signed16) NIOS2_GET_IMM16(i);
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x0e):  /* and c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] & cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          case NIOS2_OP(0x2c):                  /* andhi b,a,sv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] & (NIOS2_GET_IMM16(i) << 16);
            break;
          case NIOS2_OP(0x0c):                  /* andi b,a,sv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] & NIOS2_GET_IMM16(i);
            break;
          case NIOS2_OP(0x26):                  /* beq a,b,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] == cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x0e):                  /* bge a,b,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] >=
                (signed32) cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x2e):                  /* bgeu a,b,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] >= cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x16):                  /* blt a,b,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] <
                (signed32) cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x36):                  /* bltu a,b,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] < cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x1e):                  /* bne a,b,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] != cpu.regs.gpr[NIOS2_GET_B(i)])
              goto branch;
            break;
          case NIOS2_OP(0x06):                  /* br sv */
            if (NIOS2_GET_A(i) != 0 || NIOS2_GET_B(i) != 0)
              goto illegal_instruction_format;
branch:
            nextpc += (signed16) NIOS2_GET_IMM16(i);
            if (nextpc & 3)
              goto misaligned_destination_address;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x34):  /* break imm5 */
            if (NIOS2_GET_A(i) != 0 || NIOS2_GET_B(i) != 0 || NIOS2_GET_C(i) != 0x1e)
              goto illegal_instruction_format;
            // cpu.regs.bstatus = cpu.regs.status;
            // cpu.regs.status &= ~(NIOS2_STATUS_PIE | NIOS2_STATUS_U);
            // cpu.regs.ba = nextpc;
            // nextpc = cpu.features.break_addr;
            cpu.state = sim_stopped;
            cpu.signal = TARGET_SIGNAL_TRAP;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x09):  /* bret */
            if (NIOS2_GET_A(i) != 0x1e || NIOS2_GET_B(i) != 0 ||
                NIOS2_GET_C(i) != 0 || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            a = cpu.regs.ba;
            if (a & 3)
              goto misaligned_destination_address;
            if (cpu.regs.status & NIOS2_STATUS_U)
              goto supervisor_only_instruction;
            cpu.regs.status = cpu.regs.bstatus;
            nextpc = a;
            break;
          case NIOS2_OP(0x00):                  /* call imm26 */
            cpu.regs.ra = nextpc;
            nextpc = NIOS2_GET_IMM26(i) * 4;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1d):  /* callr a */
            if (NIOS2_GET_B(i) != 0 || NIOS2_GET_C(i) != 0x1f || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            a = cpu.regs.gpr[NIOS2_GET_A(i)];
            if (a & 3)
              goto misaligned_destination_address;
            cpu.regs.ra = nextpc;
            nextpc = a;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x20):  /* cmpeq c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if (cpu.regs.gpr[NIOS2_GET_A(i)] == cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x20):                  /* cmpeqi b,a,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] ==
                (signed16) NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x08):  /* cmpge c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] >=
                (signed32) cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x08):                  /* cmpgei b,a,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] >=
                (signed16) NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x28):  /* cmpgeu c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if (cpu.regs.gpr[NIOS2_GET_A(i)] >= cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x28):                  /* cmpgeui b,a,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] >= NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x10):  /* cmplt c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] <
                (signed32) cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x10):                  /* cmplti b,a,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] <
                (signed16) NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x30):  /* cmpltu c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if (cpu.regs.gpr[NIOS2_GET_A(i)] < cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x30):                  /* cmpltui b,a,sv */
            if (cpu.regs.gpr[NIOS2_GET_A(i)] < NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x18):  /* cmpne c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            if (cpu.regs.gpr[NIOS2_GET_A(i)] != cpu.regs.gpr[NIOS2_GET_B(i)])
              c = 1;
            else
              c = 0;
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x18):                  /* cmpnei b,a,sv */
            if ((signed32) cpu.regs.gpr[NIOS2_GET_A(i)] !=
                (signed16) NIOS2_GET_IMM16(i))
              b = 1;
            else
              b = 0;
            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          /* TODO: custom */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x25):  /* div c,a,b */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x24):  /* divu c,a,b */
            if (!cpu.features.hwdiv)
              goto unimplemented_instruction;
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            a = cpu.regs.gpr[NIOS2_GET_A(i)];
            b = cpu.regs.gpr[NIOS2_GET_B(i)];
            if (opcode & NIOS2_OPX(1))
              {
                /* signed */
                if (b == 0)
                  c = 0x80000000;
                else if (a == 0x80000000 && b == 0xffffffff)
                  c = 0x7fffffff;
                else
                  c = (signed32) a / (signed32)b;
              }
            else
              {
                /* unsigned */
                if (b == 0)
                  c = 0xffffffff;
                else
                  c = a / b;
              }
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x01):  /* eret */
            if (NIOS2_GET_A(i) != 0x1d || NIOS2_GET_B(i) != 0x1e ||
                NIOS2_GET_C(i) != 0 || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            a = cpu.regs.ea;
            if (a & 3)
              goto misaligned_destination_address;
            if (cpu.regs.status & NIOS2_STATUS_U)
              goto supervisor_only_instruction;
            cpu.regs.status = cpu.regs.estatus;
            nextpc = a;
            break;
          /* TODO: flushd */
          /* TODO: flushda */
          /* TODO: flushi */
          /* TODO: flushp */
          /* TODO: initd */
          /* TODO: initda */
          /* TODO: initi */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x0d):  /* jmp a */
            if (NIOS2_GET_B(i) != 0 || NIOS2_GET_C(i) != 0 || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            a = cpu.regs.gpr[NIOS2_GET_A(i)];
            if (a & 3)
              goto misaligned_destination_address;
            nextpc = a;
            break;
          case NIOS2_OP(0x01):                  /* jmpi imm26 */
            nextpc = NIOS2_GET_IMM26(i) * 4;
            break;
          case NIOS2_OP(0x07):                  /* ldb b,sv(a) */
          case NIOS2_OP(0x27):                  /* ldbio b,sv(a) */
          case NIOS2_OP(0x03):                  /* ldbu b,sv(a) */
          case NIOS2_OP(0x23):                  /* ldbuio b,sv(a) */
          case NIOS2_OP(0x0f):                  /* ldh b,sv(a) */
          case NIOS2_OP(0x2f):                  /* ldhio b,sv(a) */
          case NIOS2_OP(0x0b):                  /* ldhu b,sv(a) */
          case NIOS2_OP(0x2b):                  /* ldhuio b,sv(a) */
          case NIOS2_OP(0x17):                  /* ldw b,sv(a) */
          case NIOS2_OP(0x37):                  /* ldwio b,sv(a) */
            if (opcode & NIOS2_OP(0x10))
              c = 4;
            else if (opcode & NIOS2_OP(0x08))
              c = 2;
            else
              c = 1;

            a = cpu.regs.gpr[NIOS2_GET_A(i)] + (signed16) NIOS2_GET_IMM16(i);
            if ((a & (c - 1)) != 0)
              goto misaligned_data_address;

            flags = AVM_DATA | ((opcode & NIOS2_OP(0x20)) != 0 ? AVM_NOCACHE : 0);
            if (avm_read(a, (unsigned char *) &buf, c, AVM_DATA | flags) != c)
              /* TODO: data access failed */
              b = 0xffffffff;
            else if (c == 1)
              b = (opcode & NIOS2_OP(0x04)) ? (signed8) buf.b : buf.bu;
            else if (c == 2)
              b = (opcode & NIOS2_OP(0x04)) ? (signed16) buf.h : buf.hu;
            else
              b = buf.wu;

            cpu.regs.gpr[NIOS2_GET_B(i)] = b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x27):  /* mul c,a,b */
            if (!cpu.features.hwmul)
              goto unimplemented_instruction;
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] * cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          case NIOS2_OP(0x24):                  /* muli b,a,sv */
            if (!cpu.features.hwmul)
              goto unimplemented_instruction;
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] * (signed16) NIOS2_GET_IMM16(i);
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1f):  /* mulxss c,a,b */
            if (!cpu.features.hwmulx)
              goto unimplemented_instruction;
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              ((signed64) cpu.regs.gpr[NIOS2_GET_A(i)] *
                (signed32) cpu.regs.gpr[NIOS2_GET_B(i)]) >> 32;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x17):  /* mulxsu c,a,b */
            if (!cpu.features.hwmulx)
              goto unimplemented_instruction;
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              ((signed64) cpu.regs.gpr[NIOS2_GET_A(i)] *
                (unsigned32) cpu.regs.gpr[NIOS2_GET_B(i)]) >> 32;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x07):  /* mulxuu c,a,b */
            if (!cpu.features.hwmulx)
              goto unimplemented_instruction;
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              ((unsigned64) cpu.regs.gpr[NIOS2_GET_A(i)] *
                (unsigned32) cpu.regs.gpr[NIOS2_GET_B(i)]) >> 32;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1c):  /* nextpc c */
            if (NIOS2_GET_A(i) != 0 || NIOS2_GET_B(i) != 0 || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] = nextpc;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x06):  /* nor c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              ~(cpu.regs.gpr[NIOS2_GET_A(i)] | cpu.regs.gpr[NIOS2_GET_B(i)]);
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x16):  /* or c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] | cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          case NIOS2_OP(0x34):                  /* orhi b,a,uv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] | (NIOS2_GET_IMM16(i) << 16);
            break;
          case NIOS2_OP(0x14):                  /* ori b,a,uv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] | NIOS2_GET_IMM16(i);
            if (NIOS2_GET_B(i) != 0 || NIOS2_GET_A(i) != 0)
              break;
            magic = NIOS2_GET_IMM16(i);
            switch(magic)
              {
                case NIOS2_SYS_MAGIC:
                  goto keep_magic;
                case NIOS2_SYS_WRITE:
                  /* r4: file, r5: ptr, r6: len */
                  cpu.regs.gpr[2] = sim_sys_write(cpu.regs.gpr[4], cpu.regs.gpr[5], cpu.regs.gpr[6]);
                  break;
                case NIOS2_SYS__EXIT:
                  /* r4: exitcode */
                  cpu.signal = cpu.regs.gpr[4] & 255;
                  cpu.state = sim_exited;
                  break;
              }
            break;
          /* TODO: rdctl */
          /* TODO: rdprs */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x05):  /* ret */
            if (NIOS2_GET_A(i) != 0x1f || NIOS2_GET_B(i) != 0 ||
                NIOS2_GET_C(i) != 0 || NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            nextpc = cpu.regs.ra;
            if (nextpc & 3)
              goto misaligned_destination_address;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x03):  /* rol c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            b = cpu.regs.gpr[NIOS2_GET_B(i)] & 31;
            goto rotate;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x02):  /* roli c,a,imm5 */
            if (NIOS2_GET_B(i) != 0)
              goto illegal_instruction_format;
            b = NIOS2_GET_IMM5(i);
            goto rotate;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x0b):  /* ror c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            b = (32 - (cpu.regs.gpr[NIOS2_GET_B(i)] & 31)) & 31;
rotate:
            a = cpu.regs.gpr[NIOS2_GET_A(i)];
            if (b == 0)
              c = a;
            else
              c = (a << b) | (a >> (32 - b));
            cpu.regs.gpr[NIOS2_GET_C(i)] = c;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x13):  /* sll c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            b = cpu.regs.gpr[NIOS2_GET_B(i)] & 31;
            cpu.regs.gpr[NIOS2_GET_C(i)] = cpu.regs.gpr[NIOS2_GET_A(i)] << b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x12):  /* slli c,a,imm5 */
            if (NIOS2_GET_B(i) != 0)
              goto illegal_instruction_format;
            b = NIOS2_GET_IMM5(i);
            cpu.regs.gpr[NIOS2_GET_C(i)] = cpu.regs.gpr[NIOS2_GET_A(i)] << b;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x3b):  /* sra c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            b = cpu.regs.gpr[NIOS2_GET_B(i)] & 31;
            goto shift_right_arithmetic;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x3a):  /* srai c,a,imm5 */
            if (NIOS2_GET_B(i) != 0)
              goto illegal_instruction_format;
            b = NIOS2_GET_IMM5(i);
shift_right_arithmetic:
            cpu.regs.gpr[NIOS2_GET_C(i)] = cpu.regs.gpr[NIOS2_GET_A(i)];
            for (; b > 0; --b)
              *(signed32 *) &cpu.regs.gpr[NIOS2_GET_C(i)] /= 2;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1b):  /* srl c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            b = cpu.regs.gpr[NIOS2_GET_B(i)] & 31;
            goto shift_right_logical;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1a):  /* srli c,a,imm5 */
            if (NIOS2_GET_B(i) != 0)
              goto illegal_instruction_format;
            b = NIOS2_GET_IMM5(i);
shift_right_logical:
            cpu.regs.gpr[NIOS2_GET_C(i)] = cpu.regs.gpr[NIOS2_GET_A(i)];
            for (; b > 0; --b)
              cpu.regs.gpr[NIOS2_GET_C(i)] /= 2;
            break;
          case NIOS2_OP(0x05):                  /* stb b,sv,a */
          case NIOS2_OP(0x25):                  /* stbio b,sv,a */
          case NIOS2_OP(0x0d):                  /* sth b,sv,a */
          case NIOS2_OP(0x2d):                  /* sthio b,sv,a */
          case NIOS2_OP(0x15):                  /* stw b,sv,a */
          case NIOS2_OP(0x35):                  /* stwio b,sv,a */
            b = cpu.regs.gpr[NIOS2_GET_B(i)];
            if (opcode & NIOS2_OP(0x10))
              {
                c = 4;
                buf.wu = b;
              }
            else if (opcode & NIOS2_OP(0x08))
              {
                c = 2;
                buf.hu = b;
              }
            else
              {
                c = 1;
                buf.bu = b;
              }

            a = cpu.regs.gpr[NIOS2_GET_A(i)] + (signed16) NIOS2_GET_IMM16(i);
            if ((a & (c - 1)) != 0)
              goto misaligned_data_address;

            flags = AVM_DATA | ((opcode & NIOS2_OP(0x20)) != 0 ? AVM_NOCACHE : 0);
            if (avm_write(a, (unsigned char *) &buf, c, AVM_DATA | flags) != c)
              /* TODO: data access failed */
              goto nonmpu_region_violation;
            break;
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x39):  /* sub c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] - cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          /* TODO: sync */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x2d):  /* trap imm5 */
            if (NIOS2_GET_A(i) != 0 || NIOS2_GET_B(i) != 0 || NIOS2_GET_C(i) != 0x1d)
              goto illegal_instruction_format;
            // cpu.regs.estatus = cpu.regs.status;
            // cpu.regs.status &= ~(NIOS2_STATUS_PIE | NIOS2_STATUS_U);
            // cpu.regs.ea = nextpc;
            // nextpc = cpu.features.exception_addr;
            cpu.state = sim_stopped;
            cpu.signal = TARGET_SIGNAL_TRAP;
            nextpc -= 4;
            break;
          /* TODO: wrctl */
          /* TODO: wrprs */
          case NIOS2_OP(0x3a)|NIOS2_OPX(0x1e):  /* xor c,a,b */
            if (NIOS2_GET_IMM5(i) != 0)
              goto illegal_instruction_format;
            cpu.regs.gpr[NIOS2_GET_C(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] ^ cpu.regs.gpr[NIOS2_GET_B(i)];
            break;
          case NIOS2_OP(0x3c):                  /* xorhi b,a,uv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] ^ (NIOS2_GET_IMM16(i) << 16);
            break;
          case NIOS2_OP(0x1c):                  /* xori b,a,uv */
            cpu.regs.gpr[NIOS2_GET_B(i)] =
              cpu.regs.gpr[NIOS2_GET_A(i)] ^ NIOS2_GET_IMM16(i);
            break;
          default:
            goto unimplemented_instruction;
        }

      magic = 0;
keep_magic:
      cpu.regs.zero = 0;
      cpu.regs.pc = nextpc;
      continue;

nonmpu_region_violation:
      sim_printf("nonmpu_region_violation\n");
      goto stop_running;
unimplemented_instruction:
      sim_printf("unimplemented_instruction\n");
      goto stop_running;
misaligned_data_address:
      sim_printf("misaligned_data_address\n");
      goto stop_running;
misaligned_destination_address:
      sim_printf("misaligned_destination_address\n");
      goto stop_running;
supervisor_only_instruction:
      sim_printf("supervisor_only_instruction\n");
      goto stop_running;
illegal_instruction_format:
      sim_printf("illegal_instruction_format\n");
stop_running:
      cpu.state = sim_stopped;
      cpu.signal = TARGET_SIGNAL_TRAP;
      continue;
    }
  while(cpu.state == sim_running);

}

/*
 * vim:et sts=2 sw=2:
 */
