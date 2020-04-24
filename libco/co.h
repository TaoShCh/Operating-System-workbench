#ifndef __CO_H__
#define __CO_H__

typedef void (*func_t)(void *arg);
struct co;

void co_init();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);
#include <stdint.h>

#if defined(__i386__)
  #define SP "%%esp"
  #define BP "%%ebp"
  #define IP "%%eip"
  #define reg_t uint32_t
#elif defined(__x86_64__)
  #define SP "%%rsp"
  #define BP "%%rbp"
  #define IP "%%rip"
  #define reg_t uint64_t
#endif
reg_t get_bp();
reg_t get_ep();
#endif
