/*
 * newlib_stubs.c
 *
 *  Created on: 2 Nov 2010
 *      Author: nanoage.co.uk
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "serial.h"

#undef errno
extern int errno;

/*
 environ
 A pointer to a list of environment variables and their values.
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

//int _write(int file, char *ptr, int len);

void _exit(int status) {
	//printString("Inside _exit()");
    //_write(1, "exit", 4);
    while (1) {
        ;
    }
}

int _close(int file) {
	//printString("Inside _close()");
    return -1;
}
/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
int _execve(char *name, char **argv, char **env) {
	//printString("Inside _execve()");
    errno = ENOMEM;
    return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

int _fork() {
	//printString("Inside _fork()");
    errno = EAGAIN;
    return -1;
}
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat(int file, struct stat *st) {
	//printString("Inside _fstat()");
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

int _getpid() {
	//printString("Inside _getpid()");
    return 1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty(int file) {
	//printString("Inside _isatty()");
    switch (file){
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;
    default:
        //errno = ENOTTY;
        errno = EBADF;
        return 0;
    }
}


/*
 kill
 Send a signal. Minimal implementation:
 */
int _kill(int pid, int sig) {
	//printString("Inside _kill()");
    errno = EINVAL;
    return (-1);
}

/*
 link
 Establish a new name for an existing file. Minimal implementation:
 */

int _link(char *old, char *new) {
	//printString("Inside _link()");
    errno = EMLINK;
    return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek(int file, int ptr, int dir) {
	//printString("Inside _lseek()");
    return 0;
}

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
caddr_t _sbrk(int incr) {
	
    char * _heap_start=(char *) 0x10001000; // Defined by the linker via linker script
	char * _stack_end=(char *) 0x10001800;
    static char * heap_end=0;
    char *prev_heap_end;

	//printString("Inside _sbrk()");
	
    if (heap_end == 0) {
    	//printString("Initializing heap");
    	//printString("_heap_start:");
    	//PrintNumber((int)_heap_start, 16, 8);
    	//printString("_stack_end:");
    	//PrintNumber((int)_stack_end, 16, 8);
        heap_end = _heap_start;
    }
    prev_heap_end = heap_end;

	//char * stack = (char*) __get_MSP();
	char * stack = (char *)_stack_end;
	if (heap_end + incr >  stack)
	{
		printString("Heap and stack collision\n");
		//_write (STDERR_FILENO, "Heap and stack collision\n", 25);
		errno = ENOMEM;
		return  (caddr_t) -1;
		//abort ();
	}

    heap_end += incr;
	//printString("Leaving _sbrk()");
    return (caddr_t) prev_heap_end;
}

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */
int _read(int file, char *ptr, int len) {
    int n;
    int num = 0;

	//printString("Inside _read()");

    switch (file) {
    case STDIN_FILENO:
        for (n = 0; n < len; n++) {
            char c = egetc(); //getchar from uart
            *ptr++ = c;
            num++;
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return num;
}

/*
 stat
 Status of a file (by name). Minimal implementation:
 int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */

int _stat(const char *filepath, struct stat *st) {
	//printString("Inside _stat()");
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 times
 Timing information for current process. Minimal implementation:
 */

clock_t _times(struct tms *buf) {
	//printString("Inside _times()");
    return -1;
}

/*
 unlink
 Remove a file's directory entry. Minimal implementation:
 */
int _unlink(char *name) {
	//printString("Inside _unlink()");
    errno = ENOENT;
    return -1;
}

/*
 wait
 Wait for a child process. Minimal implementation:
 */
int _wait(int *status) {
	//printString("Inside _wait()");
    errno = ECHILD;
    return -1;
}

/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
int _write(int file, char *ptr, int len) {
    int n;

	//printString("Inside _write()");
    switch (file) {
    case STDOUT_FILENO: /*stdout*/
        for (n = 0; n < len; n++) {
        	eputc(*ptr++); // Put char in uart.
        }
        break;
    case STDERR_FILENO: /* stderr */
        for (n = 0; n < len; n++) {
        	eputc(*ptr++); // Put char in uart.
        }
        break;
    default:
		//printString("Leaving _write() at default: label");
        errno = EBADF;
        return -1;
    }
	//printString("Leaving _write()");

    return len;
}