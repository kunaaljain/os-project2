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
	  if (number == 0) {
	  } else if (number == 1) {
		  arg0 = ((int *)stack_p)+1;
	  } else if (number == 2) {
		  arg0 = ((int *)stack_p)+1;
		  arg1 = ((int *)stack_p)+2;
	  } else if (number == 3) {
		  arg0 = ((int *)stack_p)+1;
		  arg1 = ((int *)stack_p)+2;
		  arg2 = ((int *)stack_p)+3;
	  } else {
		  printf("invalid system call number: %d!\n" + number);
	  }
  }
//  thread_exit ();
}
