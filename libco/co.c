#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include "co.h"

#define MAX_CO 1000

#if defined(__i386__)
  #define SP "%%esp"
#elif defined(__x86_64__)
  #define SP "%%rsp"
#endif

struct co{
    uint8_t stack[4096];
    jmp_buf env;
    uint8_t state;
    uintptr_t stack_backup;
    int id;
};
struct co coroutines[MAX_CO];
struct co *current;
int num_of_co;//number of coroutines till now



void co_init() {
    srand((unsigned int)time(NULL));
    num_of_co=0;
    coroutines[0].state = 1;
    coroutines[0].id = 0;
    current=&coroutines[0];
}

func_t this_func;
void *this_arg;
uintptr_t this_stack;
uintptr_t stack_backup;

struct co* co_start(const char *name, func_t func, void *arg) {
    num_of_co++;
    coroutines[num_of_co].id = num_of_co;
    coroutines[num_of_co].state = 1;
    this_func = func;
    this_arg = arg;
    if(!setjmp(current->env)){
        this_stack = (uintptr_t)&coroutines[num_of_co] + sizeof(coroutines[num_of_co].stack);
        asm volatile("mov " SP ", %0; mov %1, " SP :
                        "=g"(stack_backup) :
                        "g"(this_stack));
        coroutines[num_of_co].stack_backup = stack_backup;
        current = &coroutines[num_of_co];
        this_func(this_arg);
        current -> state = 0;
        asm volatile("mov %0," SP : : "g"(current->stack_backup));
        current = &coroutines[0];
        longjmp(current->env, 1);
    }
    return &coroutines[num_of_co];
}

void co_yield() {
    if(!setjmp(current->env)){
        int chosen = rand() % (num_of_co + 1);
        while(chosen == current->id || coroutines[chosen].state == 0){
            chosen = rand() % (num_of_co + 1);
        }
        current = &coroutines[chosen];
        longjmp(current -> env, 1);
    }
}

void co_wait(struct co *thd) {
    while(thd -> state){
        co_yield();
    }
}

