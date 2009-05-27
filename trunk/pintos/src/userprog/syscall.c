#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  printf ("system call!\n");
  void * stack_p = f->esp;
  //get stack pointer, which is pointing onto top of stack
  if (stack_p != NULL) {
	  //get system call number
	  int number = *((int *)stack_p);
	  void* arg0;
	  void* arg1;
	  void* arg2;
	  int retval;
	  if (number == SYS_HALT) {
		  halt();
	  } else if (number == SYS_EXIT) {
		  exit()
	  } else if (number == SYS_EXEC) {

	  } else if (number == SYS_WAIT) {

	  } else if (number == SYS_CREATE) {

	  } else if (number == SYS_REMOVE) {

	  } else if (number == SYS_OPEN) {

	  } else if (number == SYS_FILESIZE) {

	  } else if (number == SYS_READ) {

	  } else if (number == SYS_WRITE) {

	  } else if (number == SYS_SEEK) {

	  } else if (number == SYS_TELL) {

	  } else if (number == SYS_CLOSE) {

	  } else {
		  printf("Unsupported system call number: %d!\n" + number);
		  thread_exit();
	  }

//	  else if (number == 1) {
//		  arg0 = ((int *)stack_p)+1;
//	  } else if (number == 2) {
//		  arg0 = ((int *)stack_p)+1;
//		  arg1 = ((int *)stack_p)+2;
//	  } else if (number == 3) {
//		  arg0 = ((int *)stack_p)+1;
//		  arg1 = ((int *)stack_p)+2;
//		  arg2 = ((int *)stack_p)+3;
//	  } else {
//		  printf("invalid system call number: %d!\n" + number);
//	  }
  }
//  thread_exit ();
}

//TODO add function to check the validation of pointers passed from user.

bool check_user_pointer(void* p) {
	return false;
}

static void halt(void) {

}
