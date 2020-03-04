# example-vm-test

Virtual Memory Test for U-series cores with MMU. This is a work in progress that integrates some aspecs of freedom-metal in order to be later integrated as a software example into freedom-e-sdk.  

This is not a comprehensive virtual memory test.  This test simply sets up
translation on a single CPU (hart) which enters User mode and runs some
low level assembly tests.

All other harts in the system execute in machine mode, and run a coherency
torture routine that runs writes and reads on valid physical memory locations.
There are no guarantees associated with this test and no formal support.

This test is targeted to run on SiFive U-series cores which contain an MMU.  
For example, a U74MC design contains 5 harts (CPUs).  

hart 0 - S76 - No Supervisor mode and no MMU.  Not part of VM test
hart 1 - U74 with MMU.  This hart sets up translation
hart 2 - U74 with MMU.  This hart run coherency_torture()
hart 3 - U74 with MMU.  This hart run coherency_torture()
hart 4 - U74 with MMU.  This hart run coherency_torture()

The coherency_torture() routine is a simple routine that causes coherence misses
without affecting program semantics.  This is accomplished by running instructions 
that use the x0 register which is hardwired to 0x0.  Custom functionality may 
be added here for additional coverage.  

This test originated in the riscv-test-env github repo and was modified to 
be used with SiFive freedom-metal startup code.  The original implementation
was intended to be uses with the riscv-tests suite.  

riscv-test-env github repo:
https://github.com/riscv/riscv-test-env/tree/master/v

riscv-tests repo:
https://github.com/riscv/riscv-tests

SiFive freedom-metal repo:
https://github.com/sifive/freedom-metal

The freedom-metal API provides a multi-core synchronization method using 
__metal_synchronize_harts() and secondary_main() functions.
We leverage the secondary_main() function, which is initially defined using
a weak reference attribute, so a C function will override the weak 
reference and a custom secondary_main() is implemented to use as a 
synchronization method for all 5 harts in the system.  

The linker file defines __metal_boot_hart which controls which 
hart runs initialization code in freedom-metal/gloss/crt0.S.  
Usually we define __metal_boot_hart = 1, so any example that is 
compiled will use the proper -march and -mabi arguments to compile 
examples for the larger cores in the system.  These parameters exist
in settings.mk, which resides in freedom-e-sdk/bsp.  

If the hart running the user code encounteres an unexpected exception due to
a page related fault, it will wind up in handle_trap() handler.  If an exception
occurs that is not delegate to Supervisor mode, then the hart will wind up
in the trap_vector() routine.  

## Build Info

This was initially tested on the HiFive Unleashed board which contains the FU540 
SoC containing 5 harts (1x E51 + 4x U54).  The details below describe information
to run this test on the HiFive Unleashed board.

Freedom Studio IDE was used to compile & debug, available on SiFive.com.
All dip switches towards edge of board (OFF). 
This dip switch setting prevents booting from SD card and CPU will wait
for debugger to connect.
Micro USB cable provides debug connection, and no external debug pod is needed.
The micro USB does not supply power to the board.  Power is supplied through
the 12V power connection, and the red button is required to be pushed in.

To compile this code, there are two methods:
1.  Command line.  First, the SiFive GCC toolchain will need to be added to your path.
Then, add a new software project under freedom-e-sdk/software.
For example, create virtual-mem-test folder and add source code there.  To compile:
"make PROGRAM=vm-test TARGET=design-rtl CONFIGURATION=release LINK_TARGET=scratchpad software"

2.  Use Freedom Studio bundle with integrated toolchain.
Copy source code to the freedom-e-sdk/software path in the toolchain 
Go to SiFive Tools -> Create a New Freedom E SDK Software Project.  Point 
the wizard to the freedom-e-sdk path in your tarball.  Make sure to modify
the Makefile to use LINK_TARGET=scratchpad in the project.

## For hardware debug:
A custom openocd.cfg file is required within Freedom Studio enable SMP session, and
a specific configuration within Freedom Studio to enable a true SMP multi-core debug
session.  Contact SiFive support for more information on this configuration.  

For Lauterbach Trace32, a RISC-V license is required.  To get started, 
type 'welcome.scripts' on the command line.  


