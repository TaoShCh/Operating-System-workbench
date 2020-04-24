#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>

#define STACK_SIZE 4096
#define FREE 0
#define RUNNING 1


struct task {
    const char *name;
    void (*entry)(void *);
    void *arg;
    char *stack;
    int state;
    int cpu;
    _Context context;
};

struct spinlock {
    volatile int locked;
    const char *name;
    int cpu;
};

struct semaphore {
    int value;
    spinlock_t lk;
    const char *name;
};

#endif
