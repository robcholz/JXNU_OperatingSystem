//
// Created by robcholz on 3/2/24.
//
#pragma once

#ifndef RB_NODE_H
#define RB_NODE_H

#include "type.h"
#include "../util/optional.h"

struct single_node {
  struct single_node *next;
  data_t data;
};

extern void single_node_init(struct single_node *self, data_t data);

#endif //RB_NODE_H
