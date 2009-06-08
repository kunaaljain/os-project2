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
#define MAX_FILE_CREATION_SIZE 20480
#define MAX_FILE_NAME_LENGTH 128

struct lock sys_call_lock;

static uint32_t global_fd;
static struct lock fd_lock;

static void syscall_handler(struct intr_frame *);
bool check_user_pointer(void*);
void syscall_execute(int ,struct intr_frame *, void*);
void* get_arg_pointer(unsigned int, void*);
int get_arg_integer(unsigned int, void*);
struct file* get_file_p(unsigned int);
bool is_referred(char*);
bool already_in_removing_list(struct thread*, char*, int);
void release_resource(void);

void syscall_init(void) {
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
	global_fd = 2;
	/* initial the lock for synchronizing global fd value retrieving */
	lock_init(&fd_lock);
	/* initial the list of fd_lists */
	list_init(&fds_ll);
//	/* initial semaphore for parent waiting for
//	   child process to complete loading ELF executable. */
	sema_init(&p_c_sema, 0);

//	list_init(&exec_sema_list);

	/* initial the removing list for the files which is going to be removed but still
	   referred by some processes*/
	list_init(&removing_list);
	/* lock for all system calls
	   system call gets lock before it runs
	   releases lock after it runs*/
	lock_init(&sys_call_lock);
}

void
msg (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  vprintf (format, args);
  va_end (args);
}

/* dispatch the syscall request to certain syscall.
   by Xiaoqi Cao*/
static void syscall_handler(struct intr_frame *f) {
	void * stack_p;

	stack_p = f->esp;
	//get stack pointer, which is pointing onto top of stack
	if (stack_p != NULL) {
		//get system call number
		int number = *((int *) stack_p);
		stack_p += 4;

//		hex_dump(0, stack_p, 128, true);

//		lock_acquire(&sys_call_lock);
		syscall_execute(number,f, stack_p);
//		lock_release(&sys_call_lock);
	} else {
		thread_exit();
	}
}

/* check the validation of pointers passed from user.
   by Xiaoqi Cao*/
bool check_user_pointer(void* p) {

//	printf("%x\n", (char *)p);

	bool err = false;
	//check if it is NULL
	if (p == NULL) {
//		printf("1");
		err = true;
		return !err;
	}
	//check if it points to the kernel VM
	if (p >= PHYS_BASE) {
//		printf("2");
		err = true;
		return !err;
	}
	//check if it points to the unmapped VM
	if (pagedir_get_page(thread_current()->pagedir, p) == NULL) {
//		printf("3");
		err = true;
		return !err;
	}

//	printf("err = %d\n", err);

	return !err;
}

/* execute corresponding syscall by its number.
   this function only dispatches the sys call calling
   into corresponding system call and set intr_frame.eax
   register by syscall's return value.
   by Xiaoqi Cao*/
