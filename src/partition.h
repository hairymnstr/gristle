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

#ifndef PARTITION_H
#define PARTITION_H 1

#define PARTITION0START 0x1BE
#define PARTITION1START 0x1CE
#define PARTITION2START 0x1DE
#define PARTITION3START 0x1EE

#define PART_TYPE_FAT16 0x06
#define PART_TYPE_FAT32 0x0B

// an MBR entry structure, because this is based on actual on-disk structure, it must be packed
typedef struct {
  uint8_t  bootable;
  uint8_t  chs[3];
  uint8_t  type;
  uint8_t  chs_end[3];
  uint32_t lba_start;
  uint32_t length;
} __attribute__((__packed__)) mbr_entry;

struct partition {
  blockno_t start;
  blockno_t length;
  uint8_t type;
};

int read_partition_table(uint8_t *mbr, blockno_t volume_size, struct partition **retlist);

#endif /* ifndef PARTITION_H */
