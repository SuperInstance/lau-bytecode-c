#include "lau_bytecode.h"
#include <string.h>

LauVm lau_vm_new(void) {
    LauVm vm;
    memset(&vm, 0, sizeof(vm));
    return vm;
}

static int stack_push(LauVm *vm, double val) {
    if (vm->sp >= 256) return -1;
    vm->stack[vm->sp++] = val;
    return 0;
}

int lau_vm_stack_pop(LauVm *vm, double *out) {
    if (vm->sp <= 0) return -1;
    *out = vm->stack[--vm->sp];
    return 0;
}

double lau_vm_stack_top(const LauVm *vm) {
    return vm->sp > 0 ? vm->stack[vm->sp - 1] : 0.0;
}

int lau_vm_step(LauVm *vm, const LauInstruction *code, int code_len) {
    if (vm->halted) return 1;
    if (vm->pc < 0 || vm->pc >= code_len) return -1;

    const LauInstruction *inst = &code[vm->pc];
    vm->tick++;
    double a, b;

    switch (inst->op) {
    case OP_PUSH:
        if (stack_push(vm, inst->value) != 0) return -1;
        vm->pc++;
        break;

    case OP_ADD:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a + b) != 0) return -1;
        vm->pc++;
        break;

    case OP_SUB:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a - b) != 0) return -1;
        vm->pc++;
        break;

    case OP_MUL:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a * b) != 0) return -1;
        vm->pc++;
        break;

    case OP_DIV:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (b == 0.0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a / b) != 0) return -1;
        vm->pc++;
        break;

    case OP_NEG:
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, -a) != 0) return -1;
        vm->pc++;
        break;

    case OP_LOAD:
        if (inst->operand < 0 || inst->operand >= 16) return -1;
        if (stack_push(vm, vm->locals[inst->operand]) != 0) return -1;
        vm->pc++;
        break;

    case OP_STORE:
        if (inst->operand < 0 || inst->operand >= 16) return -1;
        if (lau_vm_stack_pop(vm, &vm->locals[inst->operand]) != 0) return -1;
        vm->pc++;
        break;

    case OP_JMP:
        vm->pc = inst->operand;
        break;

    case OP_JZ:
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        vm->pc = (a == 0.0) ? inst->operand : vm->pc + 1;
        break;

    case OP_JNZ:
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        vm->pc = (a != 0.0) ? inst->operand : vm->pc + 1;
        break;

    case OP_LT:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a < b ? 1.0 : 0.0) != 0) return -1;
        vm->pc++;
        break;

    case OP_GT:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a > b ? 1.0 : 0.0) != 0) return -1;
        vm->pc++;
        break;

    case OP_EQ:
        if (lau_vm_stack_pop(vm, &b) != 0) return -1;
        if (lau_vm_stack_pop(vm, &a) != 0) return -1;
        if (stack_push(vm, a == b ? 1.0 : 0.0) != 0) return -1;
        vm->pc++;
        break;

    case OP_DUP:
        if (vm->sp <= 0) return -1;
        if (stack_push(vm, vm->stack[vm->sp - 1]) != 0) return -1;
        vm->pc++;
        break;

    case OP_POP:
        if (vm->sp <= 0) return -1;
        vm->sp--;
        vm->pc++;
        break;

    case OP_SWAP:
        if (vm->sp < 2) return -1;
        a = vm->stack[vm->sp - 1];
        vm->stack[vm->sp - 1] = vm->stack[vm->sp - 2];
        vm->stack[vm->sp - 2] = a;
        vm->pc++;
        break;

    case OP_CALL:
        if (vm->csp >= 64) return -1;
        vm->call_stack[vm->csp++] = vm->pc + 1;
        vm->pc = inst->operand;
        break;

    case OP_RET:
        if (vm->csp <= 0) return -1;
        vm->pc = vm->call_stack[--vm->csp];
        break;

    case OP_VIBE_GET:
        if (stack_push(vm, vm->vibe) != 0) return -1;
        vm->pc++;
        break;

    case OP_VIBE_SET:
        if (lau_vm_stack_pop(vm, &vm->vibe) != 0) return -1;
        vm->pc++;
        break;

    case OP_SENSE:
        if (inst->operand < 0 || inst->operand >= 8) return -1;
        if (stack_push(vm, vm->sensors[inst->operand]) != 0) return -1;
        vm->pc++;
        break;

    case OP_ACT:
        if (inst->operand < 0 || inst->operand >= 8) return -1;
        if (lau_vm_stack_pop(vm, &vm->actuators[inst->operand]) != 0) return -1;
        vm->pc++;
        break;

    case OP_HALT:
        vm->halted = 1;
        return 1;

    default:
        return -1;
    }

    return 0;
}

int lau_vm_run(LauVm *vm, const LauInstruction *code, int code_len, int max_steps) {
    int steps = 0;
    int result;
    while (steps < max_steps) {
        result = lau_vm_step(vm, code, code_len);
        steps++;
        if (result != 0) break;
    }
    if (!vm->halted && result == 0) {
        /* hit max steps without halting - not an error, just stopped */
        return 0;
    }
    return result;
}
