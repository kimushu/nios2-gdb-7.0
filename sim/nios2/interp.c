/**
 * @file interp.c
 * @brief Nios2 simulator
 * @author kimu_shu
 */

#include "config.h"
#include "bfd.h"
#include "gdb/callback.h"
// #include "gdb/signals.h"
#include "libiberty.h"
#include "gdb/remote-sim.h"
#include "sim-config.h"
#include "sim-types.h"
#include "sim-utils.h"
#include "gdb/sim-nios2.h"

#define status_RSIE     23
#define status_NMI      22
#define status_PRS      16
#define status_CRS      10
#define status_IL       4
#define status_IH       3
#define status_EH       2
#define status_U        1
#define status_PIE      0

static SIM_OPEN_KIND sim_kind;
static struct host_callback_struct *sim_callback;
static struct bfd *sim_bfd;
static char *sim_name;
static char sim_cpu[256];
static char sim_loading;

union regset_t
  {
    unsigned32 array[1];
    struct
      {
        unsigned32
          zero, at, r2, r3, r4, r5, r6, r7,
          r8, r9, r10, r11, r12, r13, r14, r15,
          r16, r17, r18, r19, r20, r21, r22, r23,
          et, bt, gp, sp, fp, ea, ba, ra,
          pc,
          status, estatus, bstatus,
          ienable, ipending, cpuid,
          ctl6, exception, pteaddr, tlbacc, tlbmisc,
          ctl11, badaddr, config, mpubase, mpuacc;
      };
  };

struct feature_t
  {
    unsigned hwdiv  : 1;  /* embeddedsw.CMacro.HARDWARE_DIVIDE_PRESENT */
    unsigned hwmul  : 1;  /* embeddedsw.CMacro.HARDWARE_MULTIPLY_PRESENT */
    unsigned hwmulx : 1;  /* embeddedsw.CMacro.HARDWARE_MULX_PRESENT */
  };

static union regset_t regs;
static struct feature_t feature;
static unsigned32 cpu_freq;

static void
store_le32 (unsigned char *buf, unsigned32 val)
{
  buf[3] = val >> 24;
  buf[2] = val >> 16;
  buf[1] = val >> 8;
  buf[0] = val >> 0;
}

static SIM_RC
sopcinfo_open (void)
{
  return SIM_RC_FAIL;
}

static SIM_RC
sopcinfo_get_i (const char *module, const char *path, unsigned32 *buffer)
{
  return SIM_RC_FAIL;
}

static SIM_RC
select_cpu (const char *cpu)
{
  return SIM_RC_FAIL;
}

SIM_DESC
sim_open (SIM_OPEN_KIND kind, struct host_callback_struct *callback, struct bfd *abfd, char **argv)
{
  printf("sim_open()\n");
  sim_kind = kind;
  sim_callback = callback;
  sim_bfd = abfd;
  sim_name = argv[0];

  return (SIM_DESC) 1;
}

void
sim_close (SIM_DESC sd, int quitting)
{
  /* nothing to do */
  printf("sim_close()\n");
}

SIM_RC
sim_load (SIM_DESC sd, char *prog, struct bfd *abfd, int from_tty)
{
  printf("sim_load(prog=%s)\n", prog);
  bfd *prog_bfd;

  sim_loading = 1;
  prog_bfd = sim_load_file (sd, sim_name, sim_callback, prog, abfd,
                            sim_kind == SIM_OPEN_DEBUG,
                            0, sim_write);
  sim_loading = 0;

  if (prog_bfd == NULL)
    return SIM_RC_FAIL;

  if (abfd != NULL)
    sim_bfd = abfd;

  return SIM_RC_OK;
}

SIM_RC
sim_create_inferior (SIM_DESC sd, struct bfd *abfd, char **argv, char **env)
{
  SIM_RC result;
  unsigned32 pc, tmp;
  int i;

  printf("sim_create_inferior()\n");
  result = sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.RESET_ADDR", &pc);
  if (result != SIM_RC_OK)
    {
      pc = bfd_get_start_address(sim_bfd);
      feature.hwdiv = 1;
      feature.hwmul = 1;
      feature.hwmulx = 1;
    }
  else
    {
      tmp = 1;
      result = sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.HARDWARE_DIVIDE_PRESENT", &tmp);
      feature.hwdiv = (tmp != 0) ? 1 : 0;

      tmp = 1;
      result = sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.HARDWARE_MULTIPLY_PRESENT", &tmp);
      feature.hwmul = (tmp != 0) ? 1 : 0;

      tmp = 1;
      result = sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.HARDWARE_MULX_PRESENT", &tmp);
      feature.hwmulx = (tmp != 0) ? 1 : 0;
    }

  regs.zero = 0;

  for (i = SIM_NIOS2_R1_REGNUM; i <= SIM_NIOS2_R31_REGNUM; ++i)
    regs.array[i] = 0xdeadbeef;

  for (; i <= SIM_NIOS2_MPUACC_REGNUM; ++i)
    regs.array[i] = 0x00000000;

  regs.pc = pc;
  regs.status = (1 << status_RSIE);
  sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.CPU_ID_VALUE", &regs.cpuid);

  cpu_freq = 100000000;
  sopcinfo_get_i(sim_cpu, "embeddedsw.CMacro.CPU_FREQ", &cpu_freq);

  return SIM_RC_OK;
}

int
sim_read (SIM_DESC sd, SIM_ADDR mem, unsigned char *buf, int length)
{
  printf("sim_read (mem=0x%x, len=0x%x)\n", mem, length);
  return length;
}

int
sim_write (SIM_DESC sd, SIM_ADDR mem, unsigned char *buf, int length)
{
  printf("sim_write (mem=0x%x, len=0x%x)\n", mem, length);
  return length;
}

int
sim_fetch_register (SIM_DESC sd, int regno, unsigned char *buf, int length)
{
  if (length != 4)
    return 4;

  if (regno >= SIM_NIOS2_R0_REGNUM && regno <= SIM_NIOS2_MPUACC_REGNUM)
    {
      store_le32(buf, regs.array[regno]);
      return -1;
    }

  return 0;
}

int
sim_store_register (SIM_DESC sd, int regno, unsigned char *buf, int length)
{
}

void
sim_info (SIM_DESC sd, int verbose)
{
}

void
sim_resume (SIM_DESC sd, int step, int siggnal)
{
}

int
sim_stop (SIM_DESC sd)
{
}

void
sim_stop_reason (SIM_DESC sd, enum sim_stop *reason, int *sigrc)
{
}

void
sim_do_command (SIM_DESC sd, char *cmd)
{
}

void
sim_set_callbacks (struct host_callback_struct *ptr)
{
  /* DEPRECATED */
}

void
sim_size (int i)
{
  /* DEPRECATED */
}

int
sim_trace (SIM_DESC sd)
{
  /* DEPRECATED */
  return 1;
}

/*
 * vim:et sts=2 sw=2:
 */
