#include <stdio.h>
#include <lib/user/syscall.h>
#include "tests/lib.c"

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
//	if (argc == 4) {
//		int i;
//		for (i = 0; i < argc; i++)
//			printf("%s", argv[i]);
//	} else if (argc == 2) {
//		seek(3, 10);
//	} else {

/////////////////////////////////////
//	halt (); successfully powered off


/////////////////////////////////////
//	exit (57);

/////////////////////////////////////
//  create-null // pass
//	create (NULL, 0);
//  create-normal //pass
//	create ("quux.dat", 0);
//	create-long //pass
//	static char name[512];
//    memset (name, 'x', sizeof name);
//    name[sizeof name - 1] = '\0';
//    create (name, 0);
//  create-exists //pass
//    create ("quux.dat", 0);
//    create ("warble.dat", 0);
//    create ("quux.dat", 0);
//    create ("baffle.dat", 0);
//    create ("warble.dat", 0);
//  create-empty //pass
//    create ("", 0);
//  create-bound
//    create (copy_string_across_boundary ("quux.dat"), 0));
//  create-bad-ptr
//    msg ("create(0x20101234): %d", create ((char *) 0x20101234, 0));


//////////////////////////////////////
//	remove pass

//	create ("quux.dat", 0);
//	remove("quux.dat");
//	remove("quux.dat");
//	create("quux.dat", 0);
//	remove("quux.dat");


//////////////////////////////////////
//	open
//	open twice
//	create("sample.txt", 10);
//	int h1 = open ("sample.txt");
//	int h2 = open("sample.txt");
//	printf("h1 = %d, h2 = %d\n", h1, h2);

//	open NULL
//	open (NULL);

//	open normal
//	int handle1 = open ("sample.txt");
//	int handle2 = open ("sample.txt");
//
//	char buffer1[3] = {'a','b','c'};
//	char buffer2[3] = {'d','e','f'};
//	char buffer3[3] = {'g','h','i'};
//	char buffer4[3] = {'j','k','l'};
//
//	printf("write %d bytes", write(handle1, (void *)buffer1, 3));
//	printf("write %d bytes", write(handle1, (void *)buffer2, 3));
//	printf("write %d bytes", write(handle1, (void *)buffer3, 3));
//	printf("write %d bytes", write(handle2, (void *)buffer4, 3));
//
//	printf("file size = %d\n",filesize(handle1));
//	printf("file size = %d\n",filesize(handle2));
//
//	char buffer5[15];
//
//	printf("next read or written byte position: %d\n", tell(handle1));
//
//	seek(handle1, 0);
//
//	int readbytes = read(handle1, buffer5, 15);
//
//	printf("read %d bytes, that is: %s\n", readbytes, buffer5);

//	open missing
//	handle = open ("no-such-file");
//	printf("handle = %d\n", handle);

//	open empty
//	handle = open ("");
//	printf("handle = %d\n", handle);

//	open boundary
//	open (copy_string_across_boundary ("sample.txt"));
//	open bad ptr
//	open ((char *) 0x20101234);
//	}


//	args.c pass

//	int i;
//	test_name = "args";
//
//	  msg ("begin");
//	  msg ("argc = %d", argc);
//	  for (i = 0; i <= argc; i++)
//	    if (argv[i] != NULL)
//	      msg ("argv[%d] = '%s'", i, argv[i]);
//	    else
//	      msg ("argv[%d] = null", i);
//	  msg ("end");

//	child-bad.c
//	asm volatile ("movl $0x20101234, %esp; int $0x30");
//	  fail ("should have exited with -1");

//	child-close.c pass

//	  msg ("begin");
//	  if (!isdigit (*argv[1]))
//	    fail ("bad command-line arguments");
//	  close (atoi (argv[1]));
//	  msg ("end");


//	close (0x20101234);


	//wait/exec

	char file_name1[11] = {'m','k','d','i','r',' ','1',' ','p','q','\0'};
	char file_name2[3] = {'r','m','\0'};

	int mkpid = exec(file_name1);


//	  int i = 0;
//
//	  int j = 0;
//
//	  int k = 0;
//
//
//	while (k < 9999) {
//		j = 0;
//		while (j < 9999) {
//			i = 0;
//			while (i < 9999) {
//				i++;
//			}
//			j++;
//		}
//		k++;
//	}

//	int exit_code_mkdir = wait(mkpid);
//
//	printf("waiting mkdir with exit code = %d\n", exit_code_mkdir);
//	exec(file_name2);


  return 18;
}
