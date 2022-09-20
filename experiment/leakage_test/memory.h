#ifndef MEMORY_INFO_LEAKAGE_H
#define MEMORY_INFO_LEAKAGE_H

#define SEED_MEMORY(seed) srand(seed);

void *get_stack_top(void);
void *get_cur_stack_bottom(void);
void *get_min_stack_bottom(void);

void fill_stack(void);
void *my_malloc(size_t);

#define FILL_STACK() { \
  uint64_t *__stack_bottom = (uint64_t *)get_cur_stack_bottom(); \
  uint64_t repeatedVal = (uint64_t)rand() << 48 | (uint64_t)rand() << 32 | (uint64_t)rand() << 16 | (uint64_t)rand(); \
  uint64_t *stack_loc; \
 \
  for (stack_loc = &repeatedVal - 1; stack_loc > __stack_bottom; stack_loc--) { \
    *stack_loc = repeatedVal; \
  } \
 \
  stack_loc = (uint64_t *)repeatedVal; \
  __stack_bottom = (uint64_t *)repeatedVal; \
}  

#define malloc(bytes) my_malloc(bytes)

#endif
