#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/kernel/list.h"


struct fds_stub {
	struct list *l;
	struct list_elem fds_elem;
};

struct list fds_ll;

struct removing_file {
	int fd;
	struct thread *t;
	char *file_name;
	struct list_elem r_elem;
};

struct list removing_list;

struct semaphore p_c_sema;

void syscall_init (void);

#endif /* userprog/syscall.h */
