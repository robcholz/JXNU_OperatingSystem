#include <rb/allocator/allocator.h>
#include <rb/container/forward_list.h>
#include <rb/util/optional.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


enum process_status {
  PROCESS_STATUS_RUNNING,
  PROCESS_STATUS_READY,
  PROCESS_STATUS_WAITING,
  PROCESS_STATUS_TERMINATED,
};

enum process_priority {
  PROCESS_PRIORITY_SUPER_HIGH,
  PROCESS_PRIORITY_HIGH,
  PROCESS_PRIORITY_NORMAL,
  PROCESS_PRIORITY_LOW,
};

enum process_scheduler_signal {
  PROCESS_SCHEDULER_SIGNAL_TERMINATE,
  PROCESS_SCHEDULER_SIGNAL_CONTINUE,
};

typedef uint32_t custom_pid_t;

struct process_control_block {
  custom_pid_t pid;                           // process id
  enum process_status status;             // process status
  enum process_priority desire_priority;  // process priority
  enum process_priority current_priority;
  uint32_t (*task_handler)();          // task pointer
  struct process_control_block* next;  // next pointer
  struct process_control_block* end;   // back pointer
  uint64_t service_time;               // estimated running time
  uint64_t call_count;                 // times of call
};

struct process_scheduler {
  // ready queue
  struct process_control_block* ready_queue_priority_superhigh;
  struct process_control_block* ready_queue_priority_high;
  struct process_control_block* ready_queue_priority_normal;
  struct process_control_block* ready_queue_priority_low;
  // wait queue
  struct process_control_block* wait_queue;
  // running pcb
  // there is only one process running at a time, this value can be null
  struct process_control_block* running_process;
  // pid allocator
  uint32_t current_allocated_pid;  // this is a self incremental id, the current
                                   // allocated the largest id
  struct forward_list reusable_pid_list;
  // memory handler
  struct allocator pcb_allocator;
  clock_t time_slice;
  clock_t time_elapsed;
};

/// process control block
extern void process_control_block_init(struct process_control_block* self,
                                       custom_pid_t pid,
                                       enum process_status status,
                                       uint8_t priority,
                                       uint32_t (*task_handler)());
extern void process_control_block_init_default(
    struct process_control_block* self,
    uint32_t (*task_handler)());

/// process scheduler
// public functions
extern void process_scheduler_init(struct process_scheduler* self);
extern void process_scheduler_finalize(struct process_scheduler* self);
extern void process_scheduler_run(struct process_scheduler* self,
                                  enum process_scheduler_signal signal);
// control the process lifecycle
extern struct process_control_block* process_scheduler_create_process(
    struct process_scheduler* self,
    enum process_priority priority,
    uint32_t (*task_handler)());
extern void process_scheduler_wait_process(
    struct process_scheduler* self,
    struct process_control_block* process);
extern void process_scheduler_terminate_process(
    struct process_scheduler* self,
    struct process_control_block* process);
extern void process_scheduler_wait_process_millisecond(
    struct process_scheduler* self,
    struct process_control_block* process,
    uint64_t millisecond);

// internal implementation
extern void process_scheduler_finalize_queue(
    struct process_scheduler* self,
    struct process_control_block* queue);
// process scheduler pid related
extern custom_pid_t process_scheduler_allocate_pid(struct process_scheduler* self);
extern void process_scheduler_deallocate_pid(struct process_scheduler* self,
                                             custom_pid_t pid);
// pcb queue related operation
extern void process_scheduler_push_to_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue,
    struct process_control_block* process);
extern void process_scheduler_push_to_pcb_ready_queue(
    struct process_scheduler* self,
    struct process_control_block* process,
    enum process_priority priority);
extern struct process_control_block* process_scheduler_pop_from_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue);
extern uint8_t process_scheduler_has_element_in_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue);
// operations when process done a time slice
extern void process_scheduler_current_process_slice_initialize(
    struct process_scheduler* self);
extern void process_scheduler_current_process_slice_finalize(
    struct process_scheduler* self,
    uint64_t time_elapsed);
extern void process_scheduler_current_process_slice_ready_finalize(
    struct process_scheduler* self,
    uint64_t time_elapsed);

