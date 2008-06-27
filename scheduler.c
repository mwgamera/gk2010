#include "scheduler.h"

typedef struct scheduler_task {
	
	short id;
	int fd;
	void (*handler)(void*);
	void *data;

	struct scheduler_task *next_fd;

} scheduler_task_t;

/* Tasks indexes */
scheduler_task_t**scheduler_tasks_id; /* table indexed by id */
scheduler_task_t**scheduler_tasks_df; /* table of linked lists indexed by fd */

short scheduler_register() {
	return 0;
}
