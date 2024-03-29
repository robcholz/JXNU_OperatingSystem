//
// Created by robcholz on 3/2/24.
//
#include "rb/util/optional.h"

uint8_t optional_has_value(struct optional *self) {
  return self->has_value;
}

data_t optional_get_value(struct optional *self) {
  return self->data;
}

struct optional optional_init_null() {
  struct optional optional;
  optional.has_value = 0;
  optional.data = NULL;
  return optional;
}

struct optional optional_init(data_t data) {
  struct optional optional;
  optional.has_value = 1;
  optional.data = data;
  return optional;
}