#ifndef MEMORY_INFO_LEAKAGE_H
#define MEMORY_INFO_LEAKAGE_H

#define SEED_MEMORY(seed) srand(seed); initRepeatedVal();

void initRepeatedVal(void);
void *__wrap_malloc(size_t);
void *get_stack_top(void);
void *get_cur_stack_bottom(void);
void *get_min_stack_bottom(void);

void fill_stack(void);

#define FILL_STACK() { \
  uint64_t *__stack_bottom = (uint64_t *)get_cur_stack_bottom(); \
  uint64_t repeatedVal = (uint64_t)rand() << 48 | (uint64_t)rand() << 32 | (uint64_t)rand() << 16 | (uint64_t)rand(); \
  volatile uint64_t *stack_loc; stack_loc = (void *)(&stack_loc - 1); \
 \
  for (; stack_loc > __stack_bottom; stack_loc--) { \
    *stack_loc = repeatedVal; \
  } \
 \
  stack_loc = (uint64_t *)repeatedVal; \
  __stack_bottom = (uint64_t *)repeatedVal; \
}  

#endif
