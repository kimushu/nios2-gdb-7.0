/**
 * @file sim-main.h
 * @brief NiosII simulator definitions
 * @author kimu_shu
 */

#ifndef SIM_MAIN_H
#define SIM_MAIN_H

#include "config.h"

// #include "sim-basics.h"
// #include "sim-signal.h"

#include "gdb/callback.h"
#include "gdb/remote-sim.h"
#include "sim-config.h"
#include "sim-types.h"
#include "sim-utils.h"

#ifdef WORDS_BIGENDIAN
#else
#define H2LE4(x)	(x)
#define LE2H4(x)	(x)
#endif

extern void sim_printf(const char *fmt, ...);

#endif	/* SIM_MAIN_H */
