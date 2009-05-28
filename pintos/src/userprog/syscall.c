#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"
#include "threads/vaddr.h"

static void syscall_handler(struct intr_frame *);
bool check_user_pointer(void*);
int syscall_execute(int);
void syscall_init(void) {
	intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* dispatch the syscall request to certain syscall*/
/* by Xiaoqi Cao*/
static void syscall_handler(struct intr_frame *f UNUSED) {
	printf("system call!\n");
	void * stack_p = f->esp;
	//get stack pointer, which is pointing onto top of stack
	if (stack_p != NULL) {
		//get system call number

		hex_dump(stack_p - 32, stack_p + 32, 64, true);

		int number = *((int *) stack_p);

		int retval = syscall_execute(number);
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
	return err;
}

int syscall_execute(int number) {
	printf("syscall number = %d\n", number);
	void* arg0;
	void* arg1;
	void* arg2;
	int retval;

	if (check_user_pointer(arg0) && check_user_pointer(arg1)
			&& check_user_pointer(arg2)) {
		switch (number) {
		case SYS_HALT:
		case SYS_EXIT:
		case SYS_EXEC:
		case SYS_WAIT:
		case SYS_CREATE:
		case SYS_REMOVE:
		case SYS_OPEN:
		case SYS_FILESIZE:
		case SYS_READ:
		case SYS_WRITE:
		case SYS_SEEK:
		case SYS_TELL:
		case SYS_CLOSE:
		default:
		}
	}
	//invalid pointer
	else {
		//TODO release resources

		//thread exit;
		thread_exit();
	}

//	if (number == SYS_HALT) {
//
//	} else if (number == SYS_EXIT) {
//
//	} else if (number == SYS_EXEC) {
//
//	} else if (number == SYS_WAIT) {
//
//	} else if (number == SYS_CREATE) {
//
//	} else if (number == SYS_REMOVE) {
//
//	} else if (number == SYS_OPEN) {
//
//	} else if (number == SYS_FILESIZE) {
//
//	} else if (number == SYS_READ) {
//
//	} else if (number == SYS_WRITE) {
//
//	} else if (number == SYS_SEEK) {
//
//	} else if (number == SYS_TELL) {
//
//	} else if (number == SYS_CLOSE) {
//
//	} else {
//		printf("Unsupported system call number: %d!\n" + number);
//		thread_exit();
//	}

}
