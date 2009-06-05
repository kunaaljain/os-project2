/* mkdir.c

   Creates a directory. */

#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
//  if (argc != 2)
//    {
//      printf ("usage: %s DIRECTORY\n", argv[0]);
//      return EXIT_FAILURE;
//    }
//
//  if (!mkdir (argv[1]))
//    {
//      printf ("%s: mkdir failed\n", argv[1]);
//      return EXIT_FAILURE;
//    }

  printf("mkdir\n");

  printf("mkdir.argc = %d\n", argc);

  int i = 0;
  while(i < argc) {
	  printf("mkdir.argv %d = %s\n", i, argv[i]);
	  i++;
  }

  while(true) {
	  i++;
	  if (i == 1000000) {
		  break;
	  }
  }

  return EXIT_SUCCESS;
}
