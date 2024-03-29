//
// Created by robcholz on 3/1/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <rb/util/optional.h>
#include <rb/container/forward_list.h>
#include <rb/allocator/allocator.h>

int main() {
  struct forward_list list;
  forward_list_init(&list,allocator_init_default_libc());

  for (size_t i = 0; i < 10; ++i) {
	uint8_t *val = malloc(sizeof(uint8_t));
	*val = i;
	forward_list_insert_front(&list, val);
  }
  forward_list_format_print(&list, forward_list_print_uint8);
  printf("\n");

  struct optional op = forward_list_delete_back(&list);
  free(optional_get_value(&op));

  forward_list_format_print(&list, forward_list_print_uint8);
  printf("\n");

  forward_list_node_free_finalize(&list);
  return 0;
}