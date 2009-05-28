#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
//  int i;
//
//  for (i = 0; i < argc; i++)
//    printf ("%s ", argv[i]);
//  printf ("\n");

//  printf("argc = %d\n", argc);

	//in command line, input "echo xyz 123 wer" (4 tokens) will lead to system call SYS_WRITE
	//input "echo xyz 123 wer hj" (5 tokens) will lead to system call SYS_EXIT
	//there will be different system call number which output onto console
	//this can proof that the stack pushing works!
	if (argc == 4) {
		printf("4");//call syscall: SYS_WRITE
	}

  return EXIT_SUCCESS;
}
