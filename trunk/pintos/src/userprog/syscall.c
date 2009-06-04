#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"

#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "filesys/file.h"

#define MAX_LENGTH_WRITE_ONCE 128
#define MAX_FILE_CREATION_SIZE 128
#define MAX_FILE_NAME_LENGTH 128

static uint32_t global_fd;
static struct lock fd_lock;
static void * stack_p;
static void syscall_handler(struct intr_frame *);
bool check_user_pointer(void*);
void syscall_execute(int ,struct intr_frame *);
void* get_arg_pointer(unsigned int);
int get_arg_integer(unsigned int);
struct file* get_file_p(unsigned int);

void syscall_init(void) {
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
	stack_p = NULL;
	global_fd = 2;
	/* initial the lock for synchronizing global fd value retrieving */
	lock_init(&fd_lock);
	/* initial the list of fd_lists */
	list_init(&fds_ll);
	/* initial semaphore for parent waiting for
	   child process to complete loading ELF executable. */
	sema_init(&p_c_sema, 0);
	/* initial the removing list for the files which is going to be removed but still
	   referred by some processes*/
	list_init(&removing_list);
}

/* dispatch the syscall request to certain syscall.
   by Xiaoqi Cao*/
static void syscall_handler(struct intr_frame *f) {
	stack_p = f->esp;
	//get stack pointer, which is pointing onto top of stack
	if (stack_p != NULL) {
		//get system call number
		int number = *((int *) stack_p);
		stack_p += 4;
		syscall_execute(number,f);
	} else {
		thread_exit();
	}
}

/* check the validation of pointers passed from user.
   by Xiaoqi Cao*/
bool check_user_pointer(void* p) {

	bool err = false;

	//check if it is NULL
	if (p == NULL) {
		err = true;
	}
	//check if it points to the kernel VM
	if (p > PHYS_BASE) {
		err = true;
	}
	//TODO check if it points to the unmapped VM
	return !err;
}

/* execute corresponding syscall by its number.
   this function only dispatches the sys call calling
   into corresponding system call and set intr_frame.eax
   register by syscall's return value.
   by Xiaoqi Cao*/
