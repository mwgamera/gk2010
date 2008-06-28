#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/* Registers a handler to be called when data arrives at
 * fd file descriptor. Handlers for same fd are called
 * in order as they were registered. Returns a number of
 * task in scheduler. */
short scheduler_register(int fd, void (*handler)(void*), void *data);

/* Unregisters a task from scheduler. Will crash when given bad task.
 * If you need to unregister tasks which you didn't registered, use
 * scheduler_registered to check whether task is valid. */
void scheduler_unregister(short task);

/* Run the main loop in which we wait for data and call the handlers. */
void scheduler_main(void);

/* Finish the main loop and do the clean exit.
 * One of handlers should call that to finish the program. */
void scheduler_stop(void);

/* Causes return from main loop but keeps the internal
 * state untouched. Another call of scheduler_main will
 * continue from next select. */
void scheduler_break(void);

/* True if given task is registered */
int scheduler_registered(short task);

#endif /* _SCHEDULER_H_ */
