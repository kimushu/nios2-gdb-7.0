Readme for nios2-gdb-7.0 with simulator
---------------

This GDB is based on Altera's Nios II GDB source (http://www.altera.com/support/ip/processors/nios2/ide/ips-nios2-ide-tutorial.html)

##How to build

1. "./configure --target=nios2-elf"

2.  "make"

##How to use

###Build as standalone program (not for FPGA)

1. Compile program with hook

```
    nios2-elf-gcc -o test test.c _repository\_root_/sim/nios2/hook\_sim\_nios2.c
```

2. Run simulator

```
    _repository\_root_/gdb/gdb ./test
    (gdb) target sim
    (gdb) load
    (gdb) run
```

###Build as FPGA embedded program (by Nios II SBT)

> coming soon...

##Future plans
1. Load .sopcinfo and emulate some memory/timer devices.

2. Forwarding stdin/stdout/stderr to socket (telnet port).