void process_control_block_init(struct process_control_block* self,
                                custom_pid_t pid,
                                enum process_status status,
                                uint8_t priority,
                                uint32_t (*task_handler)()) {
  self->pid = pid;
  self->status = status;
  self->desire_priority = priority;
  self->current_priority = priority;
  self->service_time = 0;
  self->next = NULL;
  self->end = NULL;
  self->task_handler = task_handler;
  self->call_count = 0;
}

void process_control_block_init_default(struct process_control_block* self,
                                        uint32_t (*task_handler)()) {
  process_control_block_init(self, 0, PROCESS_STATUS_WAITING,
                             PROCESS_PRIORITY_NORMAL, task_handler);
}

void process_scheduler_init(struct process_scheduler* self) {
  self->pcb_allocator = allocator_init_default_libc();
  self->ready_queue_priority_superhigh = allocator_malloc(
      &self->pcb_allocator, sizeof(struct process_control_block));
  self->ready_queue_priority_high = allocator_malloc(
      &self->pcb_allocator, sizeof(struct process_control_block));
  self->ready_queue_priority_normal = allocator_malloc(
      &self->pcb_allocator, sizeof(struct process_control_block));
  self->ready_queue_priority_low = allocator_malloc(
      &self->pcb_allocator, sizeof(struct process_control_block));
  self->wait_queue = allocator_malloc(&self->pcb_allocator,
                                      sizeof(struct process_control_block));
  self->running_process = NULL;
  self->current_allocated_pid = 0;
  self->time_slice = 50;  // 50 ms for time slice
  self->time_elapsed = 0;
  forward_list_init(&self->reusable_pid_list, allocator_init_default_libc());
  process_control_block_init_default(self->ready_queue_priority_superhigh,
                                     NULL);
  process_control_block_init_default(self->ready_queue_priority_high, NULL);
  process_control_block_init_default(self->ready_queue_priority_normal, NULL);
  process_control_block_init_default(self->ready_queue_priority_low, NULL);
  process_control_block_init_default(self->wait_queue, NULL);
}

void process_scheduler_finalize_queue(struct process_scheduler* self,
                                      struct process_control_block* queue) {
  struct process_control_block* pcb;
  do {
    pcb = process_scheduler_pop_from_pcb_queue(self, queue);
    if (pcb != NULL)
      process_scheduler_terminate_process(self, pcb);
  } while (pcb != NULL);
}

void process_scheduler_finalize(struct process_scheduler* self) {
  // if there is running process, set it to wait
  if (self->running_process != NULL) {
    process_scheduler_terminate_process(self, self->running_process);
  }

  // free pcb existing in the queues
  process_scheduler_finalize_queue(self, self->ready_queue_priority_superhigh);
  process_scheduler_finalize_queue(self, self->ready_queue_priority_high);
  process_scheduler_finalize_queue(self, self->ready_queue_priority_normal);
  process_scheduler_finalize_queue(self, self->ready_queue_priority_low);
  process_scheduler_finalize_queue(self, self->wait_queue);

  // free the pid list
  forward_list_node_free_finalize(&self->reusable_pid_list);

  // free the queue head
  allocator_free(&self->pcb_allocator, self->ready_queue_priority_superhigh);
  allocator_free(&self->pcb_allocator, self->ready_queue_priority_high);
  allocator_free(&self->pcb_allocator, self->ready_queue_priority_normal);
  allocator_free(&self->pcb_allocator, self->ready_queue_priority_low);
  allocator_free(&self->pcb_allocator, self->wait_queue);
}

custom_pid_t process_scheduler_allocate_pid(struct process_scheduler* self) {
  struct optional option = forward_list_delete_front(&self->reusable_pid_list);
  if (optional_has_value(&option)) {
    void* value_ptr = optional_get_value(&option);
    custom_pid_t pid = *(custom_pid_t*)(value_ptr);
    allocator_free(&self->reusable_pid_list.allocator, value_ptr);
    return pid;
  }
  return ++self->current_allocated_pid;
}

void process_scheduler_deallocate_pid(struct process_scheduler* self,
                                      custom_pid_t pid) {
  custom_pid_t* pid_ptr =
      allocator_malloc(&self->reusable_pid_list.allocator, sizeof(custom_pid_t));
  *pid_ptr = pid;
  forward_list_insert_front(&self->reusable_pid_list, pid_ptr);
}

