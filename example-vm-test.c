/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdlib.h>
#include <stdio.h>

#include <metal/cpu.h>
#include <metal/pmp.h>
#include <metal/privilege.h>

#define ECODE_ILLEGAL_INSTRUCTION	2

// ------------ multi-core support --------------------------------------------
#include <metal/lock.h>
METAL_LOCK_DECLARE(my_lock);

int main(void);

/* This flag tells the secondary harts when to start to make sure
 * that they wait until the lock is initialized */
volatile int _start_other = 0;
uintptr_t _begin_torture_flag;

/* This is a count of the number of harts who have executed their main function */
volatile int checkin_count = 0;
// ------------------------------------------------------------------------------

// ----- virtual memory test support --------------------------------------------
#include <metal/machine.h>
#include <metal/machine/platform.h>
#include <metal/machine/inline.h>

#include "riscv_user.h"
#include "riscv_vm.h"
#include "encoding.h"

extern uintptr_t _heap_end;

int machine_program_entry(int hartid);
static void coherence_torture();
static uint64_t lfsr63(uint64_t x)
{
  uint64_t bit = (x ^ (x >> 1)) & 1;
  return (x >> 1) | (bit << 62);
}
#define ENTROPY  (uint64_t)( ((read_csr(mcycle) ^ 0xa879) << 31) | ((read_csr(mcycle) << 16) | 0xF492))
// ------------------------------------------------------------------------------

/* bring in linker symbol that defines which hart is the bootup hart */
extern int __metal_boot_hart;

/* bring in user entry point and trap vector */
//extern uintptr_t userstart;         // riscv-test-env orig - uses tests from riscv-tests
extern uintptr_t user_entry_point_spin;     // custom to this implementation
extern uintptr_t user_entry_point;          // runs some asm tests
extern uintptr_t trap_vector;

/* vm setup function */
extern void vm_boot(uintptr_t test_addr);

/* Create a stack for user-mode execution */
uint8_t my_stack[768] __attribute__((aligned(16)));

/* Create the register file for user mode execution */
struct metal_register_file my_regfile = {
	.sp = (uintptr_t) (my_stack + sizeof(my_stack)),
};

void illegal_instruction_fault_handler(struct metal_cpu *cpu, int ecode)
{
	if(ecode == ECODE_ILLEGAL_INSTRUCTION) {
		printf("Caught illegal instruction in User mode\n");
		exit(0);
	} else {
		exit(7);
	}
}


/* For simple user mode entry without address translation enabled
 */
void user_mode_entry_point_no_translation()
{
    int i = 0;

    while (1) {
        __asm__("NOP");
        i++;
    }

#if 0 // original user-mode test did something simple to test privilege mode
	/* Attempt to read from a machine-mode CSR */
	int misa;
	__asm__ volatile("csrr %0, misa" : "=r" (misa));

	/* If we didn't trap, fail the test */
	exit(8);
#endif

}


/* !!!! NOTE: Only the boot hart runs main().
 * All others go to machine_program_entry().
 * Here we setup PMP, and then setup translation and
 * enter a lower privilege mode using vm_boot().
 */
int main()
{
	int rc, sp;
	struct metal_cpu *cpu;
	struct metal_interrupt *cpu_intr;
	struct metal_pmp *pmp;

	/* Sync point for boot hart */
    int num_harts = metal_cpu_get_num_harts();

    /* Acquire lock so only one CPU has access until lock is released */
    metal_lock_take(&my_lock);

    puts("Boot Hart Entered Main!\n");
    fflush(stdout);

    /* Increment global that tracks how many harts have arrived */
    checkin_count += 1;

    /* Release lock */
    metal_lock_give(&my_lock);

    /* Wait till other harts are ready */
    while(checkin_count != num_harts) ;

	/* Initialize PMPs */
	pmp = metal_pmp_get_device();
	if(!pmp) {
		printf("Unable to get PMP Device\n");
		return 4;
	}
	metal_pmp_init(pmp);

	/* Configure PMP 0 to allow access to all memory in Machine mode,
	 * but protecting it from lower privilege modes */
	struct metal_pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_TOR,
		.X = 1,
		.W = 1,
		.R = 1,
	};
	rc = metal_pmp_set_region(pmp, 0, config, -1);
	if(rc != 0) {
		return 5;
	}

	write_csr(mtvec, (uintptr_t)&trap_vector);
	__asm__("csrw mscratch, sp");

	//vm_boot ((uintptr_t)&user_entry_point_spin);    // just spin to self
	vm_boot ((uintptr_t)&user_entry_point);         // run some tests

	while (1); // should not return here

}

/* This is the C entry point for all harts in the system, called from crt0.S.
 * This function can be modified to redirect each hart in the system as required.
 * This function is originally defined with a weak reference in the
 * assembly startup file freedom-metal/gloss/crt0.S, but overridden here.
 */
int secondary_main(void) {

    int hartid = metal_cpu_get_current_hartid();
    int boot_hart = (int)&__metal_boot_hart;

    if(hartid == boot_hart) {

        int rc = metal_lock_init(&my_lock);
        if(rc != 0) {
            puts("Failed to initialize my_lock\n");
            exit(1);
        }

        /* Ensure that the lock is initialized before any readers of
         * _start_other */
        __asm__ ("fence rw,w"); /* Release semantics */

        /* allow other harts to continue in machine_program_entry() */
        _start_other = 1;

        /* main() is intended for booth hart - setup translation and enter User mode */
        return main();
    }
    else {

        /* All other harts go here */
        return machine_program_entry(hartid);
    }
}

/* Harts that stay in machine mode without translation go here */
int machine_program_entry(int hartid) {

    int delay_flag = 0x800;

    /* all harts spin here except boot hart */
    while(!_start_other);

    /* Acquire lock for print and incrementing global variable */
    metal_lock_take(&my_lock);
    puts("Other Hart\n");
    fflush(stdout);
    checkin_count += 1;
    metal_lock_give(&my_lock);

    /* rest of program */
    if (hartid > 0) {
        /* don't run until virtual memory is setup */
        while (!_begin_torture_flag);

        /* When flag is set, we resume */
        coherence_torture();
    }
    else {
        /* hart0 does not contain MMU, so it does not participate */
        while(1) {

            /* Tells all other harts to go have fun */
            _begin_torture_flag = 1;
            asm("fence");
            while(1);
        }
    }
}

static void coherence_torture()
{
  // cause coherence misses without affecting program semantics
  uint64_t random = ENTROPY;
  uint64_t max_size = 0;

  while (1)
  {
    uintptr_t paddr = DRAM_BASE + ((random % (2 * (MAX_TEST_PAGES + 1) * PGSIZE)) & -4);  // 512K max size (0x7FFFC)
    uintptr_t rand_max = 0, max_addr = (uintptr_t)&_heap_end;
    //uintptr_t paddr = DRAM_BASE + ((random % (max_addr)) & -4);     // keep physical address within range of application

    /* Track the max range that comes up using original formula */
    max_size = (paddr > max_size) ? paddr : max_size;   /* capture real range as test */
    rand_max = (rand_max > random) ? rand_max : random; /* track our random number max for visibility */

    /* Only access address range within our application, so create a new random location */
    paddr = (paddr >= max_addr) ? (DRAM_BASE + (random & (0xFFC << (random & 0x3)))) : paddr;

#ifdef __riscv_atomic
    if (random & 1) // perform a no-op write
      asm volatile ("amoadd.w zero, zero, (%0)" :: "r"(paddr));
    else
#endif
    // perform a read
    asm volatile ("lw zero, (%0)" :: "r"(paddr));
    random = lfsr63(random);
  }
}
