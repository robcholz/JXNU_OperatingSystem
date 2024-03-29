//
// Created by robcholz on 3/2/24.
//
#include <stdio.h>
#include "rb/miscellaneous/node.h"
#include "rb/util/optional.h"
#include "rb/container/forward_list.h"

void forward_list_init(struct forward_list *self, struct allocator allocator) {
  self->allocator = allocator;
  self->length = 0;
  self->head = allocator_malloc(&self->allocator, sizeof(struct single_node));
  single_node_init(self->head, NULL);
}

void forward_list_finalize(struct forward_list *self) {
  struct optional op;
  while (1) {
	op = forward_list_delete_front(self);
	if (!optional_has_value(&op)) {
	  allocator_free(&self->allocator, self->head);
	  return;
	}
  }
}

void forward_list_node_free_finalize(struct forward_list *self) {
  struct optional op;
  while (1) {
	op = forward_list_delete_front(self);
	if (optional_has_value(&op))
	  allocator_free(&self->allocator, optional_get_value(&op));
	else {
	  allocator_free(&self->allocator, self->head);
	  return;
	}
  }
}

struct optional forward_list_front(struct forward_list *self) {
  if (self->head->next==NULL)
	return optional_init_null();
  return optional_init(self->head->next->data);
}

struct optional forward_list_back(struct forward_list *self) {
  struct single_node *node = self->head->next;
  while (node!=NULL) {
	if (node->next==NULL)
	  return optional_init(node->data);
	node = node->next;
  }
  return optional_init_null();
}

size_t forward_list_length(struct forward_list *self) {
  return self->length;
}

uint8_t forward_list_is_empty(struct forward_list *self) {
  return self->length==0;
}

void forward_list_insert_front(struct forward_list *self, data_t data) {
  struct single_node *new_node = allocator_malloc(&self->allocator, sizeof(struct single_node));
  single_node_init(new_node, data);
  new_node->next = self->head->next;
  self->head->next = new_node;

  ++self->length;
}

void forward_list_insert_back(struct forward_list *self, data_t data) {
  struct single_node *new_node = allocator_malloc(&self->allocator, sizeof(struct single_node));
  single_node_init(new_node, data);

  struct single_node **back_node_addr = &self->head;
  while (*back_node_addr!=NULL) {
	back_node_addr = &(*back_node_addr)->next;
  }
  *back_node_addr = new_node;

  ++self->length;
}

struct optional forward_list_delete_front(struct forward_list *self) {
  if (self->length==0)
	return optional_init_null();

  struct single_node *del_node = self->head->next;
  data_t data = del_node->data;
  self->head->next = del_node->next;
  allocator_free(&self->allocator, del_node);

  --self->length;

  return optional_init(data);
}

struct optional forward_list_delete_back(struct forward_list *self) {
  if (self->length==0)
	return optional_init_null();

  struct single_node **back_node_addr = &self->head;
  while ((*back_node_addr)->next!=NULL) {
	back_node_addr = &(*back_node_addr)->next;
  }

  data_t data = (*back_node_addr)->data;
  allocator_free(&self->allocator, *back_node_addr);
  *back_node_addr = NULL;

  --self->length;

  return optional_init(data);
}

struct optional forward_list_search(struct forward_list *self, data_t data) {
  struct single_node *node = self->head;

  while (node!=NULL) {
	node = node->next;

	if (node!=NULL && node->data==data)
	  return optional_init(data);
  }

  return optional_init_null();
}

void forward_list_iterate(struct forward_list *self, void (*lambda)(data_t)) {
  struct single_node *node = self->head->next;
  while (node!=NULL) {
	lambda(node->data);
	node = node->next;
  }
}

void forward_list_format_print(struct forward_list *self, void (*printer_lambda)(data_t)) {
  printf("forward_list[");
  struct single_node *node = self->head->next;
  while (node!=NULL) {
	printer_lambda(node->data);
	if (node->next!=NULL)
	  printf(",");
	node = node->next;
  }
  printf("]");
}

void forward_list_print_uint8(data_t data) {
  printf("%d", *((uint8_t *)data));
}

void forward_list_print_uint16(data_t data) {
  printf("%d", *((uint16_t *)data));
}

void forward_list_print_uint32(data_t data) {
  printf("%d", *((uint32_t *)data));
}

void forward_list_print_uint64(data_t data) {
  printf("%lu", *((uint64_t *)data));
}
