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
  short *__stack_bottom = (short *)get_cur_stack_bottom(); 
  short __stack_tmp;
  for (short *stack_loc = &__stack_tmp; stack_loc > __stack_bottom; stack_loc--) {
    *stack_loc = rand();
  }
}