void process_scheduler_push_to_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue,
    struct process_control_block* process) {
  process->next = NULL;
  if (queue->end != NULL) {
    queue->end->next = process;
  }  // queue has existing element
  else {
    queue->next = process;
  }  // first element in the queue
  queue->end = process;
}

void process_scheduler_push_to_pcb_ready_queue(
    struct process_scheduler* self,
    struct process_control_block* process,
    enum process_priority priority) {
  switch (priority) {
    case PROCESS_PRIORITY_SUPER_HIGH: {
      process_scheduler_push_to_pcb_queue(
          self, self->ready_queue_priority_superhigh, process);
      break;
    }
    case PROCESS_PRIORITY_HIGH: {
      process_scheduler_push_to_pcb_queue(self, self->ready_queue_priority_high,
                                          process);
      break;
    }
    case PROCESS_PRIORITY_NORMAL: {
      process_scheduler_push_to_pcb_queue(
          self, self->ready_queue_priority_normal, process);
      break;
    }
    case PROCESS_PRIORITY_LOW: {
      process_scheduler_push_to_pcb_queue(self, self->ready_queue_priority_low,
                                          process);
      break;
    }
  }
}

struct process_control_block* process_scheduler_pop_from_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue) {
  struct process_control_block* popped = queue->next;
  if (popped != NULL) {
    queue->next = popped->next;
    if (popped->next == NULL)
      queue->end = NULL;
  }
  return popped;
}

uint8_t process_scheduler_has_element_in_pcb_queue(
    struct process_scheduler* self,
    struct process_control_block* queue) {
  return queue->next != NULL;
}

// return the const ptr of pcb
struct process_control_block* process_scheduler_create_process(
    struct process_scheduler* self,
    enum process_priority priority,
    uint32_t (*task_handler)()) {
  struct process_control_block* new_process = allocator_malloc(
      &self->pcb_allocator, sizeof(struct process_control_block));
  process_control_block_init(new_process, process_scheduler_allocate_pid(self),
                             PROCESS_STATUS_READY, priority, task_handler);
  process_scheduler_push_to_pcb_ready_queue(self, new_process, priority);
  return new_process;
}

void process_scheduler_wait_process(struct process_scheduler* self,
                                    struct process_control_block* process) {
  process->status = PROCESS_STATUS_WAITING;
  process_scheduler_push_to_pcb_queue(self, self->wait_queue, process);
}

void process_scheduler_terminate_process(
    struct process_scheduler* self,
    struct process_control_block* process) {
  process->status = PROCESS_STATUS_TERMINATED;
  process_scheduler_deallocate_pid(self, process->pid);
  allocator_free(&self->pcb_allocator, process);
}

void process_scheduler_wait_process_millisecond(
    struct process_scheduler* self,
    struct process_control_block* process,
    uint64_t millisecond) {
  process_scheduler_wait_process(self, process);
  // TODO,
}

void process_scheduler_current_process_slice_initialize(
    struct process_scheduler* self) {
  struct process_control_block* process;
  if (process_scheduler_has_element_in_pcb_queue(
          self, self->ready_queue_priority_superhigh)) {
    process = process_scheduler_pop_from_pcb_queue(
        self, self->ready_queue_priority_superhigh);
  } else if (process_scheduler_has_element_in_pcb_queue(
                 self, self->ready_queue_priority_high)) {
    process = process_scheduler_pop_from_pcb_queue(
        self, self->ready_queue_priority_high);
  } else if (process_scheduler_has_element_in_pcb_queue(
                 self, self->ready_queue_priority_normal)) {
    process = process_scheduler_pop_from_pcb_queue(
        self, self->ready_queue_priority_normal);
  } else if (process_scheduler_has_element_in_pcb_queue(
                 self, self->ready_queue_priority_low)) {
    process = process_scheduler_pop_from_pcb_queue(
        self, self->ready_queue_priority_low);
  } else {
    process = NULL;
  }
  if (process != NULL)
    process->status = PROCESS_STATUS_RUNNING;
  self->running_process = process;
}

