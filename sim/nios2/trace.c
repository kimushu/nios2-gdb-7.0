/**
 * @file trace.c
 * @brief NiosII instruction trace
 * @author kimu_shu
 */

#include "config.h"
#include <string.h>
#include <malloc.h>
#include "gdb/callback.h"
#include "gdb/signals.h"
#include "libiberty.h"
#include "gdb/remote-sim.h"
#include "sim-config.h"
#include "sim-types.h"
#include "sim-utils.h"
#include "sim-nios2.h"
#include "sim-main.h"

static int btrace_init(enum nios2_btrace_mode mode, unsigned32 size);
static int btrace_show(unsigned32 limit);

static int btrace_init(enum nios2_btrace_mode mode, unsigned32 size)
{
  unsigned32 bytes;

  btrace_free();
  cpu.btrace.mode = (size == 0) ? NIOS2_BTRACE_DISABLED : mode;
  if (mode == NIOS2_BTRACE_DISABLED)
    return 0;

  cpu.btrace.index = 0;
  cpu.btrace.size = size;
  bytes = size * sizeof(nios2_btrace_entry_t);
  cpu.btrace.buffer = (nios2_btrace_entry_t*) malloc(bytes);

  if (cpu.btrace.buffer == NULL)
    {
      sim_printf("no enough memory for trace buffer\n");
      return 1;
    }

  memset(cpu.btrace.buffer, 0xff, bytes);
  return 0;
}

static int btrace_show(unsigned32 limit)
{
  if (cpu.btrace.mode == NIOS2_BTRACE_DISABLED || cpu.btrace.buffer == NULL)
    {
      sim_printf("no trace data\n");
      return 0;
    }

  if (cpu.btrace.mode == NIOS2_BTRACE_LINEAR && limit > cpu.btrace.index)
    limit = cpu.btrace.index;
  else if (limit > cpu.btrace.size)
    limit = cpu.btrace.size;

  unsigned index = cpu.btrace.index + cpu.btrace.size - limit;

  sim_printf(" Level   From       To\n");
  for (; limit > 0; --limit, ++index)
    {
      index %= cpu.btrace.size;
      sim_printf("-%05d 0x%08x 0x%08x\n", limit,
        cpu.btrace.buffer[index].from, cpu.btrace.buffer[index].to);
    }

  return 0;
}

void btrace_record(unsigned32 from, unsigned32 to)
{
  if (cpu.btrace.mode == NIOS2_BTRACE_DISABLED ||
      cpu.btrace.index >= cpu.btrace.size)
    return;

  unsigned32 i = cpu.btrace.index++;
  cpu.btrace.buffer[i].from = from;
  cpu.btrace.buffer[i].to = to;

  if (cpu.btrace.mode == NIOS2_BTRACE_RING &&
      cpu.btrace.index >= cpu.btrace.size)
    cpu.btrace.index = 0;
}

int btrace_free(void)
{
  free(cpu.btrace.buffer);
  cpu.btrace.buffer = NULL;
  return 0;
}

int btrace_command(int argc, char *argv[])
{
  if (argc <= 1)
    {
      sim_printf(
      "List of branch-trace commands:\n\n"
      "init -- Initialize and enable branch trace\n"
      "show -- Show trace data\n"
      );
      return;
    }

  int i = 1;

  if (strcmp(argv[i], "init") == 0)
    {
      enum nios2_btrace_mode mode = NIOS2_BTRACE_RING;
      unsigned32 size = 1024;
      ++i;

      if (i < argc && strcmp(argv[i], "ring") == 0)
        {
          mode = NIOS2_BTRACE_RING;
          ++i;
        }
      else if (i < argc && strcmp(argv[i], "linear") == 0)
        {
          mode = NIOS2_BTRACE_LINEAR;
          ++i;
        }

      if (i < argc)
        size = strtoul(argv[i++], NULL, 0);

      if (i == argc)
        {
          btrace_init(mode, size);
          return 0;
        }
    }
  else if (strcmp(argv[1], "show") == 0)
    {
      unsigned32 limit = -1;
      ++i;

      if (i < argc)
        limit = strtoul(argv[i++], NULL, 0);

      if (i == argc)
        {
          btrace_show(limit);
          return 0;
        }
    }
  else
    return 1;

  sim_printf("too many options for btrace %s: %s\n", argv[1], argv[i]);
  return 0;
}


/*
 * vim:et sts=2 sw=2:
 */
