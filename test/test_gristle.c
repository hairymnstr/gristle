/*
 * Copyright (c) 2012-2013, Nathan Dumont
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of 
 *    conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of 
 *    conditions and the following disclaimer in the documentation and/or other materials 
 *    provided with the distribution.
 * 3. Neither the name of the author nor the names of any contributors may be used to endorse or
 *    promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Gristle FAT16/32 compatible filesystem driver.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "../src/gristle.h"
#include "../src/block.h"
#include "../src/block_drivers/block_pc.h"
#include "../src/partition.h"

/**************************************************************
 * Filesystem image structure:
 * 
 * root /+-> ROFILE.TXT
 *       +-> DIR1
 *       +-> NORMAL.TXT
 *************************************************************/

/**************************************************************
 * Error codes to test for on fat_open();
 * 
 * [EACCES] - write on a read only file
 * [EISDIR] - write access to a directory
 * [ENAMETOOLONG] - file path is too long
 * [ENFILE] - too many files are open
 * [ENOENT] - no file with that name or empty filename
 * [ENOSPC] - write to a full volume
 * [ENOTDIR] - part of subpath is not a directory but a file
 * [EROFS] - write access to file on read-only filesystem
 * [EINVAL] - mode is not valid
 **************************************************************/
int test_open(int p) {
  int i;
  int v;
  const char *desc[] = {"Test O_WRONLY on a read only file.",
                        "Test O_RDWR on a read only file.",
                        "Test O_RDONLY on a read only file.",
                        "Test O_WRONLY on a directory.",
                        "Test O_RDWR on a directory.",
                        "Test O_RDONLY on a directory.",
                        "Test O_WRONLY on a missing file.",
                        "Test O_RDWR on a missing file.",
                        "Test O_RDONLY on a missing file.",
                        "Test O_WRONLY on a path with file as non terminal member.",
                        "Test O_RDWR on a path with file as non terminal member.",
                        "Test O_RDONLY on a path with a file as non terminal member.",
  };
  const char *filename[] = {"/ROFILE.TXT",
                            "/ROFILE.TXT",
                            "/ROFILE.TXT",
                            "/DIR1",
                            "/DIR1",
                            "/DIR1",
                            "/MISSING.TXT",
                            "/MISSING.TXT",
                            "/MISSING.TXT",
                            "/ROFILE.TXT/NONE.TXT",
                            "/ROFILE.TXT/NONE.TXT",
                            "/ROFILE.TXT/NONE.TXT",
  };
  const int flags[] = {O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
                       O_WRONLY,
                       O_RDWR,
                       O_RDONLY,
  };
  const int result[] = {EACCES,
                        EACCES,
                        0,
                        EISDIR,
                        EISDIR,
                        0,
                        ENOENT,
                        ENOENT,
                        ENOENT,
                        ENOTDIR,
                        ENOTDIR,
                        ENOTDIR,
  };
  const int cases = 12;
  
  int rerrno;
  
  for(i=0;i<cases;i++) {
    printf("[%4d] Testing %s", p++, desc[i]);
    v = fat_open(filename[i], flags[i], 0, &rerrno);
    if(rerrno == result[i]) {
      printf("  [ ok ]\n");
    } else {
      printf("  [fail]\n  expected (%d) %s\n  got (%d) %s\n", result[i], strerror(result[i]), rerrno, strerror(-rerrno));
    }
    if(v > -1) {
      fat_close(v, &rerrno);
      if(rerrno != 0) {
        printf("fat_close returned %d (%s)\n", rerrno, strerror(rerrno));
      }
      
    }
  }
  return p;
}

extern struct fat_info fatfs;

int main(int argc, char *argv[]) {
  int p = 0;
  int rerrno = 0;
  FILE *fp;
  int len;
  uint8_t *d;
  int result;
  int parts;
  uint8_t temp[512];
  struct partition *part_list;
  
//   int v;
  printf("Running FAT tests...\n\n");
  printf("[%4d] start block device emulation...", p++);
  printf("   %d\n", block_init());
  
  printf("[%4d] mount filesystem, FAT32", p++);
  
  result = fat_mount(0, block_get_volume_size(), PART_TYPE_FAT32);

  printf("   %d\n", result);

  if(result != 0) {
    // mounting failed.
    // try listing the partitions
    block_read(0, temp);
    parts = read_partition_table(temp, block_get_volume_size(), &part_list);
    
    printf("Found %d valid partitions.\n", parts);
    
    if(parts > 0) {
      result = fat_mount(part_list[0].start, part_list[0].length, part_list[0].type);
    }
    if(result != 0) {
      printf("Mount failed\n");
      exit(-2);
    }
    
  }
  
  printf("Part type = %02X\n", fatfs.type);
  //   p = test_open(p);

  int fd;
  
//   printf("Open\n");
//   fd = fat_open("/newfile.txt", O_WRONLY | O_CREAT, 0777, &rerrno);
//   printf("fd = %d, errno=%d (%s)\n", fd, rerrno, strerror(rerrno));
//   if(fd > -1) {
//     printf("Write\n");
//     fat_write(fd, "Hello World\n", 12, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//     printf("Close\n");
//     fat_close(fd, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//   }
  
//   printf("Open\n");
//   fd = fat_open("/newfile.png", O_WRONLY | O_CREAT, 0777, &rerrno);
//   printf("fd = %d, errno=%d (%s)\n", fd, rerrno, strerror(rerrno));
//   if(fd > -1) {
//     fp = fopen("gowrong_draft1.png", "rb");
//     fseek(fp, 0, SEEK_END);
//     len = ftell(fp);
//     d = malloc(len);
//     fseek(fp, 0, SEEK_SET);
//     fread(d, 1, len, fp);
//     fclose(fp);
//     printf("Write PNG\n");
//     fat_write(fd, d, len, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//     printf("Close\n");
//     fat_close(fd, &rerrno);
//     printf("errno=%d (%s)\n", rerrno, strerror(rerrno));
//   }
  
  printf("errno = (%d) %s\n", rerrno, strerror(rerrno));
  result = fat_mkdir("/foo", 0777, &rerrno);
  printf("mkdir /foo: %d (%d) %s\n", result, rerrno, strerror(rerrno));
  
  result = fat_mkdir("/foo/bar", 0777, &rerrno);
  printf("mkdir /foo/bar: %d (%d) %s\n", result, rerrno, strerror(rerrno));
  
  result = fat_rmdir("/foo/bar", &rerrno);
  printf("rmdir /foo/bar: %d (%d) %s\n", result, rerrno, strerror(rerrno));
  
  result = fat_rmdir("/foo", &rerrno);
  printf("rmdir /foo: %d (%d) %s\n", result, rerrno, strerror(rerrno));
  
  block_pc_snapshot_all("writenfs.img");
  exit(0);
}
