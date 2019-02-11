#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>


#define STACK_MAX 256


struct {
    uint8_t *ip;

    /* Fixed-size stack */
    double stack[STACK_MAX];
    double *stack_top;

    /* A single register containing the result */
    double result;
} vm;


typedef enum {
    /* push the immediate argument onto the stack */
    OP_PUSHI,

    /* pop 2 values from the stack, add and push the result onto the stack */
    OP_ADD,

    /* pop 2 values from the stack, subtract and push the result onto the stack */
    OP_SUB,

    /* pop 2 values from the stack, divide and push the result onto the stack */
    OP_DIV,

    /* pop 2 values from the stack, multiply and push the result onto the stack */
    OP_MUL,

    /* pop 2 values from the stack, push C onto the stack where C = A^B */
    OP_POW,

    /* pop 1 value from the stack, push square root of it onto the stack */
    OP_SQRT,

    /* pop 1 value from the stack, push natural logarithm of it onto the stack */
    OP_LN,

    /* pop the top of the stack and set it as execution result */
    OP_POP_RES,

    /* stop execution */
    OP_DONE,
} opcode;


typedef enum interpret_result {
    SUCCESS,
    ERROR_DIVISION_BY_ZERO,
    ERROR_UNKNOWN_OPCODE,
} interpret_result;


void vm_reset(void)
{
    puts("Reset vm state");
    vm = (typeof(vm)) { NULL };
    vm.stack_top = vm.stack;
}


void vm_stack_push(double value)
{
    *vm.stack_top = value;
    vm.stack_top++;
}


double vm_stack_pop(void)
{
    vm.stack_top--;
    return *vm.stack_top;
}


interpret_result vm_interpret(uint8_t *bytecode)
{
    vm_reset();

    puts("Start interpreting");
    vm.ip = bytecode;
    for (;;) {
        uint8_t instruction = *vm.ip++;
        switch (instruction) {
            case OP_PUSHI: {
                /* get the argument, push it onto stack */
                double arg = *vm.ip++;
                vm_stack_push(arg);
                break;
            }
            case OP_ADD: {
                /* Pop 2 values, add 'em, push the result back to the stack */
                double arg_right = vm_stack_pop();
                double arg_left = vm_stack_pop();
                double res = arg_left + arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_SUB: {
                /* Pop 2 values, subtract 'em, push the result back to the stack */
                double arg_right = vm_stack_pop();
                double arg_left = vm_stack_pop();
                double res = arg_left - arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_DIV: {
                /* Pop 2 values, divide 'em, push the result back to the stack */
                double arg_right = vm_stack_pop();
                /* Don't forget to handle the div by zero error */
                if (arg_right == 0)
                    return ERROR_DIVISION_BY_ZERO;
                double arg_left = vm_stack_pop();
                double res = arg_left / arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_MUL: {
                /* Pop 2 values, multiply 'em, push the result back to the stack */
                double arg_right = vm_stack_pop();
                double arg_left = vm_stack_pop();
                double res = arg_left * arg_right;
                vm_stack_push(res);
                break;
            }
            case OP_POW: {
                double arg_right = vm_stack_pop();
                double arg_left = vm_stack_pop();
                double res = pow(arg_left, arg_right);
                vm_stack_push(res);
                break;
            }
            case OP_SQRT: {
                double arg = vm_stack_pop();
                double res = sqrt(arg);
                vm_stack_push(res);
                break;
            }
            case OP_LN: {
                double arg = vm_stack_pop();
                double res = log(arg);
                vm_stack_push(res);
                break;
            }
            case OP_POP_RES: {
                /* Pop the top of the stack, set it as a result value */
                double res = vm_stack_pop();
                vm.result = res;
                break;
            }
            case OP_DONE: {
                return SUCCESS;
            }
            default:
                return ERROR_UNKNOWN_OPCODE;
        }
    }

    return SUCCESS;
}


int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    {
        /* Push and pop the result */
        uint8_t code[] = { OP_PUSHI, 5, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 5);
    }

    printf("\n");

    {
        /* Addition */
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 5, OP_ADD, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 15);
    }

    printf("\n");

    {
        /* Subtraction */
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 6, OP_SUB, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 4);
    }

    printf("\n");

    {
        /* Division */
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 5, OP_DIV, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 2);
    }

    printf("\n");

    {
        /* Division with error */
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 0, OP_DIV, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == ERROR_DIVISION_BY_ZERO);
    }

    printf("\n");

    {
        /* Multiplication */
        uint8_t code[] = { OP_PUSHI, 10, OP_PUSHI, 2, OP_MUL, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 20);
    }

    printf("\n");

    {
        /* Expression: 2*(11+3) */
        uint8_t code[] = { OP_PUSHI, 2, OP_PUSHI, 11, OP_PUSHI, 3, OP_ADD, OP_MUL, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 28);
    }

    printf("\n");

    {
        /* POW */
        uint8_t code[] = { OP_PUSHI, 2, OP_PUSHI, 5, OP_POW, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 32);
    }

    printf("\n");

    {
        /* SQRT */
        uint8_t code[] = { OP_PUSHI, 4, OP_PUSHI, 81, OP_SQRT, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 9);
    }

    printf("\n");

    {
        /* LN */
        uint8_t code[] = { OP_PUSHI, 5, OP_LN, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result > 1.60);
        assert(vm.result < 1.61);
    }

    printf("\n");

    {
        /* sum all */
        uint8_t code[] = { OP_PUSHI, 2, OP_PUSHI, 3, OP_PUSHI, 5, OP_ADD, OP_ADD, OP_POP_RES, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result == 10);
    }

    printf("\n");

    {
        /* various operations

            3 * (1 + 3) ^ ln(7 / 3)

             res
             mul
                      pow
                          ln
                  add     div
           3     1   3   7   3
        */

        uint8_t code[] = {
            OP_PUSHI, 3,
            OP_PUSHI, 1,
            OP_PUSHI, 3,
            OP_ADD,
            OP_PUSHI, 7,
            OP_PUSHI, 3,
            OP_DIV,
            OP_LN,
            OP_POW,
            OP_MUL,
            OP_POP_RES,
            OP_DONE
        };
        interpret_result result = vm_interpret(code);
        printf("vm state: %f\n", vm.result);

        assert(result == SUCCESS);
        assert(vm.result > 9.71);
        assert(vm.result < 9.72);
    }

    return EXIT_SUCCESS;
}