void syscall_execute(int number, struct intr_frame *f, void* stack_p) {
	if (number != 9) {
		printf("thread %d syscall number = %d\n", thread_current()->tid, number);
	}
	int n = 0;
	int m = 0;
	char* p = NULL;
	switch (number) {

	case SYS_HALT://0
		halt();
		break;
	case SYS_EXIT://1
		n = get_arg_integer(0, stack_p);
		exit(n);
		break;
	case SYS_EXEC://2
		p = (char *)get_arg_pointer(0, stack_p);
		if (p != NULL) {
			pid_t process_identifier;
			process_identifier = exec(p);
			f->eax = process_identifier;
		}
		break;
	case SYS_WAIT://3
//		printf("thread %d SYS_WAIT\n", thread_current()->tid);
		n = get_arg_integer(0, stack_p);
		int c_exit_status;
//		printf("thread %d waits %d\n", thread_current()->tid, n);
		c_exit_status = wait(n);
		f->eax = c_exit_status;
		break;
	case SYS_CREATE://4
		p = (char *)get_arg_pointer(0, stack_p);
		n = get_arg_integer(1, stack_p);
//		printf("n = %d, *p = %c, p = %x\n", n, *p, p);
		if (p != NULL) {
			bool suc = false;
			suc = create(p, n);
			f->eax = (uint32_t)suc;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_REMOVE://5
		p = (char *)get_arg_pointer(0, stack_p);
//		printf("*p = %c, p = %x\n", *p, p);
		if (p != NULL) {
			bool suc;
			suc = remove(p);
			f->eax = suc;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_OPEN://6
		p = (char *)get_arg_pointer(0, stack_p);
//		printf("*p = %c, p = %x\n", *p, p);
		if (p != NULL) {
			int fd = open(p);
			f->eax = fd;
		}
//		printf("f->eax = %d\n", f->eax);
		break;
	case SYS_FILESIZE://7
		n = get_arg_integer(0, stack_p);
		f->eax = filesize(n);
		break;
	case SYS_READ://8
		n = get_arg_integer(0, stack_p);
		p = (char *)get_arg_pointer(1, stack_p);
		m = get_arg_integer(2, stack_p);
		f->eax = read(n,p,m);
		break;
	case SYS_WRITE://9
		n = get_arg_integer(0, stack_p);
		p = (char *)get_arg_pointer(1, stack_p);
		m = get_arg_integer(2, stack_p);
//		printf("n = %d, *p = %c, p = %x, m = %d\n", n, *p, p ,m);
		int retval = write(n,p,m);
		f->eax = retval;
		break;
	case SYS_SEEK://10
		n = get_arg_integer(0, stack_p);
		m = get_arg_integer(1, stack_p);
		seek(n,m);
		break;
	case SYS_TELL://11
		n = get_arg_integer(0, stack_p);
		f->eax = tell(n);
		break;
	case SYS_CLOSE://12
		n = get_arg_integer(0, stack_p);
		close(n);
		break;
	default:
//		printf("Unsupported system call number: %d!\n", number);
		thread_exit();
		break;
	}
}


/* implementation of syscall SYS_HALT
   by Xiaoqi Cao*/
void halt () {
	shutdown_power_off();
}

/* implementation of syscall SYS_EXIT
   by Xiaoqi Cao*/
void exit (int status) {

//	printf("exiting status = %d, pid = %d\n", status, thread_current()->tid);

//	printf("%d1", thread_current()->tid);

	//close all the files opened
	struct thread *t = thread_current();
	struct list_elem *open_fde = list_begin(&t->fd_list);
	while(open_fde != list_end(&t->fd_list)) {
		struct list_elem *tmp_open_fde = open_fde;
		open_fde = list_next(open_fde);
		list_remove(tmp_open_fde);
	}

//	printf("exiting, fd_list size = %d\n", list_size(&t->fd_list));

//	printf("%d1", thread_current()->tid);

	//remove item form removing list if it has
	struct list_elem *rle = list_begin(&removing_list);
	while(rle != list_end(&removing_list)) {
		struct list_elem *tmprle = rle;
		rle = list_next(rle);
		struct removing_file *r_f = list_entry(tmprle, struct removing_file, r_elem);
		if (r_f->t == t) {
//			printf("removing %s from removing list\n", r_f->file_name);
			list_remove(tmprle);
		}
	}

//	printf("exiting, removing_list size = %d\n", list_size(&removing_list));
//	printf("%d1", thread_current()->tid);

	//set all its child processes to orphaned
	struct list_elem *ste = list_begin(&t->sub_threads);
	while(ste != list_end(&t->sub_threads)) {
		struct list_elem *tmp_ste = ste;
		ste = list_next(ste);
		struct thread *st = list_entry(tmp_ste, struct sub_thread, s_t_elem);
		st->pt = NULL;
	}

	//set exit code and exit flag to parent thread

//	printf("%d1", thread_current()->tid);

	if (t->tid > 3) {
		struct thread *pt = t->pt;
		struct list_elem *stle = list_begin(&pt->sub_threads);
		while(stle != list_end(&pt->sub_threads)) {
			struct list_elem *tmpstle = stle;
			stle = list_next(stle);
			struct sub_thread *st = list_entry(tmpstle, struct sub_thread, s_t_elem);
			if (st->pid == t->tid) {
				//find itself, record exit info
				st->exited = true;
				st->exit_code = status;
//				printf("st-status = %d\n", status);
				if (st->waited) {

					printf("process %d is waited by parent %d\n", t->tid, pt->tid);

					st->waited = false;
					printf("thread %d, sub_thread %d, sema_value %d\n", pt->tid, t->tid, st->waited_sema.value);
					sema_up(&st->waited_sema);
					struct thread *parent = t->pt;
					printf("parent thread id = %d\n", parent->tid);
//					thread_unblock(t->pt);
					printf("thread %d, sub_thread %d, sema_value %d\n", pt->tid, t->tid, st->waited_sema.value);
				}

				printf("st->exited %d, st->exit_code = %d, st->waited = %d\n", st->exited, st->exit_code, st->waited);

				break;
			}
		}
	}

//	printf("1");

	//Process Termination Messages

    if (t->tid > 2) {
		char *p_space = strchr(t->name, (int)(' '));
		if (p_space != NULL) {
		   char file_name_[p_space - t->name + 1];
		   strlcpy(file_name_, t->name, (p_space - t->name + 1));
		   msg ("%s: exit(%d)\n", file_name_, status);
		} else {
		   msg ("%s: exit(%d)\n", t->name, status);
		}
    }
//	printf ("%s: exit(%d)\n", t->name, status);

	thread_exit();
}


/* implementation of syscall SYS_EXEC
   by Xiaoqi Cao*/
pid_t exec (const char *file) {

//	printf("exec %s\n", file);

	struct sub_thread *st = malloc(sizeof(struct sub_thread));

	st->waited = false;
	st->exited = false;
	sema_init(&st->waited_sema, 0);

	struct thread *t = thread_current();
//	lock_acquire(&sys_call_lock);
	int pid = process_execute(file);

	if (pid != TID_ERROR) {

		if (st != NULL) {
			st->pid = pid;
			struct thread *ct = get_thread_by_pid(pid);

			ct->pt = t;
//			printf("st->pid = %d\n", st->pid);
			list_push_back(&t->sub_threads, &st->s_t_elem);
//			printf("thread %d's child_list_size = %d\n", t->tid, list_size(&t->sub_threads));
		}
//		lock_release(&sys_call_lock);
		return pid;

	}

//	lock_release(&sys_call_lock);


	return -1;
}

/* implementation of syscall SYS_WAIT
   by Xiaoqi Cao*/
int wait (pid_t pidt) {

	return process_wait(pidt);
}

/* implementation of syscall SYS_CREATE
   by Xiaoqi Cao*/
bool create (const char *file, unsigned initial_size) {

	if (initial_size <= MAX_FILE_CREATION_SIZE) {

		if (strlen(file) > MAX_FILE_NAME_LENGTH) {
//			printf("File creation failed because file name is too long! %d\n", strlen(file));
			;
		} else if(strlen(file) == 0) {
//			printf("File creation failed because file name can not be empty!\n");
			;
		} else {
			return filesys_create(file, initial_size);
		}
	} else {
//		printf("Initial file size is too big, initial_size = %d. Creation failed.\n", initial_size);
		;
	}
	return false;
}

/* implementation of syscall SYS_REMOVE
   by Xiaoqi Cao*/
bool remove (const char *file) {
	if (!is_referred(file)) {
		return filesys_remove(file);
	}
	return true;
}

/* implementation of syscall SYS_OPEN
   by Xiaoqi Cao*/
int open (const char *file) {

	if (strlen(file) == 0) {
//		printf("File open failed because file name can not be empty!\n");
		;
	} else {

		//if the file is required to be removed,
		//it is invisible for opening operation.

		struct list_elem *rfe = list_begin(&removing_list);

//		printf("rfe == NULL ? %d\n", rfe==NULL);
//		printf("removing list size = %d\n", list_size(&removing_list));

		while(rfe != list_end(&removing_list)) {
			struct list_elem *tmprfe = rfe;
			rfe = list_next(rfe);
			struct removing_file *rf = list_entry(tmprfe, struct removing_file, r_elem);
			if (strcmp(rf->file_name, file) == 0) {
				return -1;
			}
		}

		//if the file is not required to be removed,
		//it can be opened.
		lock_acquire(&fd_lock);
		struct file *f = filesys_open(file);
		if (f != NULL) {
			uint32_t n_fd = global_fd++;

			//fd == 1 screen
			//fd == 0 keyboard
			//fd >= 2 file
			if (n_fd >= 2) {
				struct list *fd_list = thread_add_fd(thread_current(), n_fd, f, file);
//				printf("fd_list size = %d\n", list_size(fd_list));
				if (fd_list != NULL) {
					if (list_size(fd_list) == 1) {
						struct fds_stub *fdss = malloc(sizeof(struct fds_stub));
						fdss->l = fd_list;
						list_push_back(&fds_ll, &fdss->fds_elem);
//						printf("fds_ll_size = %d\n", list_size(&fds_ll));
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

//		printf("fd's host process is %d\n", t->tid);

		if (t != NULL) {
			// thread_remove_fd
			thread_remove_fd(t, fd);

//			printf("thread %d fd_list size = %d\n", t->tid, list_size(&t->fd_list));

			// if thread_has empty fd_list ==> remove empty fd_list from fds_ll
			if (list_empty(&t->fd_list)) {
//				printf("fds_ll size = %d\n", list_size(&fds_ll));
				struct list_elem *fdlle = list_begin(&fds_ll);
				while(fdlle != list_end(&fds_ll)) {
					struct list_elem *tmpfdlle = fdlle;
					fdlle = list_next(fdlle);
					struct fds_stub *fdss = list_entry(tmpfdlle, struct fds_stub, fds_elem);
					if (fdss->l == &t->fd_list) {
//						printf("before remove from fds_ll\n");
						list_remove(tmpfdlle);
//						printf("fds_ll size = %d\n", list_size(&fds_ll));
						break;
					}
				}
			}


//			printf("thread %d fd_list size = %d\n", t->tid, list_size(&t->fd_list));
//			printf("fds_ll list size = %d\n", list_size(&fds_ll));


		}

//		printf("before close file\n");

		file_close(f);

//		printf("after close file\n");
		//to check the file needs to be removed?
		//if so remove it (removing file physically). if not, leave.

//		printf("removing list size = %d\n", list_size(&removing_list));

		struct list_elem *rle = list_begin(&removing_list);
		int referred_times_by_myself = 0;
		struct list_elem *tmprle = NULL;
		while(rle != list_end(&removing_list)) {
			tmprle = rle;
			rle = list_next(rle);
			struct removing_file *r_f = list_entry(tmprle, struct removing_file, r_elem);

//			printf("r_f->fd = %d, fd = %d, t->tid = %d\n",r_f->fd, fd, t->tid);
			if (r_f->fd == fd && t == r_f->t) {

				referred_times_by_myself++;
//				printf("referred_times_by_myself = %d\n", referred_times_by_myself);
				break;
			}
		}
		//if the referers only include thread itself,
		//it can remove it when closing it.
		if (referred_times_by_myself == 1 && tmprle != NULL) {
//			printf("go to remove tmprle for fd %d from removing list\n", list_entry(tmprle, struct removing_file, r_elem)->fd);
			list_remove(tmprle);
//			printf("after remove item removing list size = %d\n", list_size(&removing_list));
			bool can_be_removed = false;
			struct list_elem *rle_ = list_begin(&removing_list);
			int counter = 0;
			while(rle_ != list_end(&removing_list)) {
				struct list_elem *tmprle_= rle_;
				rle_ = list_next(rle_);
				struct removing_file *r_f_ = list_entry(tmprle_, struct removing_file, r_elem);
				if (r_f_->fd == -1) {
					counter++;
				} else {
					break;
				}
			}

			if (counter == list_size(&removing_list)) {
				can_be_removed = true;
			}

//			printf("can_be_removed = %d\n", can_be_removed);

			if (can_be_removed) {
				filesys_remove(list_entry(tmprle, struct removing_file, r_elem)->file_name);
			}
		}
	}
}

/* get pointer of file by fd
   returns NULL if can not find fd which equals to given value
   by Xiaoqi Cao*/
struct file* get_file_p(unsigned int fd) {

	if (fd == 1 || fd == 0) {
//		printf("fd = %d, can not get file structure pointer.\n", fd);
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

void release_resource() {
	struct thread *t = thread_current();
	//close all the files opened
	struct list_elem *open_fde = list_begin(&t->fd_list);
	while(open_fde != list_end(&t->fd_list)) {
		struct list_elem *tmp_open_fde = open_fde;
		open_fde = list_next(open_fde);
		list_remove(tmp_open_fde);
	}

	//remove item form removing list if it has
	struct list_elem *rle = list_begin(&removing_list);
	while(rle != list_end(&removing_list)) {
		struct list_elem *tmprle = rle;
		rle = list_next(rle);
		struct removing_file *r_f = list_entry(tmprle, struct removing_file, r_elem);
		if (r_f->t == t) {
			list_remove(tmprle);
		}
	}

	//set all its child processes to orphaned
	struct list_elem *ste = list_begin(&t->sub_threads);
	while(ste != list_end(&t->sub_threads)) {
		struct list_elem *tmp_ste = ste;
		ste = list_next(ste);
		struct thread *st = list_entry(tmp_ste, struct sub_thread, s_t_elem);
		st->pt = NULL;
	}

}

/* Get argument as a pointer
   It can be converted into char* or another pointer type.
   It at the same time does validation work to the user pointer
   by Xiaoqi Cao*/
void* get_arg_pointer(unsigned int index, void* stack_p) {
	if (index <=2 && stack_p != NULL) {

		void * user_pointer = *((void **)(stack_p + index * 4));
		if (check_user_pointer(user_pointer)) {
			return user_pointer;
		} else {

			//invalid pointer
			//release resources
//			release_resource();
			//terminate process
//			process_exit();
			exit(-1);
		}
	}

	return NULL;
}

/* Get argument as an integer
   by Xiaoqi Cao*/
int get_arg_integer(unsigned int index, void* stack_p) {
	if (index <=2 && stack_p != NULL) {
		int value = *((int *)(stack_p + 4 * index));
		return value;
	}
}

/* To find the file is referred by process.
   if a file is referred by other process,
   it can not be removed until all the references are clear.
   by Xiaoqi Cao*/
bool is_referred(char* file_name) {
	struct list_elem *le = list_begin(&fds_ll);
	bool referred = false;
	//list of list
	while(le != list_end(&fds_ll)) {
		struct list_elem *tmp = le;
		le = list_next(le);
		struct fds_stub *fdss = list_entry(tmp, struct fds_stub, fds_elem);
		if (fdss != NULL) {
			struct list * fd_list = fdss->l;
			if (fd_list != NULL) {
				struct list_elem *fd_le = list_begin(fd_list);
				//list of fd_elem
				while(fd_le != list_end(fd_list)) {
					struct list_elem *tmp_fd_le = fd_le;
					fd_le = list_next(fd_le);
					struct fd_elem *fde = list_entry(tmp_fd_le, struct fd_elem, fdl_elem);
					if (fde != NULL) {
//						printf("arg file_name: %s\n", file_name);
//						printf("fde->file_name: %s\n", fde->file_name);
						if (strcmp(file_name, fde->file_name) == 0) {
							//located fd structure of file_name
//							printf("file_name matched\n");
							struct thread *ct = thread_current();
							//add itself to removing list
							//the removing list stores the <file, fd, thread> tuple,
							//which indicates a certain thread wants to remove a certain file but it can not remove it right now,
							//because the file has been referred.
							//so when closing the file, close system call should check if the closing file
							//has been required to remove by some threads.
							//if so, and it has not other reference, remove it physically.
							//if so, but it has other reference, just close it.
							//if not, no matter referred by some threads or not,just close it.

							bool referred_by_c_t = false;

//							printf("ct->fd_list size = %d\n", list_size(&ct->fd_list));
							struct list_elem *cfle = list_begin(&ct->fd_list);
							struct list_elem *tmpcfle;
							while(cfle != list_end(&ct->fd_list)) {
								tmpcfle = cfle;
								cfle = list_next(cfle);
								struct fd_elem* cfd = list_entry(tmpcfle, struct fd_elem, fdl_elem);
//								printf("cfd->file_name = %s\n", cfd->file_name);
								if (strcmp(cfd->file_name, file_name) == 0) {
									if (!already_in_removing_list(ct, file_name, cfd->fd)) {
//										printf("not in removing list\n");
										struct removing_file *r_f = malloc(sizeof(struct removing_file));
										r_f->fd = list_entry(tmpcfle, struct fd_elem, fdl_elem)->fd;
										r_f->file_name = file_name;
										r_f->t = thread_current();
										list_push_back(&removing_list, &r_f->r_elem);
//										printf("removing list size = %d\n", list_size(&removing_list));
//										break;
									}
									referred_by_c_t = true;
								}
							}

							if (!referred_by_c_t) {
								if (!already_in_removing_list(ct, file_name, -1)) {
									struct removing_file *r_f = malloc(sizeof(struct removing_file));
									r_f->fd = -1;
									r_f->file_name = file_name;
									r_f->t = thread_current();
									list_push_back(&removing_list, &r_f->r_elem);
								}
							}

							referred = true;
							goto ret;
						}
					}
				}
			}
		}
	}

	ret:

//	printf("referred = %d\n", referred);

	return referred;
}

/* to find whether the removing file has been already in removing list.
   return true, if it has been already in it,
   return false, if it is not in it.
   by Xiaoqi Cao*/
bool already_in_removing_list(struct thread *t, char *file_name, int fd) {
	struct list_elem *rle = list_begin(&removing_list);
	while(rle != list_end(&removing_list)) {
		struct list_elem *tmprle = rle;
		rle = list_next(rle);
		struct removing_file *r_f = list_entry(tmprle, struct removing_file, r_elem);
		if (fd != -1) {
			if (r_f->fd == fd && strcmp(file_name, r_f->file_name) == 0 && t->tid == r_f->t->tid) {
				return true;
			}
		} else {
			if (strcmp(file_name, r_f->file_name) == 0 && t->tid == r_f->t->tid) {
				return true;
			}
		}
	}
	return false;
}


