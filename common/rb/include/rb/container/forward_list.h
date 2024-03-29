//
// Created by robcholz on 3/2/24.
//
#pragma once

#ifndef RB_CONTAINER_FORWARD_LIST_H
#define RB_CONTAINER_FORWARD_LIST_H

#include "../miscellaneous/type.h"
#include "../allocator/allocator.h"

struct forward_list {
  struct single_node *head;
  size_t length;
  struct allocator allocator;
};

extern void forward_list_init(struct forward_list *self, struct allocator allocator);

extern void forward_list_finalize(struct forward_list *self);

extern void forward_list_node_free_finalize(struct forward_list *self);

extern struct optional forward_list_front(struct forward_list *self);

extern struct optional forward_list_back(struct forward_list *self);

extern size_t forward_list_length(struct forward_list *self);

extern uint8_t forward_list_is_empty(struct forward_list *self);

extern void forward_list_insert_front(struct forward_list *self, data_t data);

extern void forward_list_insert_back(struct forward_list *self, data_t data);

extern struct optional forward_list_delete_front(struct forward_list *self);

extern struct optional forward_list_delete_back(struct forward_list *self);

extern struct optional forward_list_search(struct forward_list *self, data_t data);

extern void forward_list_iterate(struct forward_list *self, void (*lambda)(data_t));

extern void forward_list_format_print(struct forward_list *self, void (*printer_lambda)(data_t));

extern void forward_list_print_uint8(data_t data);

extern void forward_list_print_uint16(data_t data);

extern void forward_list_print_uint32(data_t data);

extern void forward_list_print_uint64(data_t data);

#endif //RB_CONTAINER_FORWARD_LIST_H
