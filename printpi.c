#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

/*
 * Opens up the file pi50.4.bin and prints out the digits of pi to stdout
 * Written in three minutes.  Please don't blame me if it doesn't work for
 * you, or work on your system.  Tested only on NetBSD and FreeBSD
 *
 * It's ugly and stupid, but it works. :)
 */

#define FILE_SIZE 25000000 /* twenty five million bytes = 50M digits of pi */

int main() {
  int i;
  int c;
  unsigned char *buf;
  int file;

  file = open("pi50.4.bin", O_RDONLY);
  if (file == -1) {
        perror("Could not open pi file\n");
        exit(-1);
  }
  buf = mmap(NULL, FILE_SIZE, PROT_READ, MAP_SHARED, file, 0);
  for (i = 0; i < FILE_SIZE; i++) {
    printf("%d%d", (buf[i]&0xf0)>>4, buf[i] & 0xf);
  }
  munmap(buf, FILE_SIZE);
  close(file);
}
