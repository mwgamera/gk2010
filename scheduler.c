#include <stdlib.h>
#include "scheduler.h"

typedef struct scheduler_task {
	
	short id;
	int fd;
	void (*handler)(void*);
	void *data;

	struct scheduler_task *next_fd;

} scheduler_task_t;

/* Tasks indexes */
static scheduler_task_t **scheduler_tasks_id; /* table indexed by id */
static scheduler_task_t **scheduler_tasks_fd; /* table of lists indexed by fd */
static int tasks_id_max = 0;
static int tasks_fd_max = 0;

/* Register a task */
short scheduler_register(int fd, void (*fun)(void*), void *dat) {

	/* create a task */
	scheduler_task_t *tt, *task = malloc(sizeof*task);
	task->fd = fd;
	task->handler = fun;
	task->data = dat;
	task->next_fd = NULL;

	/* resize indexes */
	scheduler_tasks_id = realloc( scheduler_tasks_id,
		++tasks_id_max * sizeof*scheduler_tasks_id );
	if (tasks_fd_max <= fd) {
		scheduler_tasks_fd = realloc( scheduler_tasks_fd,
			(fd+1) * sizeof*scheduler_tasks_fd );
		while (tasks_fd_max <= fd)
			scheduler_tasks_fd[tasks_fd_max++] = NULL;
	}

	/* put task in id index */
	scheduler_tasks_id[ task->id = tasks_id_max-1 ] = task;
	
	/* and descriptors index */
	if ((tt = scheduler_tasks_fd[fd]) == NULL)
		scheduler_tasks_fd[fd] = task;
	else {
		while (tt->next_fd != NULL)
			tt = tt->next_fd;
		tt->next_fd = task;
	}
	
	return task->id;
}

