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

#include <stdint.h>
#include <stdio.h>
#include "block.h"
#include "partition.h"

int read_partition_table(uint8_t *mbr, blockno_t volume_size, struct partition **retlist) {
  static struct partition retval[4];
  mbr_entry *entry = (mbr_entry *)(mbr + PARTITION0START);
  int i, j=0;
//   printf("Start: 0, Length: %u\r\n", (unsigned int)volume_size);
  for(i=0;i<4;i++) {
    // validate this partition entry
//     printf("Start: %u, Length: %u, Type: %02x\r\n", (unsigned int)entry->lba_start, (unsigned int)entry->length, (unsigned int)entry->type);
    if((entry->lba_start < volume_size) && 
       ((entry->lba_start + entry->length) <= volume_size) &&
       (entry->lba_start > 0) &&
       (entry->length > 0)) {
      // the partion is non-zero length and smaller than the disk.
      // no guarantee the partition isn't overlapping another
      retval[j].start = entry->lba_start;
      retval[j].type = entry->type;
      retval[j].length = entry->length;
      j++;
    }
    entry += 1;
  }
  
  *retlist = retval;
  return j;
}
