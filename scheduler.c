#include <stdlib.h>
#include <sys/select.h> /* POSIX 1003.1-2001 */
#include "scheduler.h"

typedef struct scheduler_task {

  short id;
  int fd;
  scheduler_fd_t type;
  void (*handler)(void*);
  void *data;

  struct scheduler_task *next_fd;

} scheduler_task_t;

/* Tasks indexes */
static scheduler_task_t **scheduler_tasks_id = NULL; /* table indexed by id */
static scheduler_task_t **scheduler_tasks_fd = NULL; /* table of lists indexed by fd */
static int tasks_id_max = 0;
static int tasks_fd_max = 0;

/* Break flag */
static int scheduler_break_flag = 0;

/* Register a task */
short scheduler_register(int fd, scheduler_fd_t t, void (*fun)(void*), void *dat) {

  scheduler_task_t *tt, *task, **tp;

  /* resize indexes */
  tp = realloc( scheduler_tasks_id,
      (tasks_id_max+1) * sizeof*scheduler_tasks_id );
  if (tp == NULL) return -1;
  scheduler_tasks_id = tp;
  tasks_id_max++;

  if (tasks_fd_max <= fd) {
    tp = realloc( scheduler_tasks_fd,
        (fd+1) * sizeof*scheduler_tasks_fd );
    if (tp == NULL) return -1;
    scheduler_tasks_fd = tp;
    while (tasks_fd_max <= fd)
      scheduler_tasks_fd[tasks_fd_max++] = NULL;
  }

  /* create a task */
  task = malloc(sizeof*task);
  task->fd = fd;
  task->type = t;
  task->handler = fun;
  task->data = dat;
  task->next_fd = NULL;

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

/* Remove a task. */
void scheduler_unregister(short id) {
  int i = tasks_id_max;
  scheduler_task_t *tt, *task = scheduler_tasks_id[id];

  /* removing task from id index */
  scheduler_tasks_id[id] = NULL;

  /* removing task from fd index (nb. will surely crash when index corrupted) */
  if ((tt = scheduler_tasks_fd[task->fd]) == task)
    scheduler_tasks_fd[task->fd] = task->next_fd;
  else {
    while( tt->next_fd != task )
      tt = tt->next_fd;
    tt->next_fd = task->next_fd; /* relink */
  }

  /* recalculate max values */
  i = tasks_id_max;
  while (scheduler_tasks_id[--i] == NULL)
    --tasks_id_max;
  i = tasks_fd_max;
  while (scheduler_tasks_fd[--i] == NULL)
    --tasks_fd_max;

  /* free task */
  free(task);
}

int scheduler_registered(short id) {
  /* invalid when out of allocated range of set NULL */
  if (id < 0 || id >= tasks_id_max || scheduler_tasks_id[id] == NULL)
    return 0;
  else
    return 1;
}

/* Run the main loop */
void scheduler_main() {
  int i;
  fd_set fds[3];

  /* refuse to work without tasks */
  if (!tasks_fd_max) return; 

  /* select */
  scheduler_break_flag = 0;

  do {  /* set up a set */
    for(i=0; i<3; i++)
      FD_ZERO(&(fds[i]));

    /* WARNING: Relies on fact that type values are enforced to be 0-2 */
    for (i=0; i < tasks_id_max; i++)
      if (scheduler_tasks_id[i] != NULL)
        FD_SET(scheduler_tasks_id[i]->fd, &(fds[scheduler_tasks_id[i]->type]));

    /* select */
    if (!select(tasks_fd_max,
          &(fds[SCHEDULER_FD_READ]),
          &(fds[SCHEDULER_FD_WRITE]),
          &(fds[SCHEDULER_FD_EXCEPT]), NULL))
      scheduler_break_flag = 1;
    else
    for (i=0; i < tasks_fd_max; i++) {
      unsigned k = 3;
      while (k-- > 0)
        if (FD_ISSET(i,&fds[k])) {
          scheduler_task_t *tt = scheduler_tasks_fd[i];
          while (tt != NULL) {
            if (tt->type == k)
              tt->handler(tt->data);
            if (scheduler_break_flag)
              return;
            tt = tt->next_fd;
          }
        }
    }
  } while (!scheduler_break_flag);
  return;
}

/* Break the main */
void scheduler_break() { scheduler_break_flag = 1; }

/* Free everything and break */
void scheduler_stop() {
  if (tasks_id_max)
    while (tasks_id_max--)
      if (scheduler_tasks_id[tasks_id_max] != NULL)
        free(scheduler_tasks_id[tasks_id_max]);
  tasks_fd_max = 0;

  free(scheduler_tasks_id);
  free(scheduler_tasks_fd);

  scheduler_tasks_id = NULL;
  scheduler_tasks_fd = NULL;

  scheduler_break();
}
