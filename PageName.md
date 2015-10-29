#Project 2 Main Page.

### Introduction ###

The project 2 of OS lecture in UNI-SAARLAND SS09.

### News ###
**02-06-2009**

Implemented tell/seek/read/write/filesize/create/open and so on.
They are waiting for further tests.

Have some idea of wait and other functions.
<br />
1  add sub\_threads list to thread to hold the child processes' thread.
<br />
2  add some function in thread.h/thread.c to work as tools.
<br />
3  implement the logic required by document of project 2.

**01-06-2009**
Implementing file operation functions. Considering the wait function.

**29-05-2009**
The structure of stack for system call is like below:

HIGH.<br />
arg2.<br />
arg1.<br />
arg0.<br />
number(f->esp).<br />
LOW.

getting value of parameter i (i ~ 0,2) when this parameter slot is unused, you will get random value.

**28-05-2009**
Argument passing basically passed. Its correctness can be proofed by give echo different amount of parameters when running through command line. The output will be different when using 4 arguments and other amount of arguments.


### Working Status ###

**02-06-2009** Implemented tell/seek/read/write/filesize/create/open and so on. Have some idea of wait and other functions.

**29-05-2009** Refined system call dispatcher and explored the stack structure for system call calling.

**28-05-2009** Argument passing basically passed. But can not make hex\_dump() work normally.

**26-05-2009** Half coding finished about passing argument and system call handler.

### Details ###

Problem list:

**1** What is unmapped VM? How to determine whether a pointer is pointing to unmapped VM?

**2** Should we determine whether an argument's type is int or pointer just through the call number? is there any better way to determine the argument's type?

**3** How can make hex\_dump() work? It so far prints out same content no matter push anything into stack.

**4** The process has to maintain a list of file descriptors, do it mean the corresponding thread should contains a list of FD?

**5** Is there a easy way to associate the file structure and FD together and enable to finding a file structure by FD?

**6** For now, how does pintos manage its process id? or it never manages its process' ids?

**7** How to connect file(struct file/fd) and its name together? open-->>remove
### Messages ###

Format: Date -- Content.