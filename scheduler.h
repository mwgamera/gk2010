#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/* Type of event to wait for. */
typedef enum scheduler_fd_type {
	SCHEDULER_FD_READ	= 0,
	SCHEDULER_FD_WRITE	= 1,
	SCHEDULER_FD_EXCEPT	= 2
} scheduler_fd_t;

/* Registers a handler to be called when fd is ready to perform
 * nonblocking operation specified by type. Returns task ID. */
short scheduler_register(int fd, scheduler_fd_t type, void (*handler)(void*), void *data);

/* Unregisters a task from scheduler. Will crash when given bad task.
 * If you need to unregister tasks which you didn't registered, use
 * scheduler_registered to check whether task is valid. */
void scheduler_unregister(short task);

/* Run the main loop in which we wait for data and call the handlers.
 * When specified event occurs, first handlers waiting for exceptions
 * are called followed by write and read handlers. Handlers with lower
 * fd numbers are run first. Handlers for the same fd and type are run
 * in order they were registered. */
void scheduler_main(void);

/* Finish the main loop and do the clean exit.
 * One of handlers should call that to finish the program. */
void scheduler_stop(void);

/* Causes return from main loop but keeps the internal
 * state untouched. Another call of scheduler_main will
 * continue from next select. */
void scheduler_break(void);

/* True if given task is registered. */
int scheduler_registered(short task);

#endif /* _SCHEDULER_H_ */
