/**
 * @file sim-nios2.h
 * @brief Definitions for the interface between NiosII simulator and GDB
 * @author kimu_shu
 */

#ifndef SIM_NIOS2_H
#define SIM_NIOS2_H

#ifdef __cplusplus
extern "C" { // }
#endif

enum sim_nios2_regs
  {
    SIM_NIOS2_R0_REGNUM,  /* zero */
    SIM_NIOS2_R1_REGNUM,  /* at */
    SIM_NIOS2_R2_REGNUM,
    SIM_NIOS2_R3_REGNUM,
    SIM_NIOS2_R4_REGNUM,
    SIM_NIOS2_R5_REGNUM,
    SIM_NIOS2_R6_REGNUM,
    SIM_NIOS2_R7_REGNUM,
    SIM_NIOS2_R8_REGNUM,
    SIM_NIOS2_R9_REGNUM,
    SIM_NIOS2_R10_REGNUM,
    SIM_NIOS2_R11_REGNUM,
    SIM_NIOS2_R12_REGNUM,
    SIM_NIOS2_R13_REGNUM,
    SIM_NIOS2_R14_REGNUM,
    SIM_NIOS2_R15_REGNUM,
    SIM_NIOS2_R16_REGNUM,
    SIM_NIOS2_R17_REGNUM,
    SIM_NIOS2_R18_REGNUM,
    SIM_NIOS2_R19_REGNUM,
    SIM_NIOS2_R20_REGNUM,
    SIM_NIOS2_R21_REGNUM,
    SIM_NIOS2_R22_REGNUM,
    SIM_NIOS2_R23_REGNUM,
    SIM_NIOS2_R24_REGNUM, /* et */
    SIM_NIOS2_R25_REGNUM, /* bt */
    SIM_NIOS2_R26_REGNUM, /* gp */
    SIM_NIOS2_R27_REGNUM, /* sp */
    SIM_NIOS2_R28_REGNUM, /* fp */
    SIM_NIOS2_R29_REGNUM, /* ea */
    SIM_NIOS2_R30_REGNUM, /* ba */
    SIM_NIOS2_R31_REGNUM, /* ra */
    SIM_NIOS2_PC_REGNUM,
    SIM_NIOS2_STATUS_REGNUM,
    SIM_NIOS2_ESTATUS_REGNUM,
    SIM_NIOS2_BSTATUS_REGNUM,
    SIM_NIOS2_IENABLE_REGNUM,
    SIM_NIOS2_IPENDING_REGNUM,
    SIM_NIOS2_CPUID_REGNUM,
    SIM_NIOS2_CTL6_REGNUM,
    SIM_NIOS2_EXCEPTION_REGNUM,
    SIM_NIOS2_PTEADDR_REGNUM,
    SIM_NIOS2_TLBACC_REGNUM,
    SIM_NIOS2_TLBMISC_REGNUM,
    SIM_NIOS2_CTL11_REGNUM,
    SIM_NIOS2_BADADDR_REGNUM,
    SIM_NIOS2_CONFIG_REGNUM,
    SIM_NIOS2_MPUBASE_REGNUM,
    SIM_NIOS2_MPUACC_REGNUM,
  };

typedef union
  {
    unsigned32 gpr[1];
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
  }
nios2_regs_t;

typedef struct
  {
    unsigned32 reset_addr;
    unsigned32 break_addr;
    unsigned32 exception_addr;
    unsigned32 cpuid;

    unsigned8 hwdiv;
    unsigned8 hwmul;
    unsigned8 hwmulx;
    unsigned8 mmu;
    unsigned8 mpu;
  }
nios2_features_t;

typedef struct
  {
    nios2_regs_t regs;
    nios2_features_t features;

    enum sim_stop state;
    int signal;
  }
nios2_cpu_t;

#define NIOS2_STATUS_PIE        (1u<<0)
#define NIOS2_STATUS_U          (1u<<1)
#define NIOS2_STATUS_EH         (1u<<2)
#define NIOS2_STATUS_IH         (1u<<3)
#define NIOS2_STATUS_IL_SHIFT   4
#define NIOS2_STATUS_CRS_SHIFT  10
#define NIOS2_STATUS_PRS_SHIFT  16
#define NIOS2_STATUS_MMI        (1u<<22)
#define NIOS2_STATUS_RSIE       (1u<<23)

#define AVM_INSTRUCTION   (1<<12)
#define AVM_DATA          (1<<13)
#define AVM_MASTER_MASK   (AVM_INSTRUCTION|AVM_DATA)

#define AVM_READONLY      (1<<16)
#define AVM_NOCACHE       (1<<17)


enum nios2_sys_magics
  {
    NIOS2_SYS_WRITE = 0x0001,
    NIOS2_SYS__EXIT = 0x0010,
    NIOS2_SYS_MAGIC = 0xcafe,
  };

extern int nios2_reset(void);
extern int nios2_interpret(int step);
extern nios2_cpu_t cpu;

extern int avm_add_memory(const char *name, SIM_ADDR base, int flags, unsigned char *buf, int length);
extern int avm_end_address(SIM_ADDR *pend, int flags);
extern void avm_clear_sections(void);
extern int avm_read(SIM_ADDR mem, unsigned char *buf, int length, int flags);
extern int avm_write(SIM_ADDR mem, unsigned char *buf, int length, int flags);
extern int avm_write_force(SIM_ADDR mem, unsigned char *buf, int length, int flags);

extern int sim_sys_write(int file, SIM_ADDR ptr, int len);
extern int sim_sys_read(int file, SIM_ADDR ptr, int len);
extern void sim_sys__exit(int exitcode);

#ifdef __cplusplus
}
#endif

#endif
