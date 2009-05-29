#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

static void * stack_p;
static void syscall_handler(struct intr_frame *);
bool check_user_pointer(void*);
int syscall_execute(int ,struct intr_frame *);
void* get_arg_pointer(unsigned int);
int get_arg_integer(unsigned int);
void syscall_init(void) {
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
	stack_p = NULL;
}

/* dispatch the syscall request to certain syscall*/
/* by Xiaoqi Cao*/
static void syscall_handler(struct intr_frame *f) {
	printf("system call!\n");
	stack_p = f->esp;
	//get stack pointer, which is pointing onto top of stack
	if (stack_p != NULL) {
		//get system call number

		int number = *((int *) stack_p);

		int retval = syscall_execute(number,f);
	}
	thread_exit();
}

/* check the validation of pointers passed from user */
/* by Xiaoqi Cao*/
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
   register by syscall's return value.*/
/* by Xiaoqi Cao*/
int syscall_execute(int number, struct intr_frame *f) {
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
		if (p != NULL) {
			bool suc = false;
			suc = create(p, n);
			f->eax = (uint32_t)suc;
		}
		break;
	case SYS_REMOVE:
		p = (char *)get_arg_pointer(0);
		if (p != NULL) {
			bool suc;
			suc = remove(p);
			f->eax = suc;
		}
		break;
	case SYS_OPEN:
		p = (char *)get_arg_pointer(0);
		if (p != NULL) {
			int fd = open(p);
			f->eax = fd;
		}
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
		f->eax = write(n,p,m);
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
		printf("Unsupported system call number: %d!\n" + number);
		thread_exit();
		break;
	}
}


/* implemented system calls for project 2*/
/* by Xiaoqi Cao*/
void halt () {
	shutdown_power_off();
}

void exit (int status) {

}

pid_t exec (const char *file) {
	return 0;
}

int wait (pid_t pidt) {
	return 0;
}


bool create (const char *file, unsigned initial_size) {
	return false;
}

bool remove (const char *file) {
	return false;
}

int open (const char *file) {
	return 0;
}

int filesize (int fd) {
	return 0;
}

int read (int fd, void *buffer, unsigned length) {
	return 0;
}

int write (int fd, const void *buffer, unsigned length) {
	return 0;
}

void seek (int fd, unsigned position) {

}

unsigned tell (int fd) {
	return 0;
}

void close (int fd) {

}

/* Get argument as a pointer
   It can be converted into char* or another pointer type.
   It at the same time does validation work to the user pointer*/
/* by Xiaoqi Cao*/
void* get_arg_pointer(unsigned int index) {
	if (index >=0 && index <=2 && stack_p != NULL) {

		void * user_pointer = *((void **)(stack_p + index * 4));
		if (check_user_pointer(user_pointer)) {
			return user_pointer;
		} else {
			//invalid pointer
			//TODO release resources

			//terminate process
			process_exit();
		}
	}
}

/* Get argument as an integer*/
/* by Xiaoqi Cao*/
int get_arg_integer(unsigned int index) {
	if (index >=0 && index <=2 && stack_p != NULL) {
		return *((int*)(stack_p + index * 4));
	}
}
