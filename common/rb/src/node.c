//
// Created by robcholz on 3/2/24.
//
#include "rb/miscellaneous/node.h"

void single_node_init(struct single_node* self, data_t data) {
  self->next = NULL;
  self->data = data;
}
