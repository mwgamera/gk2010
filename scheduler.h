#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/* Registers a handler to be called when data arrives at
 * fd file descriptor. Handlers for same fd are called
 * in order as they were registered. Returns a number of
 * task in scheduler. */
short scheduler_register(int fd, void (*handler)(void*), void *data);

/* Unregisters a task from scheduler. */
void scheduler_unregister(short task);

/* Run the main loop in which we wait for data and call the handlers. */
void scheduler_main(void);

#endif /* _SCHEDULER_H_ */