void syscall_execute(int number, struct intr_frame *f) {
	printf("syscall number = %d\n", number);
	int n = 0;
	int m = 0;
	char* p = NULL;
	switch (number) {
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		n = get_arg_integer(0);
		exit(n);
		break;
	case SYS_EXEC:
		p = (char *)get_arg_pointer(0);
		if (p != NULL) {
			pid_t process_identifier;
			process_identifier = exec(p);
			f->eax = process_identifier;
		}
		break;
	case SYS_WAIT:
		n = get_arg_integer(0);
		if (p != NULL) {
			int c_exit_status;
			c_exit_status = wait(n);
			f->eax = c_exit_status;
		}
		break;
	case SYS_CREATE:
		p = (char *)get_arg_pointer(0);
		n = get_arg_integer(1);
//		printf("n = %d, *p = %c, p = %x\n", n, *p, p);
		if (p != NULL) {
			bool suc = false;
			suc = create(p, n);
			f->eax = (uint32_t)suc;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_REMOVE:
		p = (char *)get_arg_pointer(0);
//		printf("*p = %c, p = %x\n", *p, p);
		if (p != NULL) {
			bool suc;
			suc = remove(p);
			f->eax = suc;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_OPEN:
		p = (char *)get_arg_pointer(0);
//		printf("*p = %c, p = %x\n", *p, p);
		if (p != NULL) {
			int fd = open(p);
			f->eax = fd;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_FILESIZE:
		n = get_arg_integer(0);
		f->eax = filesize(n);
		break;
	case SYS_READ:
		n = get_arg_integer(0);
		p = (char *)get_arg_pointer(1);
		m = get_arg_integer(2);
		f->eax = read(n,p,m);
		break;
	case SYS_WRITE:
		n = get_arg_integer(0);
		p = (char *)get_arg_pointer(1);
		m = get_arg_integer(2);
//		printf("n = %d, *p = %c, p = %x, m = %d\n", n, *p, p ,m);
		int retval = write(n,p,m);
		f->eax = retval;
		break;
	case SYS_SEEK:
		n = get_arg_integer(0);
		m = get_arg_integer(1);
		seek(n,m);
		break;
	case SYS_TELL:
		n = get_arg_integer(0);
		f->eax = tell(n);
		break;
	case SYS_CLOSE:
		n = get_arg_integer(0);
		close(n);
		break;
	default:
		printf("Unsupported system call number: %d!\n", number);
		thread_exit();
		break;
	}
}


/* implementation of syscall SYS_HALT
   by Xiaoqi Cao*/
void halt () {
	shutdown_power_off();
}

void exit (int status) {

	//close all the files opened
	struct thread *t = thread_current();
	struct list_elem *open_fde = list_begin(&t->fd_list);
	while(open_fde != list_end(&t->fd_list)) {
		struct list_elem *tmp_open_fde = open_fde;
		list_remove(tmp_open_fde);
	}

	//set all its child processes to orphaned
	struct list_elem *ste = list_begin(&t->sub_threads);
	while(ste != list_end(&t->sub_threads)) {
		struct list_elem *tmp_ste = ste;
		ste = list_next(ste);
		struct thread *st = list_entry(tmp_ste, struct sub_thread, s_t_elem);
		st->pt = NULL;
	}

	process_exit();
}

pid_t exec (const char *file) {

	int pid = process_execute(file);

	if (pid != TID_ERROR) {

		struct sub_thread *st = malloc(sizeof(struct sub_thread));
		st->t = get_thread_by_pid(pid);
		st->waited = false;
		st->exited = false;

		struct thread *t = thread_current();

		list_push_back(&t->sub_threads, &st->s_t_elem);

		return pid;

	}

	return -1;
}

int wait (pid_t pidt) {




	return 0;
}

/* implementation of syscall SYS_CREATE
   by Xiaoqi Cao*/
bool create (const char *file, unsigned initial_size) {

	if (initial_size <= MAX_FILE_CREATION_SIZE) {

		if (strlen(file) > MAX_FILE_NAME_LENGTH) {
			printf("File creation failed because file name is too long! %d\n", strlen(file));
		} else if(strlen(file) == 0) {
			printf("File creation failed because file name can not be empty!\n");
		} else {
			return filesys_create(file, initial_size);
		}
	} else {
		printf("Initial file size is too big, initial_size = %d. Creation failed.\n", initial_size);
	}
	return false;
}

/* implementation of syscall SYS_REMOVE
   by Xiaoqi Cao*/
bool remove (const char *file) {



	return filesys_remove(file);
}


/* implementation of syscall SYS_OPEN
   by Xiaoqi Cao*/
int open (const char *file) {

	if (strlen(file) == 0) {
		printf("File open failed because file name can not be empty!\n");
	} else {
		lock_acquire(&fd_lock);
		struct file *f = filesys_open(file);
		if (f != NULL) {

			uint32_t n_fd = global_fd++;

			if (n_fd >= 2) {
				struct list *fd_list = thread_add_fd(thread_current(), n_fd, f);

//				printf("fd_list.size = %d\n", list_size(fd_list));
//				if (list_size(fd_list) > 0) {
//					struct fd_elem *fde = list_entry(list_rbegin(fd_list), struct fd_elem, fdl_elem);
//					printf("last element = %d", fde->fd);
//				}

				if (fd_list != NULL) {
					if (list_size(fd_list) == 1) {
						struct fds_stub *fdss = malloc(sizeof(struct fds_stub));
						fdss->l = fd_list;
						list_push_back(&fds_ll, &fdss->fds_elem);
					}
				}
			}

			lock_release(&fd_lock);
			return n_fd;
		}
		lock_release(&fd_lock);
	}

	return -1;
}


/* implementation of syscall SYS_FILEZISE
   by Xiaoqi Cao*/
int filesize (int fd) {

	struct file *f = get_file_p(fd);
	if (f != NULL) {
		return file_length(f);
	}
}


/* implementation of syscall SYS_READ
   by Xiaoqi Cao*/
int read (int fd, void *buffer, unsigned length) {
	if (buffer != NULL) {
		unsigned i = 0;
		if (fd == 0) {
			while(i < length) {
				uint8_t key = input_getc();
				char *p = (char *)buffer;
				p[i] = (char)key;
				i++;
			}
			return i;
		} else {
			//TODO read from file indicated by fd
			struct file * f = get_file_p(fd);
			if (f != NULL) {
				off_t readbytes = file_read (f, buffer, length);
				return readbytes;
			}
		}
	}
	return -1;
}

/* implementation of syscall SYS_WRITE
   by Xiaoqi Cao*/
int write (int fd, const void *buffer, unsigned length) {
	if (buffer != NULL && strlen(buffer) > 0) {
		if (fd == 1) {
			while (length > MAX_LENGTH_WRITE_ONCE) {
				char *p = (char *)buffer;
				putbuf(p, MAX_LENGTH_WRITE_ONCE);
				length = length - MAX_LENGTH_WRITE_ONCE;
				p = p + MAX_LENGTH_WRITE_ONCE;
			}
			putbuf(buffer, length);
			return length;
		} else {
			//TODO write to file indicated by fd
			struct file * f = get_file_p(fd);
			if (f != NULL) {
				off_t writebytes = file_write (f, buffer, length);
				return writebytes;
			}
		}
	}
	return 0;
}

/* implementation of syscall SYS_SEEK
   by Xiaoqi Cao*/
void seek (int fd, unsigned position) {
	struct file *f = get_file_p(fd);
	if (f != NULL) {
		file_seek(f, position);
	}
}

/* implementation of syscall SYS_TELL
   by Xiaoqi Cao*/
unsigned tell (int fd) {

	struct file *f = get_file_p(fd);

	off_t pos = -1;

	if (f != NULL) {
		pos = file_tell (f);
	}

	return pos;
}

/* implementation of syscall SYS_CLOSE
   by Xiaoqi Cao*/
void close (int fd) {
	struct file *f = get_file_p(fd);
	if (f != NULL) {
		// get thread

		struct thread *t = get_thread_by_fd(fd);
		if (t != NULL) {
			// thread_remove_fd
			struct thread *t = thread_current();
			thread_remove_fd(t, fd);

			// if thread_has empty fd_list ==> remove empty fd_list from fds_ll
			if (list_empty(&t->fd_list)) {
				struct list_elem *fdlle = list_begin(&fds_ll);
				while(fdlle != list_end(*fds_ll)) {
					struct list_elem *tmpfdlle = fdlle;
					fdlle = list_next(fdlle);
					struct fds_stub *fdss = list_entry(tmpfdlle, struct fds_stub, fds_elem);
					if (fdss->l == &t->fd_list) {
						list_remove(fdss);
						// free fds_stub
						free(fdss);
						break;
					}
				}
			}
		}
		file_close(f);
	}
}

/* get pointer of file by fd
   returns NULL if can not find fd which equals to given value
   by Xiaoqi Cao*/
struct file* get_file_p(unsigned int fd) {

	if (fd == 1 || fd == 0) {
		printf("fd = %d, can not get file structure pointer.\n", fd);
		return NULL;
	}

	struct list_elem *le = list_begin(&fds_ll);
	while(le != list_end(&fds_ll)) {
		struct list_elem *tmp = le;
		le = list_next(le);
		struct fds_stub *fdss = list_entry(tmp, struct fds_stub, fds_elem);
		if (fdss != NULL) {
			struct list * fd_list = fdss->l;
			if (fd_list != NULL) {
				struct list_elem *fd_le = list_begin(fd_list);
				while(fd_le != list_end(fd_list)) {
					struct list_elem *tmp_fd_le = fd_le;
					fd_le = list_next(fd_le);
					struct fd_elem *fde = list_entry(tmp_fd_le, struct fd_elem, fdl_elem);
					if (fde != NULL) {
						if (fde->fd == fd) {

							return fde->f;
						}
					}
				}
			}
		}
	}
	return NULL;
}

/* Get argument as a pointer
   It can be converted into char* or another pointer type.
   It at the same time does validation work to the user pointer
   by Xiaoqi Cao*/
void* get_arg_pointer(unsigned int index) {
	if (index <=2 && stack_p != NULL) {

		void * user_pointer = *((void **)(stack_p + index * 4));
		if (check_user_pointer(user_pointer)) {
			return user_pointer;
		} else {
			//invalid pointer
			printf("Can not call system call, invalid user pointer!");
			//TODO release resources

			//terminate process
			process_exit();
		}
	}

	return NULL;
}

/* Get argument as an integer
   by Xiaoqi Cao*/
int get_arg_integer(unsigned int index) {
	if (index <=2 && stack_p != NULL) {
		return *((int*)(stack_p + index * 4));
	}
}
