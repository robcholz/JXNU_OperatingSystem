//
// Created by robcholz on 3/20/24.
//
#include "rb/container/hash_map.h"

void hash_map_init_default(struct hash_map* self) {
  self->allocator = allocator_init_default_libc();
  self->array = allocator_malloc(
      &self->allocator, INITIAL_CAPACITY * sizeof(hash_map_single_node_t));
  self->array_capacity = INITIAL_CAPACITY;
  self->length = 0;
  hash_map_init_array(self->array, self->array_capacity);
}

void hash_map_insert(struct hash_map* self, hash_map_key_t key, data_t value) {
  hash_map_single_node_t* slot = &self->array[hash_map_hash(self, key)];
  if (slot->next == NULL) {
    slot->data = value;
  }  // no collision
  else {
    hash_map_single_node_t* new_node =
        allocator_malloc(&self->allocator, sizeof(hash_map_single_node_t));
    hash_map_single_node_init(new_node, key, value);
    hash_map_single_node_t** slot_addr = &slot;
    while ((*slot_addr) != NULL)
      slot_addr = &(*slot_addr)->next;
    *slot_addr = new_node;
  }  // has collision
}

uint8_t hash_map_contains(struct hash_map* self, hash_map_key_t key) {
  struct optional option = hash_map_get(self, key);
  return optional_has_value(&option);
}

struct optional hash_map_get(struct hash_map* self, hash_map_key_t key) {
  hash_map_single_node_t* slot = &self->array[hash_map_hash(self, key)];
  while (slot) {
    if (slot->next == NULL && slot->key == key)  // only has one element
      return optional_init(slot->data);
    slot = slot->next;
  }
  return optional_init_null();
}

size_t hash_map_get_length(struct hash_map* self){
  return self->length;
}

size_t hash_map_get_capacity(struct hash_map* self){
  return self->array_capacity;
}

size_t hash_map_hash(struct hash_map* self, hash_map_key_t key) {
  return key % self->array_capacity;
}

void hash_map_single_node_init(hash_map_single_node_t* self,
                               hash_map_key_t key,
                               data_t data) {
  self->next = NULL;
  self->key = key;
  self->data = data;
}

void hash_map_init_array(hash_map_single_node_t* array, size_t length) {
  size_t counter = 0;
  for (; counter < length; ++counter) {
    hash_map_single_node_init(&array[counter], 0, NULL);
  }
}
