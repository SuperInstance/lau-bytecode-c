#include "../include/lau_bytecode.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int failed = 0;
static int test_count = 0;

#define TEST(name) do { \
    test_count++; \
    printf("  Test %d: %s ... ", test_count, name); \
    fflush(stdout); \
} while(0)

#define PASS() do { printf("PASS\n"); } while(0)
#define FAIL(msg) do { \
    printf("FAIL: %s\n", msg); \
    failed++; \
} while(0)
#define ASSERT(cond, msg) do { \
    if (!(cond)) { FAIL(msg); return; } \
} while(0)
#define ASSERT_DOUBLE_EQ(a, b, msg) do { \
    if (fabs((a) - (b)) > 1e-9) { \
        printf("FAIL: %s (got %f, expected %f)\n", msg, (a), (b)); \
        failed++; \
        return; \
    } \
} while(0)

static void test_push_and_add(void) {
    TEST("Push and add");
    LauInstruction code[] = {
        {OP_PUSH, 10.0, 0},
        {OP_PUSH, 20.0, 0},
        {OP_ADD, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    int r = lau_vm_run(&vm, code, 4, 100);
    ASSERT(r == 1, "should halt");
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 30.0, "10+20 should be 30");
    PASS();
}

static void test_push_and_sub(void) {
    TEST("Push and subtract");
    LauInstruction code[] = {
        {OP_PUSH, 50.0, 0},
        {OP_PUSH, 13.0, 0},
        {OP_SUB, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 4, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 37.0, "50-13 should be 37");
    PASS();
}

static void test_multiply(void) {
    TEST("Multiply");
    LauInstruction code[] = {
        {OP_PUSH, 7.0, 0},
        {OP_PUSH, 6.0, 0},
        {OP_MUL, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 4, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 42.0, "7*6 should be 42");
    PASS();
}

static void test_division_and_div_by_zero(void) {
    TEST("Division (and div-by-zero error)");
    /* Normal division */
    {
        LauInstruction code[] = {
            {OP_PUSH, 10.0, 0},
            {OP_PUSH, 4.0, 0},
            {OP_DIV, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        lau_vm_run(&vm, code, 4, 100);
        ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 2.5, "10/4 should be 2.5");
    }
    /* Division by zero */
    {
        LauInstruction code[] = {
            {OP_PUSH, 10.0, 0},
            {OP_PUSH, 0.0, 0},
            {OP_DIV, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        int r = lau_vm_run(&vm, code, 4, 100);
        ASSERT(r == -1, "div by zero should return -1");
    }
    PASS();
}

static void test_negate(void) {
    TEST("Negate");
    LauInstruction code[] = {
        {OP_PUSH, 42.0, 0},
        {OP_NEG, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 3, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), -42.0, "negate 42 should be -42");
    PASS();
}

static void test_store_and_load_local(void) {
    TEST("Store and load local");
    LauInstruction code[] = {
        {OP_PUSH, 99.0, 0},
        {OP_STORE, 0, 3},         /* store in local[3] */
        {OP_LOAD, 0, 3},          /* load from local[3] */
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 4, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 99.0, "stored and loaded 99");
    PASS();
}

static void test_dup(void) {
    TEST("Dup opcode");
    LauInstruction code[] = {
        {OP_PUSH, 7.0, 0},
        {OP_DUP, 0, 0},
        {OP_ADD, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 4, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 14.0, "dup+add 7 should be 14");
    PASS();
}

static void test_swap(void) {
    TEST("Swap opcode");
    /* Stack: [10,20]. Swap -> [20,10]. Sub -> 20-10=10 */
    LauInstruction code[] = {
        {OP_PUSH, 10.0, 0},
        {OP_PUSH, 20.0, 0},
        {OP_SWAP, 0, 0},
        {OP_SUB, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 5, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 10.0, "swap+sub should be 10");
    PASS();
}

static void test_comparison_lt_gt_eq(void) {
    TEST("Comparison: LT, GT, EQ");
    /* LT: 3 < 5 => true */
    {
        LauInstruction code[] = {
            {OP_PUSH, 3.0, 0},
            {OP_PUSH, 5.0, 0},
            {OP_LT, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        lau_vm_run(&vm, code, 4, 100);
        ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 1.0, "3<5 should be 1");
    }
    /* GT: 10 > 3 => true */
    {
        LauInstruction code[] = {
            {OP_PUSH, 10.0, 0},
            {OP_PUSH, 3.0, 0},
            {OP_GT, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        lau_vm_run(&vm, code, 4, 100);
        ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 1.0, "10>3 should be 1");
    }
    /* EQ: 5 == 5 => true */
    {
        LauInstruction code[] = {
            {OP_PUSH, 5.0, 0},
            {OP_PUSH, 5.0, 0},
            {OP_EQ, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        lau_vm_run(&vm, code, 4, 100);
        ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 1.0, "5==5 should be 1");
    }
    /* EQ: 5 == 4 => false */
    {
        LauInstruction code[] = {
            {OP_PUSH, 5.0, 0},
            {OP_PUSH, 4.0, 0},
            {OP_EQ, 0, 0},
            {OP_HALT, 0, 0}
        };
        LauVm vm = lau_vm_new();
        lau_vm_run(&vm, code, 4, 100);
        ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 0.0, "5==4 should be 0");
    }
    PASS();
}

static void test_conditional_jump_jz(void) {
    TEST("Conditional jump JZ");
    /* 0=PUSH0, 1=JZ(op=3), 2=HALT(skip), 3=PUSH42, 4=HALT */
    LauInstruction code[] = {
        {OP_PUSH, 0.0, 0},
        {OP_JZ, 0, 3},
        {OP_HALT, 0, 0},
        {OP_PUSH, 42.0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 5, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 42.0, "JZ should jump to push 42");
    PASS();
}

static void test_conditional_jump_jnz(void) {
    TEST("Conditional jump JNZ");
    /* 0=PUSH1, 1=JNZ(op=3), 2=HALT(skip), 3=PUSH42, 4=HALT */
    LauInstruction code[] = {
        {OP_PUSH, 1.0, 0},
        {OP_JNZ, 0, 3},
        {OP_HALT, 0, 0},
        {OP_PUSH, 42.0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 5, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 42.0, "JNZ should jump to push 42");
    PASS();
}

static void test_unconditional_jump(void) {
    TEST("Unconditional jump (loop)");
    /* Sum 1..5 = 15 using locals[0]=counter, [1]=sum */
    /* Sum 1..5 = 15. locals[0]=counter, [1]=sum.
       After loop, push sum onto stack and HALT.
       0=PUSH1,1=STORE0,2=PUSH0,3=STORE1
       4=LOAD0,5=PUSH5,6=GT,7=JNZ(op=17)
       8=LOAD1,9=LOAD0,10=ADD,11=STORE1
       12=LOAD0,13=PUSH1,14=ADD,15=STORE0,16=JMP(op=4)
       17=LOAD1,18=HALT */
    int code_len = 19;
    LauInstruction code[] = {
        {OP_PUSH, 1.0, 0},
        {OP_STORE, 0, 0},
        {OP_PUSH, 0.0, 0},
        {OP_STORE, 0, 1},
        {OP_LOAD, 0, 0},
        {OP_PUSH, 5.0, 0},
        {OP_GT, 0, 0},
        {OP_JNZ, 0, 17},
        {OP_LOAD, 0, 1},
        {OP_LOAD, 0, 0},
        {OP_ADD, 0, 0},
        {OP_STORE, 0, 1},
        {OP_LOAD, 0, 0},
        {OP_PUSH, 1.0, 0},
        {OP_ADD, 0, 0},
        {OP_STORE, 0, 0},
        {OP_JMP, 0, 4},
        {OP_LOAD, 0, 1},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, code_len, 1000);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 15.0, "sum 1..5 should be 15");
    PASS();
}

static void test_fibonacci(void) {
    TEST("Fibonacci computation (loop-based)");
    /* Compute fib(10) = 55.
       locals[0]=n, [1]=a(fib[0]=0), [2]=b(fib[1]=1), [3]=temp.
       Iterate n times shifting a=b, b=a+b. */
    /* 0=PUSH10,1=STORE0,2=PUSH0,3=STORE1,4=PUSH1,5=STORE2 */
    /* 6=LOAD0,7=PUSH0,8=GT,9=JZ(op=23) */
    /* 10=LOAD1,11=LOAD2,12=ADD,13=STORE3 */
    /* 14=LOAD2,15=STORE1,16=LOAD3,17=STORE2 */
    /* 18=LOAD0,19=PUSH1,20=SUB,21=STORE0,22=JMP(op=6) */
    /* 23=LOAD1,24=HALT */
    int code_len = 25;
    LauInstruction code[] = {
        {OP_PUSH, 10.0, 0},       /* 0 */
        {OP_STORE, 0, 0},
        {OP_PUSH, 0.0, 0},        /* 2 */
        {OP_STORE, 0, 1},
        {OP_PUSH, 1.0, 0},        /* 4 */
        {OP_STORE, 0, 2},
        {OP_LOAD, 0, 0},          /* 6 */
        {OP_PUSH, 0.0, 0},
        {OP_GT, 0, 0},            /* 8 */
        {OP_JZ, 0, 23},           /* 9 */
        {OP_LOAD, 0, 1},          /* 10 */
        {OP_LOAD, 0, 2},
        {OP_ADD, 0, 0},           /* 12 */
        {OP_STORE, 0, 3},
        {OP_LOAD, 0, 2},          /* 14 */
        {OP_STORE, 0, 1},
        {OP_LOAD, 0, 3},          /* 16 */
        {OP_STORE, 0, 2},
        {OP_LOAD, 0, 0},          /* 18 */
        {OP_PUSH, 1.0, 0},
        {OP_SUB, 0, 0},           /* 20 */
        {OP_STORE, 0, 0},
        {OP_JMP, 0, 6},           /* 22 */
        {OP_LOAD, 0, 1},          /* 23 */
        {OP_HALT, 0, 0}           /* 24 */
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, code_len, 1000);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 55.0, "fib(10) should be 55");
    PASS();
}

static void test_call_ret(void) {
    TEST("Call/Ret (subroutine)");
    /* Subroutine at index 3 doubles the top of stack */
    /* 0=PUSH13, 1=CALL(op=3), 2=HALT, 3=DUP, 4=ADD, 5=RET */
    int code_len = 6;
    LauInstruction code[] = {
        {OP_PUSH, 13.0, 0},
        {OP_CALL, 0, 3},
        {OP_HALT, 0, 0},
        {OP_DUP, 0, 0},
        {OP_ADD, 0, 0},
        {OP_RET, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, code_len, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 26.0, "double 13 should be 26");
    PASS();
}

static void test_vibe_get_set(void) {
    TEST("VibeGet and VibeSet");
    LauInstruction code[] = {
        {OP_PUSH, 77.0, 0},
        {OP_VIBE_SET, 0, 0},
        {OP_VIBE_GET, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 4, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 77.0, "vibe should be 77");
    PASS();
}

static void test_sense_act(void) {
    TEST("Sense and Act channels");
    /* Set sensor[2] to 99, sense it, act on channel 5 */
    /* 0=SENSE2, 1=PUSH42, 2=ACT5, 3=SENSE5, 4=POP, 5=PUSH1, 6=HALT */
    int code_len = 7;
    LauInstruction code[] = {
        {OP_SENSE, 0, 2},
        {OP_PUSH, 42.0, 0},
        {OP_ACT, 0, 5},
        {OP_SENSE, 0, 5},
        {OP_POP, 0, 0},
        {OP_PUSH, 1.0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    vm.sensors[2] = 99.0;
    lau_vm_run(&vm, code, code_len, 100);
    ASSERT_DOUBLE_EQ(vm.actuators[5], 42.0, "actuator[5] should be 42");
    PASS();
}

static void test_stack_underflow(void) {
    TEST("Stack underflow error");
    LauInstruction code[] = {
        {OP_ADD, 0, 0}
    };
    LauVm vm = lau_vm_new();
    int r = lau_vm_run(&vm, code, 1, 10);
    ASSERT(r == -1, "stack underflow should return -1");
    PASS();
}

static void test_max_steps(void) {
    TEST("Max steps limit");
    /* Infinite loop */
    LauInstruction code[] = {
        {OP_JMP, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    lau_vm_run(&vm, code, 2, 50);
    ASSERT(!vm.halted, "should not be halted");
    PASS();
}

static void test_thermostat(void) {
    TEST("Complex program: thermostat (sense -> compare -> act)");
    /* If sensor[0] > 25, set actuator[0] = 1 (cooling on), else = 0 */
    /* 0=SENSE0, 1=PUSH25, 2=GT, 3=JZ(op=7), 4=PUSH1, 5=ACT0, 6=JMP(op=9) */
    /* 7=PUSH0, 8=ACT0, 9=HALT */
    int code_len = 10;
    LauInstruction code[] = {
        {OP_SENSE, 0, 0},
        {OP_PUSH, 25.0, 0},
        {OP_GT, 0, 0},
        {OP_JZ, 0, 7},
        {OP_PUSH, 1.0, 0},
        {OP_ACT, 0, 0},
        {OP_JMP, 0, 9},
        {OP_PUSH, 0.0, 0},
        {OP_ACT, 0, 0},
        {OP_HALT, 0, 0}
    };
    /* Test with temp 30 -> cooling on */
    {
        LauVm vm = lau_vm_new();
        vm.sensors[0] = 30.0;
        lau_vm_run(&vm, code, code_len, 100);
        ASSERT_DOUBLE_EQ(vm.actuators[0], 1.0, "cooling should be on at 30C");
    }
    /* Test with temp 20 -> cooling off */
    {
        LauVm vm = lau_vm_new();
        vm.sensors[0] = 20.0;
        lau_vm_run(&vm, code, code_len, 100);
        ASSERT_DOUBLE_EQ(vm.actuators[0], 0.0, "cooling should be off at 20C");
    }
    PASS();
}

static void test_conservation(void) {
    TEST("Conservation check: sum sensors, compare to expected");
    /* Sum all 8 sensors and check if match expected 888 */
    /* 0..7=SENSE0..7 interleaved with ADD */
    int code_len = 18;
    LauInstruction code[] = {
        {OP_SENSE, 0, 0},
        {OP_SENSE, 0, 1},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 2},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 3},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 4},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 5},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 6},
        {OP_ADD, 0, 0},
        {OP_SENSE, 0, 7},
        {OP_ADD, 0, 0},
        {OP_PUSH, 888.0, 0},
        {OP_EQ, 0, 0},
        {OP_HALT, 0, 0}
    };
    LauVm vm = lau_vm_new();
    vm.sensors[0] = 88.0;
    vm.sensors[1] = 100.0;
    vm.sensors[2] = 200.0;
    vm.sensors[3] = 50.0;
    vm.sensors[4] = 50.0;
    vm.sensors[5] = 100.0;
    vm.sensors[6] = 200.0;
    vm.sensors[7] = 100.0;
    lau_vm_run(&vm, code, code_len, 100);
    ASSERT_DOUBLE_EQ(lau_vm_stack_top(&vm), 1.0, "sum should equal 888");
    PASS();
}

int main(void) {
    printf("=== LAU Bytecode VM Tests ===\n\n");

    test_push_and_add();
    test_push_and_sub();
    test_multiply();
    test_division_and_div_by_zero();
    test_negate();
    test_store_and_load_local();
    test_dup();
    test_swap();
    test_comparison_lt_gt_eq();
    test_conditional_jump_jz();
    test_conditional_jump_jnz();
    test_unconditional_jump();
    test_fibonacci();
    test_call_ret();
    test_vibe_get_set();
    test_sense_act();
    test_stack_underflow();
    test_max_steps();
    test_thermostat();
    test_conservation();

    printf("\n=== Results: %d tests, %d failed ===\n", test_count, failed);
    return failed > 0 ? 1 : 0;
}
