/**
 * @file sim-main.c
 * @brief NiosII simulator main
 * @author kimu_shu
 */

#include "config.h"
#include "bfd.h"
#include "gdb/callback.h"
#include "gdb/signals.h"
#include "libiberty.h"
#include "gdb/remote-sim.h"
#include "sim-main.h"
#include "sim-nios2.h"
#include <stdarg.h>

static SIM_OPEN_KIND sim_kind;
static struct host_callback_struct *sim_callback;
static struct bfd *sim_bfd;
static char *sim_name;
static char sim_cpu[256];
static char sim_loading;

struct feature_t
  {
    unsigned hwdiv  : 1;  /* embeddedsw.CMacro.HARDWARE_DIVIDE_PRESENT */
    unsigned hwmul  : 1;  /* embeddedsw.CMacro.HARDWARE_MULTIPLY_PRESENT */
    unsigned hwmulx : 1;  /* embeddedsw.CMacro.HARDWARE_MULX_PRESENT */
  };

static union regset_t regs;
static struct feature_t feature;
static unsigned32 cpu_freq;

void
sim_printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  if(sim_callback)
    {
      (*sim_callback->vprintf_filtered)(sim_callback, fmt, args);
    }
  va_end(args);
}

SIM_DESC
sim_open (SIM_OPEN_KIND kind, struct host_callback_struct *callback, struct bfd *abfd, char **argv)
{
  sim_kind = kind;
  sim_callback = callback;
  sim_bfd = abfd;
  sim_name = argv[0];

  if(0)
    {
      char **p;
      for (p = argv; *p; ++p) sim_printf("argv: %s\n", *p);
    }

  return (SIM_DESC) 1;
}

void
sim_close (SIM_DESC sd, int quitting)
{
  avm_clear_sections();
}

SIM_RC
sim_load (SIM_DESC sd, char *prog, struct bfd *abfd, int from_tty)
{
  bfd *prog_bfd;
  SIM_ADDR end;

  sim_loading = 1;
  prog_bfd = sim_load_file (sd, sim_name, sim_callback, prog, abfd,
                            sim_kind == SIM_OPEN_DEBUG,
                            0, sim_write);

  if (avm_end_address(&end, AVM_INSTRUCTION | AVM_DATA) >= 0)
    avm_add_memory("heap", end, AVM_DATA | AVM_INSTRUCTION, NULL, 0x800000);
  avm_add_memory("stack", 0x7800000, AVM_DATA | AVM_INSTRUCTION, NULL, 0x800000);
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

  if(0)
    {
      char **p;
      for (p = argv; *p; ++p) sim_printf("argv: %s\n", *p);
      // for (p = env; *p; ++p) sim_printf("env: %s\n", *p);
    }

  cpu.features.reset_addr = bfd_get_start_address(sim_bfd);
  cpu.features.hwdiv = 1;
  cpu.features.hwmul = 1;
  cpu.features.hwmulx = 1;

  nios2_reset();
  // avm_dump_sections(1);

  return SIM_RC_OK;
}

int
sim_read (SIM_DESC sd, SIM_ADDR mem, unsigned char *buf, int length)
{
  // sim_printf("sim_read (mem=0x%x, len=0x%x)\n", mem, length);
  return avm_read(mem, buf, length, AVM_DATA | AVM_INSTRUCTION);
}

int
sim_write (SIM_DESC sd, SIM_ADDR mem, unsigned char *buf, int length)
{
  // sim_printf("sim_write (mem=0x%x, len=0x%x)\n", mem, length);
  if (sim_loading)
    return avm_add_memory("", mem, AVM_INSTRUCTION | AVM_DATA, buf, length);

  return avm_write(mem, buf, length, AVM_DATA);
}

int
sim_fetch_register (SIM_DESC sd, int regno, unsigned char *buf, int length)
{
  if (length != 4)
    return 4;

  if (regno >= SIM_NIOS2_R0_REGNUM && regno <= SIM_NIOS2_MPUACC_REGNUM)
    {
      *((unsigned32*) buf) = H2LE4(cpu.regs.array[regno]);
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
  // sim_printf("sim_resume(step=%d)\n", step);
  nios2_interpret(step);
}

int
sim_stop (SIM_DESC sd)
{
  cpu.state = sim_stopped;
  cpu.signal = TARGET_SIGNAL_INT;
}

void
sim_stop_reason (SIM_DESC sd, enum sim_stop *reason, int *sigrc)
{
  // sim_printf("sim_stop_reason() -> sim_%s, %d\n",
  //   "running\0polling\0exited\0\0stopped\0signalled" + (8 * cpu.state),
  //   cpu.signal);
  *reason = cpu.state;
  *sigrc = cpu.signal;
}

void
sim_do_command (SIM_DESC sd, char *cmd)
{
}

void
sim_set_callbacks (struct host_callback_struct *ptr)
{
  /* DEPRECATED */
  sim_printf("sim_set_callbacks()\n");
}

void
sim_size (int i)
{
  /* DEPRECATED */
  sim_printf("sim_size()\n");
}

int
sim_trace (SIM_DESC sd)
{
  /* DEPRECATED */
  sim_printf("sim_trace()\n");
  return 1;
}

int
sim_sys_write (int file, SIM_ADDR ptr, int len)
{
  int i;
  char buf;

  if (file != 1 /* stdout */ && file != 2 /* stderr */)
    return -1;

  for (i = len; i > 0; --i, ++ptr)
    {
      if (avm_read(ptr, &buf, 1, AVM_DATA) != 1)
        buf = 0xff;
      sim_printf("%c", buf);
    }

  return len;
}

int
sim_sys_read (int file, SIM_ADDR ptr, int len)
{
  if (file != 0 /* stdin */)
    return -1;
  /* TODO */
  return -1;
}

/*
 * vim:et sts=2 sw=2:
 */
