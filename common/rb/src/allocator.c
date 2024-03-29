#include <stdlib.h>
#include "rb/allocator/allocator.h"

void allocator_init(struct allocator *self, void *(*malloc)(size_t), void (*free)(void *)) {
  self->malloc = malloc;
  self->free = free;
}

struct allocator allocator_init_default_libc() {
  struct allocator allocator;
  allocator_init(&allocator, malloc, free);
  return allocator;
}

void *allocator_malloc(struct allocator *self, size_t bytes) {
  return self->malloc(bytes);
}

void allocator_free(struct allocator *self, void *pointer) {
  self->free(pointer);
}
