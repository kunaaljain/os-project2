#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/kernel/list.h"


struct fds_stub {
	struct list *l;
	struct list_elem fds_elem;
};

struct list fds_ll;

void syscall_init (void);

#endif /* userprog/syscall.h */
