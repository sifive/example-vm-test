// See LICENSE for license details.

#include "riscv_user.h"

	.globl user_entry_point

user_entry_point:

//	j user_entry_point		// use for initial testing

//-------------------------------------------------------------
// Addition tests - Arithmetic
//-------------------------------------------------------------

    TEST_RR_OP( 1002,  add, 0x00000000, 0x00000000, 0x00000000 );
    TEST_RR_OP( 1003,  add, 0x00000002, 0x00000001, 0x00000001 );
    TEST_RR_OP( 1004,  add, 0x0000000a, 0x00000003, 0x00000007 );

    TEST_RR_OP( 1005,  add, 0xffffffffffff8000, 0x0000000000000000, 0xffffffffffff8000 );
    TEST_RR_OP( 1006,  add, 0xffffffff80000000, 0xffffffff80000000, 0x00000000 );
    TEST_RR_OP( 1007,  add, 0xffffffff7fff8000, 0xffffffff80000000, 0xffffffffffff8000 );

    TEST_RR_OP( 1008,  add, 0x0000000000007fff, 0x0000000000000000, 0x0000000000007fff );
    TEST_RR_OP( 1009,  add, 0x000000007fffffff, 0x000000007fffffff, 0x0000000000000000 );
    TEST_RR_OP( 1010, add, 0x0000000080007ffe, 0x000000007fffffff, 0x0000000000007fff );

    TEST_RR_OP( 1011, add, 0xffffffff80007fff, 0xffffffff80000000, 0x0000000000007fff );
    TEST_RR_OP( 1012, add, 0x000000007fff7fff, 0x000000007fffffff, 0xffffffffffff8000 );

    TEST_RR_OP( 1013, add, 0xffffffffffffffff, 0x0000000000000000, 0xffffffffffffffff );
    TEST_RR_OP( 1014, add, 0x0000000000000000, 0xffffffffffffffff, 0x0000000000000001 );
    TEST_RR_OP( 1015, add, 0xfffffffffffffffe, 0xffffffffffffffff, 0xffffffffffffffff );

    TEST_RR_OP( 10016, add, 0x0000000080000000, 0x0000000000000001, 0x000000007fffffff );

//-------------------------------------------------------------
// Multiply tests - Arithmetic
//-------------------------------------------------------------

#if __riscv_xlen == 64

    TEST_RR_OP( 2002,  mul, 0x0000000000001200, 0x0000000000007e00, 0x6db6db6db6db6db7 );
    TEST_RR_OP( 2003,  mul, 0x0000000000001240, 0x0000000000007fc0, 0x6db6db6db6db6db7 );

    TEST_RR_OP( 2004,  mul, 0x00000000, 0x00000000, 0x00000000 );
    TEST_RR_OP( 2005,  mul, 0x00000001, 0x00000001, 0x00000001 );
    TEST_RR_OP( 2006,  mul, 0x00000015, 0x00000003, 0x00000007 );

    TEST_RR_OP( 2007,  mul, 0x0000000000000000, 0x0000000000000000, 0xffffffffffff8000 );
    TEST_RR_OP( 2008,  mul, 0x0000000000000000, 0xffffffff80000000, 0x00000000 );
    TEST_RR_OP( 2009,  mul, 0x0000400000000000, 0xffffffff80000000, 0xffffffffffff8000 );

    TEST_RR_OP( 2010,  mul, 0x000000000000ff7f, 0xaaaaaaaaaaaaaaab, 0x000000000002fe7d );
    TEST_RR_OP( 2011,  mul, 0x000000000000ff7f, 0x000000000002fe7d, 0xaaaaaaaaaaaaaaab );

#else

    TEST_RR_OP( 2002,  mul, 0x00001200, 0x00007e00, 0xb6db6db7 );
    TEST_RR_OP( 2003,  mul, 0x00001240, 0x00007fc0, 0xb6db6db7 );

    TEST_RR_OP( 2004,  mul, 0x00000000, 0x00000000, 0x00000000 );
    TEST_RR_OP( 2005,  mul, 0x00000001, 0x00000001, 0x00000001 );
    TEST_RR_OP( 2006,  mul, 0x00000015, 0x00000003, 0x00000007 );

    TEST_RR_OP( 2007,  mul, 0x00000000, 0x00000000, 0xffff8000 );
    TEST_RR_OP( 2008,  mul, 0x00000000, 0x80000000, 0x00000000 );
    TEST_RR_OP( 2009,  mul, 0x00000000, 0x80000000, 0xffff8000 );

    TEST_RR_OP(2010,  mul, 0x0000ff7f, 0xaaaaaaab, 0x0002fe7d );
    TEST_RR_OP(2011,  mul, 0x0000ff7f, 0x0002fe7d, 0xaaaaaaab );

    TEST_RR_OP(2012,  mul, 0x00000000, 0xff000000, 0xff000000 );

    TEST_RR_OP(2013,  mul, 0x00000001, 0xffffffff, 0xffffffff );
    TEST_RR_OP(2014,  mul, 0xffffffff, 0xffffffff, 0x00000001 );
    TEST_RR_OP(2015,  mul, 0xffffffff, 0x00000001, 0xffffffff );

#endif

//-------------------------------------------------------------
// Exclusive OR tests - Logical tests
//-------------------------------------------------------------

    TEST_RR_OP( 4002, xor, 0xf00ff00f, 0xff00ff00, 0x0f0f0f0f );
    TEST_RR_OP( 4003, xor, 0xff00ff00, 0x0ff00ff0, 0xf0f0f0f0 );
    TEST_RR_OP( 4004, xor, 0x0ff00ff0, 0x00ff00ff, 0x0f0f0f0f );
    TEST_RR_OP( 4005, xor, 0x00ff00ff, 0xf00ff00f, 0xf0f0f0f0 );

//-------------------------------------------------------------
// Divide Instruction - Arithmetic tests
//-------------------------------------------------------------

    TEST_RR_OP( 5002, div,  3,  20,   6 );
    TEST_RR_OP( 5003, div, -3, -20,   6 );
    TEST_RR_OP( 5004, div, -3,  20,  -6 );
    TEST_RR_OP( 5005, div,  3, -20,  -6 );

    TEST_RR_OP( 5006, div, -1<<31, -1<<31,  1 );
    TEST_RR_OP( 5007, div, -1<<31, 1<<31, -1 );

    TEST_RR_OP( 5008, div, -1, -1<<31, 0 );
    TEST_RR_OP( 5009, div, -1,      1, 0 );
    TEST_RR_OP( 5010, div, -1,      0, 0 );

done:

	/* run forever or until we get an exception trap */
	j user_entry_point

//ASM_EXIT
pass:
    fence;
    li ASM_RETURN_REG, PASS;
    j asm_exit;
fail:
    fence;
    li ASM_RETURN_REG, FAIL;
asm_exit:
    //RESTORE_REGS
    call exit;
    mv ra, s0;
    ret;