void process_scheduler_current_process_slice_finalize(
    struct process_scheduler* self,
    uint64_t time_elapsed) {}

void process_scheduler_current_process_slice_ready_finalize(
    struct process_scheduler* self,
    uint64_t time_elapsed) {
  struct process_control_block* process = self->running_process;
  process->service_time += time_elapsed;
  process->call_count++;
  process->status = PROCESS_STATUS_READY;
  switch (process->current_priority) {
    case PROCESS_PRIORITY_SUPER_HIGH: {
      process->current_priority = PROCESS_PRIORITY_HIGH;
      break;
    }
    case PROCESS_PRIORITY_HIGH: {
      process->current_priority = PROCESS_PRIORITY_NORMAL;
      break;
    }
    case PROCESS_PRIORITY_NORMAL: {
      process->current_priority = PROCESS_PRIORITY_LOW;
      break;
    }
    case PROCESS_PRIORITY_LOW: {
      process->current_priority = process->desire_priority;
      break;
    }
  }
  process_scheduler_push_to_pcb_ready_queue(self, process,
                                            process->current_priority);
}

void process_scheduler_run(struct process_scheduler* self,
                           enum process_scheduler_signal signal) {
time_slice_init: {
  if (signal == PROCESS_SCHEDULER_SIGNAL_TERMINATE)
    return;
  // handle algorithm here
  // process is taken from the queue
  process_scheduler_current_process_slice_initialize(self);
  goto time_slice_start;

time_slice_start: {
  clock_t start_time, current_time;
  start_time = clock();  // Get the initial time
  uint8_t signal = 1;
  while (signal==1) {
    // program running
    uint32_t return_value = self->running_process->task_handler();
    return_value;
    // program running
    current_time = clock();
    self->time_elapsed = (current_time - start_time) * 1000 / CLOCKS_PER_SEC;
    if (self->time_elapsed < self->time_slice) {
      // satisfy the time slice
      signal = 0;
      process_scheduler_current_process_slice_ready_finalize(
          self, self->time_elapsed);
      goto time_slice_init;
    } else {
      // time slice expired
      signal = 0;
      process_scheduler_current_process_slice_ready_finalize(
          self, self->time_elapsed);
      goto time_slice_init;
    }
  }
}
}
}

struct process_scheduler scheduler;
struct process_control_block *test_process1, *test_process2, *test_process3,
    *test_process4;

uint32_t test_process_callback1() {
  printf("callback1 send you a msg, called %ld times \n",
         test_process1->call_count);
  process_scheduler_wait_process_millisecond(&scheduler,test_process1,5000);
  printf("callback1 completed the task!\n");
  return 0;
}

uint32_t test_process_callback2() {
  printf("callback2 send you a msg, called %ld times \n",
         test_process2->call_count);
  process_scheduler_wait_process_millisecond(&scheduler,test_process1,3000);
  printf("callback2 completed the task!\n");
  return 0;
}

uint32_t test_process_callback3() {
  printf("callback3 send you a msg, called %ld times \n",
         test_process3->call_count);
  process_scheduler_wait_process_millisecond(&scheduler,test_process1,4000);
  printf("callback3 completed the task!\n");
  return 0;
}

uint32_t test_process_callback4() {
  printf("callback4 send you a msg, called %ld times \n",
         test_process4->call_count);
  process_scheduler_wait_process_millisecond(&scheduler,test_process1,2000);
  printf("callback4 completed the task!\n");
  return 0;
}

int main() {
  process_scheduler_init(&scheduler);
  test_process1 = process_scheduler_create_process(
      &scheduler, PROCESS_PRIORITY_SUPER_HIGH, test_process_callback1);
  test_process2 = process_scheduler_create_process(
      &scheduler, PROCESS_PRIORITY_HIGH, test_process_callback2);
  test_process3 = process_scheduler_create_process(
      &scheduler, PROCESS_PRIORITY_NORMAL, test_process_callback3);
  test_process4 = process_scheduler_create_process(
      &scheduler, PROCESS_PRIORITY_LOW, test_process_callback4);

  process_scheduler_run(&scheduler, PROCESS_SCHEDULER_SIGNAL_CONTINUE);

  process_scheduler_finalize(&scheduler);
  return 0;
}