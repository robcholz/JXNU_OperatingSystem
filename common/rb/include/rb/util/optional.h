//
// Created by robcholz on 3/2/24.
//
#pragma once

#ifndef RB_OPTIONAL_H
#define RB_OPTIONAL_H

#include <stdint.h>
#include "../miscellaneous/type.h"

struct optional {
  uint8_t has_value;
  data_t data;
};

extern uint8_t optional_has_value(struct optional *self);

extern data_t optional_get_value(struct optional *optional);

extern struct optional optional_init_null();

extern struct optional optional_init(data_t data);

#endif //RB_OPTIONAL_H
