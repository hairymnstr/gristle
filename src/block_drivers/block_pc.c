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
#include <stdint.h>
#include "hash.h"
#include "../block.h"
#include "block_pc.h"

uint64_t block_fs_size=0;
uint8_t *blocks = NULL;
int block_ro;
static const char *image_name = NULL;

void block_pc_set_image_name(const char * const filename) {
    image_name = filename;
    return;
}

int block_init() {
  FILE *block_fp;
  if(!(block_fp = fopen(image_name, "rb"))) {
    return -1;
  }
  fseek(block_fp, 0, SEEK_END);
  block_fs_size = ftell(block_fp);
  if(!(block_fs_size < 2048L * 1024L * 1024L)) {
    fprintf(stderr, "Aborting, image is over 2GB.\n");
    fclose(block_fp);
    return -1;
  }
  if((blocks = (uint8_t *)malloc(sizeof(uint8_t) * block_fs_size)) == NULL) {
      fprintf(stderr, "Failed to malloc() enough memory for the filesystem.\r\n");
      fclose(block_fp);
      return -1;
  }
  
  fseek(block_fp, 0, SEEK_SET);
  if(fread(blocks, 1, block_fs_size, block_fp) < block_fs_size) {
      free(blocks);
      fprintf(stderr, "Failed to read the filesystem image.\n");
      return -1;
  }
  
  fclose(block_fp);
  return 0;
}

int block_halt() {
    if(blocks) {
        free(blocks);
    }
    return 0;
}

int block_read(blockno_t block, void *buffer) {
//   printf("block read from %x\n", block * BLOCK_SIZE);
  /* we can't allow the file to grow (wouldn't happen with a physical volume) so need to check
     first because in rb+ file will grow if we seek past the end. */
  if((block+1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
//   fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
//   if(fread(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
//     return -1;
//   }
//   fflush(block_fp);
  memcpy(buffer, blocks + block * BLOCK_SIZE, BLOCK_SIZE);
  return 0;
}

int block_write(blockno_t block, void *buffer) {
//   printf("block write at %x\n", block * BLOCK_SIZE);
  if((block + 1) * BLOCK_SIZE - 1 > block_fs_size) {
    return -1;
  }
  
//   fseek(block_fp, block * BLOCK_SIZE, SEEK_SET);
//   if(fwrite(buffer, 1, BLOCK_SIZE, block_fp) < BLOCK_SIZE) {
//     return -1;
//   }
//   fflush(block_fp);
  memcpy(blocks + block * BLOCK_SIZE, buffer, BLOCK_SIZE);
  return 0;
}

blockno_t block_get_volume_size() {
  return block_fs_size / BLOCK_SIZE;
}

int block_get_block_size() {
  return BLOCK_SIZE;
}

int block_get_device_read_only() {
  return block_ro;
}

void block_pc_set_ro() {
  block_ro = -1;
}

void block_pc_set_rw() {
  block_ro = 0;
}

int block_pc_snapshot(const char *filename, uint64_t start, uint64_t len) {
  FILE *fp;
  
  if(!(fp = fopen(filename, "wb"))) {
    return -1;
  }
  
  fwrite(blocks + start, 1, len, fp);
  
  fclose(fp);
  
  return 0;
}

int block_pc_snapshot_all(const char *filename) {
  return block_pc_snapshot(filename, 0, block_fs_size);
}

int block_pc_hash(uint64_t start, uint64_t len, uint8_t hash[16]) {
  return md5_memory(&blocks[start], len , hash);
}

int block_pc_hash_all(uint8_t hash[16]) {
  return md5_memory(blocks, block_fs_size, hash);
}
