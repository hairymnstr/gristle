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

#ifndef GRISTLE_H
#define GRISTLE_H 1

#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include "block.h"
#include "dirent.h"

#define GRISTLE_BAD_PATH 255

#define MAX_OPEN_FILES 4
#define MAX_PATH_LEN 256

#define FAT_ERROR_CLUSTER 1
#define FAT_END_OF_FILE 2

/* FAT file attribute bit masks */
#define FAT_ATT_RO  0x01
#define FAT_ATT_HID 0x02
#define FAT_ATT_SYS 0x04
#define FAT_ATT_VOL 0x08
#define FAT_ATT_SUBDIR 0x10
#define FAT_ATT_ARC 0x20
#define FAT_ATT_DEV 0x40

struct fat_info {
  uint8_t   read_only;
  uint8_t   fat_entry_len;
  uint32_t  end_cluster_marker;
  uint8_t   sectors_per_cluster;
  uint32_t  cluster0;
  uint32_t  active_fat_start;
  uint32_t  sectors_per_fat;
  uint32_t  root_len;
  uint32_t  root_start;
  uint32_t  root_cluster;
  uint8_t   type;               // type of filesystem (FAT16 or FAT32)
  blockno_t part_start;         // start of partition containing filesystem
  uint32_t  total_sectors;
  uint8_t   sysbuf[512];
};

typedef struct {
  uint8_t   jump[3];
  char      name[8];
  uint16_t  sector_size;
  uint8_t   cluster_size;
  uint16_t  reserved_sectors;
  uint8_t   num_fats;
  uint16_t  root_entries;
  uint16_t  total_sectors;
  uint8_t   media_descriptor;
  uint16_t  sectors_per_fat;
  uint16_t  sectors_per_track;
  uint16_t  number_of_heads;
  uint32_t  partition_start;
  uint32_t  big_total_sectors;
  uint8_t   drive_number;
  uint8_t   current_head;
  uint8_t   boot_sig;
  uint32_t  volume_id;
  char      volume_label[11];
  char      fs_label[8];
} __attribute__((__packed__)) boot_sector_fat16;

typedef struct {
  uint8_t   jump[3];                   /*    0 */
  char      name[8];                   /*    3 */
  uint16_t  sector_size;               /*    B */
  uint8_t   cluster_size;              /*    D */
  uint16_t  reserved_sectors;          /*    E */
  uint8_t   num_fats;                  /*   10 */
  uint16_t  root_entries;              /*   11 */
  uint16_t  total_sectors;             /*   13 */
  uint8_t   media_descriptor;          /*   15 */
  uint16_t  short_sectors_per_fat;     /*   16 */
  uint16_t  sectors_per_track;         /*   18 */
  uint16_t  number_of_heads;           /*   1A */
  uint32_t  partition_start;           /*   1C */
  uint32_t  big_total_sectors;         /*   20 */
  uint32_t  sectors_per_fat;           /*   24 */
  uint16_t  fat_flags;                 /*   28 */
  uint16_t  version;                   /*   2A */
  uint32_t  root_start;                /*   2C */
  uint16_t  fs_info_start;             /*   30 */
  uint16_t  boot_copy;                 /*   32 */
  char      reserved[12];              /*   34 */
  uint8_t   drive_number;              /*   40 */
  uint8_t   current_head;              /*   41 */
  uint8_t   boot_sig;                  /*   42 */
  uint32_t  volume_id;                 /*   43 */
  char      volume_label[11];          /*   47 */
  char      fs_label[8];               /*   52 */
} __attribute__((__packed__)) boot_sector_fat32;

#define FS_INFO_SIG1 0x0000
#define FS_INFO_SIG2 0x01E4
#define FREE_CLUSTERS 0x01E8
#define LAST_ALLOCATED 0x01EC

typedef struct {
  char      filename[8];
  char      extension[3];
  uint8_t   attributes;
  uint8_t   reserved;
  uint8_t   create_time_fine;
  uint16_t  create_time;
  
  uint16_t  create_date;
  uint16_t  access_date;
  uint16_t  high_first_cluster;
  uint16_t  modified_time;
  uint16_t  modified_date;
  uint16_t  first_cluster;
  uint32_t  size;
} __attribute__((__packed__)) direntS;

typedef struct {
  uint8_t   flags;
  uint8_t   buffer[512];
  uint32_t  sector;
  uint32_t  cluster;
  uint8_t   sectors_left;
  uint16_t  cursor;
  uint8_t   error;
  char      filename[8];
  char      extension[3];
  uint8_t   attributes;
  size_t    size;
  uint32_t  full_first_cluster;
  uint32_t  entry_sector;
  uint8_t   entry_number;
  uint32_t  parent_cluster;
  uint32_t  file_sector;
  time_t    created;
  time_t    modified;
  time_t    accessed;
} FileS;

// flag values for FileS
#define FAT_FLAG_OPEN 1
#define FAT_FLAG_READ 2
#define FAT_FLAG_WRITE 4
#define FAT_FLAG_APPEND 8
#define FAT_FLAG_DIRTY 16
#define FAT_FLAG_FS_DIRTY 32

#define FAT_INTERNAL_CALL 4242

// int sdfat_lookup_path(int, const char *);
// int sdfat_next_sector(int fd);

int str_to_fatname(char *url, char *dosname);

int fat_mount(blockno_t start, blockno_t volume_size, uint8_t part_type_hint);

/**
 * \brief basic open a file function
 * 
 * Conforms to the IEEE standard open function with the addition of a return error number parameter.
 * Will open a file according to the flags and mode using given name.  This function doesn't set
 * the global errno parameter, it writes any error code to the integer pointer parameter.  This 
 * makes the function potentially thread safe although it hasn't been fully tested.
 * 
 * \param name is the file path/name to be opened
 * \param flags is a bitwise OR of flags from fcntl.h including read/write/create/append etc.
 * \param mode is the permissions setting for creating the file, largely ignored on FAT
 * \param rerrno if there is an error the error code will be written to the integer pointed to by
 * errno
 * \returns -1 on error or a file number for the opened file.
 **/
int fat_open(const char *name, int flags, int mode, int *rerrno);

int fat_close(int fd, int *rerrno);
int fat_read(int, void *, size_t, int *);
int fat_write(int, const void *, size_t, int *);
int fat_fstat(int, struct stat *, int *);
int fat_lseek(int, int, int, int *);
int fat_get_next_dirent(int, struct dirent *, int *rerrno);

int fat_unlink(const char *path, int *rerrno);
int fat_rmdir(const char *path, int *rerrno);
int fat_mkdir(const char *path, int mode, int *rerrno);

#endif /* ifndef GRISTLE_H */
