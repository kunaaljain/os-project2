/* rm.c

   Removes files specified on command line. */

#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
//  bool success = true;
//  int i;
//
//  for (i = 1; i < argc; i++)
//    if (!remove (argv[i]))
//      {
//        printf ("%s: remove failed\n", argv[i]);
//        success = false;
//      }
//  return success ? EXIT_SUCCESS : EXIT_FAILURE;

	printf("rm\n");

	  printf("rm.argc = %d", argc);

	  int i = 0;
	  while(i<argc) {
		  printf("rm.argv %d = %s\n", i, *argv[i]);
		  i++;
	  }

	return 0;
}
