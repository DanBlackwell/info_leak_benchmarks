#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>

void *get_stack_top() {

  FILE *file = fopen("/proc/self/maps", "r");
  char buf[4096];
  uintptr_t start, end, offset, major, minor, unknown;
  char flags[5];
  char name[400];

  while (fgets(buf, sizeof(buf), file)) {
    sscanf(buf, "%lx-%lx %4c %lx %ld:%ld %ld %s", &start, &end, flags, &offset, &major, &minor, &unknown, name);
    if (strcmp("[stack]", name) == 0) {
      break;
    }
  }

  return (void *)end;
}

void *get_cur_stack_bottom() {
  FILE *file = fopen("/proc/self/maps", "r");
  char buf[4096];
  uintptr_t start, end, offset, major, minor, unknown;
  char flags[5];
  char name[400];

  while (fgets(buf, sizeof(buf), file)) {
    sscanf(buf, "%lx-%lx %4c %lx %ld:%ld %ld %s", &start, &end, flags, &offset, &major, &minor, &unknown, name);
    if (strcmp("[stack]", name) == 0) {
      break;
    }
  }

  return (void *)start;
}

void *get_min_stack_bottom() {
  void *min = get_stack_top();

  struct rlimit limit;
  getrlimit (RLIMIT_STACK, &limit);
  return (char *)min - limit.rlim_cur;
}

void *my_malloc(size_t bytes) {
  short *raw = (short *)malloc(bytes);
  for (size_t i = 0; i < bytes / sizeof(short); i++) {
    raw[i] = rand();
  }

  return (void *)raw;
}

void fill_stack() {
  uint64_t *__stack_bottom = (uint64_t *)get_cur_stack_bottom();
  uint64_t repeatedVal = (uint64_t)rand() << 48 | (uint64_t)rand() << 32 | (uint64_t)rand() << 16 | (uint64_t)rand();
  volatile uint64_t *stack_loc; stack_loc = (void *)(&stack_loc - 1);

  for (; stack_loc > __stack_bottom; stack_loc--) {
    *stack_loc = repeatedVal;
  }

  stack_loc = (uint64_t *)repeatedVal;
  __stack_bottom = (uint64_t *)repeatedVal;
}
