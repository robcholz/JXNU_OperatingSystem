//
// Created by robcholz on 3/8/24.
//
#pragma once

#ifndef RB_ALLOCATOR_H
#define RB_ALLOCATOR_H

#include <stddef.h>

struct allocator {
  void *(*malloc)(size_t);
  void (*free)(void *);
};

extern void allocator_init(struct allocator *self, void *(*malloc)(size_t), void (*free)(void *));

extern struct allocator allocator_init_default_libc();

extern void *allocator_malloc(struct allocator *self, size_t bytes);

extern void allocator_free(struct allocator *self, void *pointer);

#endif //RB_ALLOCATOR_H
