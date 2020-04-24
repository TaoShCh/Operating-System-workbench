static inline void cli(){
    asm volatile("cli");
}

static inline void sti(){
    asm volatile("sti");
}

static inline void hlt(){
    asm volatile("hlt");
}

static inline void pause(){
    asm volatile("pause");
}

static inline uint32_t get_efl() {
  volatile uint32_t efl;
  asm volatile ("pushf; pop %0": "=r"(efl));
  return efl;
}

#define FL_IF 0x00000200

#define LOCKDEF(name) \
  static volatile intptr_t name##_locked = 0; \
  static int name##_lock_flags[8]; \
  void name##_lock() { \
    name##_lock_flags[_cpu()] = get_efl() & FL_IF; \
    cli(); \
    while (1) { \
      if (0 == _atomic_xchg(&name##_locked, 1)) break; \
      pause(); \
    } \
  } \
  void name##_unlock() { \
    _atomic_xchg(&name##_locked, 0); \
    if (name##_lock_flags[_cpu()]) sti(); \
  }
