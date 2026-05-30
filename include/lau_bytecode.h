#ifndef LAU_BYTECODE_H
#define LAU_BYTECODE_H

typedef enum {
    OP_PUSH, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG,
    OP_LOAD, OP_STORE, OP_JMP, OP_JZ, OP_JNZ,
    OP_LT, OP_GT, OP_EQ, OP_DUP, OP_POP, OP_SWAP,
    OP_HALT, OP_CALL, OP_RET, OP_VIBE_GET, OP_VIBE_SET,
    OP_SENSE, OP_ACT
} LauOpcode;

typedef struct {
    LauOpcode op;
    double value;
    int operand;
} LauInstruction;

typedef struct {
    double stack[256];
    int sp;
    double locals[16];
    int pc;
    double vibe;
    double sensors[8];
    double actuators[8];
    int call_stack[64];
    int csp;
    int halted;
    int tick;
} LauVm;

LauVm lau_vm_new(void);
int lau_vm_step(LauVm *vm, const LauInstruction *code, int code_len);
int lau_vm_run(LauVm *vm, const LauInstruction *code, int code_len, int max_steps);
double lau_vm_stack_top(const LauVm *vm);
int lau_vm_stack_pop(LauVm *vm, double *out);

#endif /* LAU_BYTECODE_H */
