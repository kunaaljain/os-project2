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


  long i = 0;
//  while(i < argc) {
//	  printf("mkdir.argv %d = %s\n", i, argv[i]);
//	  i++;
//  }

  long j = 0;

  while (j < 99999999) {
	i = 0;
    while (i < 999999999) {
		i++;
	}
    j++;
  }
  return 12;

//  return EXIT_SUCCESS;
}

