//
// Created by robcholz on 3/20/24.
//
#pragma once

#ifndef RB_CONTAINER_HASH_MAP_H
#define RB_CONTAINER_HASH_MAP_H

#include "../allocator/allocator.h"
#include "../miscellaneous/node.h"

static const size_t INITIAL_CAPACITY = 8;
static const float LOAD_FACTOR = 0.5f;

typedef uint32_t hash_map_key_t;

typedef struct hash_map_single_node {
  struct hash_map_single_node* next;
  hash_map_key_t key;
  data_t data;
} hash_map_single_node_t;

struct hash_map {
  // key: uint32_t, value: optional
  hash_map_single_node_t* array;
  size_t array_capacity;
  size_t length;
  struct allocator allocator;
};

extern void hash_map_init_default(struct hash_map* self);

extern void hash_map_insert(struct hash_map* self,
                            hash_map_key_t key,
                            data_t value);

extern uint8_t hash_map_contains(struct hash_map* self, hash_map_key_t key);

extern struct optional hash_map_get(struct hash_map* self, hash_map_key_t key);

extern size_t hash_map_get_length(struct hash_map* self);

extern size_t hash_map_get_capacity(struct hash_map* self);

static size_t hash_map_hash(struct hash_map* self, hash_map_key_t key);

static void hash_map_single_node_init(hash_map_single_node_t* self,
                                      hash_map_key_t key,
                                      data_t data);

static void hash_map_init_array(hash_map_single_node_t* array, size_t length);

#endif  // RB_CONTAINER_HASH_MAP_H
