/**
 * @file system.c
 * @brief System emulation for NiosII simulator
 * @author kimu_shu
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include "libiberty.h"
#include "sim-main.h"
#include "sim-nios2.h"

#define READ_MODE         0
#define WRITE_MODE        1
#define FORCE_WRITE_MODE  2

struct mm_section_t {
  SIM_ADDR base;
  SIM_ADDR end;
  int flags;
  unsigned char *data;
  char name[1];
};

static struct mm_section_t **mm_sects;
static int mm_sect_count;

static struct mm_section_t *
find_mm_section (SIM_ADDR mem, int *pindex)
{
  int top, bottom;

  top = 0;
  bottom = mm_sect_count;
  while (top < bottom)
    {
      int i = (top + bottom) / 2;
      struct mm_section_t *s = mm_sects[i];

      if (mem < s->base)
        bottom = i;
      else if (mem >= s->end)
        top = i + 1;
      else
        {
          if (pindex)
            *pindex = i;
          return s;
        }
    }

  if (pindex)
    *pindex = top;

  return NULL;
}

static int
avm_access(SIM_ADDR mem, unsigned char *buf, int length, int flags, int mode)
{
  size_t offset;
  int result = 0;

  if (mem & 0x80000000u)
    flags |= AVM_NOCACHE;
  mem &= ~0x80000000u;

  while (length > 0)
    {
      struct mm_section_t *s;
      int sec_len;

      if ((flags & AVM_INSTRUCTION) && (mem & 3))
        break;  /* unaligned access */

      s = find_mm_section(mem, NULL);
      if (!s)
        break;  /* no such address */

      if (((s->flags & flags) & AVM_MASTER_MASK) == 0)
        break;  /* no connection to master */

      if ((s->flags & AVM_READONLY) && mode == WRITE_MODE)
        break;  /* write protected */

      sec_len = (int)(s->end - mem);
      if (sec_len > length)
        sec_len = length;

      offset = mem - s->base;

      /*
      sim_printf("avm_access base=0x%x, offset=0x%x, len=0x%x, %s\n", s->base, offset, sec_len,
        mode == READ_MODE ? "read" : "write");
      */

      if (mode == READ_MODE)
        memcpy(buf, s->data + offset, sec_len);
      else
        memcpy(s->data + offset, buf, sec_len);

      buf += sec_len;
      length -= sec_len;
      result += sec_len;
    }

  return result;
}

int
avm_add_memory(const char *name, SIM_ADDR base, int flags, unsigned char *buf, int length)
{
  SIM_ADDR end = base + length;
  int index;
  size_t namelen = strlen(name);
  size_t memlen = (length + 3) & ~3u;
  struct mm_section_t *s;

  if (length <= 0)
    return 0;

  if (find_mm_section(base, &index) != NULL)
    return 0; /* already exist */

  if (index < mm_sect_count && mm_sects[index]->base < end)
    return 0; /* overlaps into the next section */

  /* add new section pointer */
  ++mm_sect_count;
  mm_sects = (struct mm_section_t **) realloc(mm_sects, sizeof(*mm_sects) * mm_sect_count);
  memmove(mm_sects + index + 1, mm_sects + index,
    sizeof(*mm_sects) * (mm_sect_count - index - 1));

  /* build new section */
  s = (struct mm_section_t *) malloc(sizeof(*s) + namelen);
  mm_sects[index] = s;
  s->base = base;
  s->end = end;
  s->flags = flags;
  s->data = (unsigned char *) malloc(memlen);
  memset(s->data, 0xff, memlen);
  if (buf)
    memcpy(s->data, buf, length);

  strcpy(s->name, name);
  return length;
}

int
avm_dump_sections(int verbose)
{
  struct mm_section_t **p;
  int i, j, len;

  sim_printf("mm_sect_count: %d\n", mm_sect_count);
  for (i = 0, p = mm_sects; i < mm_sect_count; ++i, ++p)
    {
      sim_printf("mm_sects[%d]: {range: 0x%08x-0x%08x, flags: 0x%08x, name:\"%s\" }",
        i, (*p)->base, (*p)->end-1, (*p)->flags, (*p)->name);

      if (verbose == 0)
        len = 0;
      else
        len = (*p)->end - (*p)->base;

      for (j = 0; j < len; j += 4)
        {
          if ((j & 15) == 0)
            sim_printf("\n# %08x:", j + (*p)->base);
          sim_printf(" %08x", *(unsigned32*)((*p)->data + j));
        }

      sim_printf("\n");
    }
}

void
avm_clear_sections(void)
{
  struct mm_section_t **p;
  int i;
  for (i = 0, p = mm_sects; i < mm_sect_count; ++i, ++p)
    {
      struct mm_section_t *s = *p;
      free(s->data);
      free(s);
    }

  free(mm_sects);
  mm_sects = NULL;
  mm_sect_count = 0;
}

int
avm_read(SIM_ADDR mem, unsigned char *buf, int length, int flags)
{
  return avm_access(mem, buf, length, flags, READ_MODE);
}

int
avm_write(SIM_ADDR mem, unsigned char *buf, int length, int flags)
{
  return avm_access(mem, buf, length, flags, WRITE_MODE);
}

int
avm_write_force(SIM_ADDR mem, unsigned char *buf, int length, int flags)
{
  return avm_access(mem, buf, length, flags, FORCE_WRITE_MODE);
}

/*
 * vim:et sts=2 sw=2:
 */
