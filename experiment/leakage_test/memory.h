#ifndef MEMORY_INFO_LEAKAGE_H
#define MEMORY_INFO_LEAKAGE_H

#define SEED_MEMORY(seed) srand(seed);

void *get_stack_top(void);
void *get_cur_stack_bottom(void);
void *get_min_stack_bottom(void);

void fill_stack(void);
void *my_malloc(size_t);

#define malloc(bytes) my_malloc(bytes)

#endif