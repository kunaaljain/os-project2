#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/kernel/list.h"

/* the item of fd_list head, fd_list is the fds held by a thread*/
struct fds_stub {
	struct list *l;
	struct list_elem fds_elem;
};

/* the list of fd_list head, fd_list is the fds held by a thread*/
struct list fds_ll;


/* the item to record the file which will be deleted in future due to a process calls
   the remove system call, but it will not be deleted now due to some process may refer it
   , after all the process release its fd corresponding to the certain file, the file
   this is for implementing the unix file removing mechanism.*/
struct removing_file {
	int fd;
	struct thread *t;
	char *file_name;
	struct list_elem r_elem;
};

/* the list of removing file*/
struct list removing_list;

void syscall_init (void);

#endif /* userprog/syscall.h */
