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

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "dirent.h"
#include <errno.h>
#include "block.h"
#include "partition.h"
#include "embext.h"

int ext2_lookup_path(int fd, const char *path, int *rerrno) {

  return 0;
}

int ext2_select_inode(int fd, int inode, struct ext2context *context) {
  struct inode *in;
  struct block_group_descriptor *block_table;
  uint32_t inode_block;
  uint32_t block_group = (inode - 1) / context->superblock.s_inodes_per_group;
  uint32_t inode_index = (inode - 1) % context->superblock.s_inodes_per_group;
  
  // now load the block group descriptor for that block group
  uint32_t bg_block = 1;
  if(context->superblock.s_log_block_size == 0) {
    bg_block++;
  }
  
  bg_block <<= (context->superblock.s_log_block_size + 1);
  
  bg_block += ((block_group * 32) / 512);
  
  block_read(bg_block + context->part_start, context->sysbuf);
  
  block_table = (struct block_group_descriptor *)&context->sysbuf[(block_group * 32) & 0x1ff];
  
  inode_block = block_table->bg_inode_table;
  
  printf("Inode block %d\r\n", inode_block);
  
  inode_block <<= (context->superblock.s_log_block_size + 1);
  
  inode_block += (inode_index >> 2);
  printf("loc %d\r\n", inode_block);
  
  block_read(inode_block + context->part_start, context->sysbuf);
  
  int i, j;
  for(i=0;i<16;i++) {
    for(j=0;j<32;j++) {
      printf("%02X ", context->sysbuf[i * 32 + j]);
    }
    printf("\n");
  }
  printf("inode index & 3 = %d\n", inode_index & 3);
  in = (struct inode *)(&context->sysbuf[context->superblock.s_inode_size * (inode_index & 3)]);
  printf("i_mode %u\n", in->i_mode);
  printf("i_uid %u\n", in->i_uid);
  printf("i_size %u\n", in->i_size);
  printf("i_atime %u\n", in->i_atime);
  printf("i_ctime %u\n", in->i_ctime);
  printf("i_mtime %u\n", in->i_mtime);
  printf("i_dtime %u\n", in->i_dtime);
  
  for(i=0;i<15;i++) {
    printf("block %d: %d\n", i, in->i_block[i]);
  }
  
  block_read(547 * 8, context->sysbuf);
  for(i=0;i<16;i++) {
    for(j=0;j<32;j++) {
      printf("%02X ", context->sysbuf[i * 32 + j]);
    }
    printf("\n");
  }
  return 0;
}

int ext2_next_sector(int fd, struct ext2context *context) {
  
}

/**
 * callable file access routines
 */

int ext2_mount(blockno_t part_start, blockno_t volume_size, 
               uint8_t filesystem_hint, struct ext2context **context) {
  (*context) = (struct ext2context *)malloc(sizeof(struct ext2context));
  (*context)->part_start = part_start;
  block_read(part_start+2, (*context)->sysbuf);
  memcpy(&(*context)->superblock, (*context)->sysbuf, sizeof(struct superblock));
  
  printf("s_inodes_count %u\n", (*context)->superblock.s_inodes_count);
  printf("s_blocks_count %u\n", (*context)->superblock.s_blocks_count);
  printf("s_r_blocks_count %u\n", (*context)->superblock.s_r_blocks_count);
  printf("s_free_blocks_count %u\n", (*context)->superblock.s_free_blocks_count);
  printf("s_free_inodes_count %u\n", (*context)->superblock.s_free_inodes_count);
  printf("s_first_data_block %u\n", (*context)->superblock.s_first_data_block);
  printf("s_log_block_size %u\n", (*context)->superblock.s_log_block_size);
  printf("s_log_frag_size %u\n", (*context)->superblock.s_log_frag_size);
  printf("s_blocks_per_group %u\n", (*context)->superblock.s_blocks_per_group);
  printf("s_frags_per_group %u\n", (*context)->superblock.s_frags_per_group);
  printf("s_inodes_per_group %u\n", (*context)->superblock.s_inodes_per_group);
  printf("s_mtime %u\n", (*context)->superblock.s_mtime);
  printf("s_wtime %u\n", (*context)->superblock.s_wtime);
  printf("s_mnt_count %u\n", (*context)->superblock.s_mnt_count);
  printf("s_max_mnt_count %u\n", (*context)->superblock.s_max_mnt_count);
  printf("s_magic %04X\n", (*context)->superblock.s_magic);
  printf("s_state %u\n", (*context)->superblock.s_state);
  printf("s_errors %u\n", (*context)->superblock.s_errors);
  printf("s_minor_rev_level %u\n", (*context)->superblock.s_minor_rev_level);
  printf("s_lastcheck %u\n", (*context)->superblock.s_lastcheck);
  printf("s_checkinterval %u\n", (*context)->superblock.s_checkinterval);
  printf("s_creator_os %u\n", (*context)->superblock.s_creator_os);
  printf("s_rev_level %u\n", (*context)->superblock.s_rev_level);
  printf("s_def_resuid %u\n", (*context)->superblock.s_def_resuid);
  printf("s_def_resgid %u\n", (*context)->superblock.s_def_resgid);
  printf("s_first_ino %u\n", (*context)->superblock.s_first_ino);
  printf("s_inode_size %u\n", (*context)->superblock.s_inode_size);
  printf("s_block_group_nr %u\n", (*context)->superblock.s_block_group_nr);
  printf("s_feature_compat %u\n", (*context)->superblock.s_feature_compat);
  printf("s_feature_incompat %u\n", (*context)->superblock.s_feature_incompat);
  printf("s_feature_ro_compat %u\n", (*context)->superblock.s_feature_ro_compat);
  printf("s_uuid[16] %s\n", (*context)->superblock.s_uuid);
  printf("s_volume_name[16] %s\n", (*context)->superblock.s_volume_name);
  printf("s_last_mounted[64] %s\n", (*context)->superblock.s_last_mounted);
  printf("s_algo_bitmap %u\n", (*context)->superblock.s_algo_bitmap);
  printf("s_prealloc_blocks %u\n", (*context)->superblock.s_prealloc_blocks);
  printf("s_prealloc_dir_blocks %u\n", (*context)->superblock.s_prealloc_dir_blocks);
  printf("s_journal_uuid[16] %s\n", (*context)->superblock.s_journal_uuid);
  printf("s_journal_inum %u\n", (*context)->superblock.s_journal_inum);
  printf("s_journal_dev %u\n", (*context)->superblock.s_journal_dev);
  printf("s_last_orphan %u\n", (*context)->superblock.s_last_orphan);
  printf("s_hash_seed[0] %u\n", (*context)->superblock.s_hash_seed[0]);
  printf("s_hash_seed[1] %u\n", (*context)->superblock.s_hash_seed[1]);
  printf("s_hash_seed[2] %u\n", (*context)->superblock.s_hash_seed[2]);
  printf("s_hash_seed[3] %u\n", (*context)->superblock.s_hash_seed[3]);
  printf("s_def_hash_version %u\n", (*context)->superblock.s_def_hash_version);
  printf("s_default_mount_options %u\n", (*context)->superblock.s_default_mount_options);
  printf("s_first_meta_bg %u\n", (*context)->superblock.s_first_meta_bg);
  
  int block_size = (1 << ((*context)->superblock.s_log_block_size + 1));       // block size in disk blocks
  block_read(block_size * 1, (*context)->sysbuf);
  
  struct block_group_descriptor * block_table;
  int i;
  for(i=0;i<5;i++) {
    block_table = (struct block_group_descriptor *)(&(*context)->sysbuf[i*32]);
    printf("== Blockno %d ==========\n", i);
    printf("bg_block_bitmap %u\n", block_table->bg_block_bitmap);
    printf("bg_inode_bitmap %u\n", block_table->bg_inode_bitmap);
    printf("bg_inode_table %u\n", block_table->bg_inode_table);
    printf("bg_free_blocks_count %u\n", block_table->bg_free_blocks_count);
    printf("bg_free_inodes_count %u\n", block_table->bg_free_inodes_count);
    printf("bg_used_dirs_count %u\n", block_table->bg_used_dirs_count);
    printf("bg_pad %u\n", block_table->bg_pad);
  }
  
  ext2_select_inode(0, EXT2_ROOT_INO, (*context));

}

int ext2_open(const char *name, int flags, int mode, int *rerrno, struct ext2context *context) {
//   int i;
//   int8_t fd;
//   (*rerrno) = 0;
//   
//   fd = fat_get_next_file();
//   if(fd < 0) {
//     (*rerrno) = ENFILE;
//     return -1;   /* too many open files */
//   }
// 
//   i = fat_lookup_path(fd, name, rerrno);
//   if((flags & O_RDWR)) {
//     file_num[fd].flags |= (FAT_FLAG_READ | FAT_FLAG_WRITE);
//   } else {
//     if((flags & O_WRONLY) == 0) {
//       file_num[fd].flags |= FAT_FLAG_READ;
//     } else {
//       file_num[fd].flags |= FAT_FLAG_WRITE;
//     }
//   }
//   
//   if(flags & O_APPEND) {
//     file_num[fd].flags |= FAT_FLAG_APPEND;
//   }
//   if((i == -1) && ((*rerrno) == ENOENT)) {
//     /* file doesn't exist */
//     if((flags & (O_CREAT)) == 0) {
//       /* tried to open a non-existent file with no create */
//       file_num[fd].flags = 0;
//       (*rerrno) = ENOENT;
//       return -1;
//     } else {
//       /* opening a new file for writing */
//       /* only create files in directories that aren't read only */
//       if(fatfs.read_only) {
//         file_num[fd].flags = 0;
//         (*rerrno) = EROFS;
//         return -1;
//       }
//       /* create an empty file structure ready for use */
//       file_num[fd].sector = 0;
//       file_num[fd].cluster = 0;
//       file_num[fd].sectors_left = 0;
//       file_num[fd].cursor = 0;
//       file_num[fd].error = 0;
//       if(mode & S_IWUSR) {
//         file_num[fd].attributes = 0;
//       } else {
//         file_num[fd].attributes = FAT_ATT_RO;
//       }
//       file_num[fd].size = 0;
//       file_num[fd].full_first_cluster = 0;
//       file_num[fd].entry_sector = 0;
//       file_num[fd].entry_number = 0;
//       file_num[fd].file_sector = 0;
//       file_num[fd].created = time(NULL);
//       file_num[fd].modified = 0;
//       file_num[fd].accessed = 0;
//       
//       memset(file_num[fd].buffer, 0, 512);
//       
//       file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
//       (*rerrno) = 0;    /* file not found but we're aloud to create it so success */
//       return fd;
//     }
//   } else if(i == 0) {
//     /* file does exist */
//     if(flags & (O_CREAT | O_EXCL)) {
//       /* tried to force creation of an existing file */
//       file_num[fd].flags = 0;
//       (*rerrno) = EEXIST;
//       return -1;
//     } else {
//       if((flags & (O_WRONLY | O_RDWR)) == 0) {
//         /* read existing file */
//         file_num[fd].file_sector = 0;
//         return fd;
//       } else {
//         /* file opened for write access, check permissions */
//         if(fatfs.read_only) {
//           /* requested write on read only filesystem */
//           file_num[fd].flags = 0;
//           (*rerrno) = EROFS;
//           return -1;
//         }
//         if(file_num[fd].attributes & FAT_ATT_RO) {
//           /* The file is read-only refuse permission */
//           file_num[fd].flags = 0;
//           (*rerrno) = EACCES;
//           return -1;
//         }
//         if(file_num[fd].attributes & FAT_ATT_SUBDIR) {
//           /* Tried to open a directory for writing */
//           file_num[fd].flags = 0;
//           (*rerrno) = EISDIR;
//           return -1;
//         }
//         if(flags & O_TRUNC) {
//           /* Need to truncate the file to zero length */
//           fat_free_clusters(file_num[fd].full_first_cluster);
//           file_num[fd].size = 0;
//           file_num[fd].full_first_cluster = 0;
//           file_num[fd].sector = 0;
//           file_num[fd].cluster = 0;
//           file_num[fd].sectors_left = 0;
//           file_num[fd].file_sector = 0;
//           file_num[fd].created = time(NULL);
//           file_num[fd].modified = time(NULL);
//           file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
//         }
//         file_num[fd].file_sector = 0;
//         return fd;
//       }
//     }
//   } else {
//     file_num[fd].flags = 0;
//     return -1;
//   }
}

int ext2_close(int fd, int *rerrno, struct ext2context *context) {
//   (*rerrno) = 0;
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if(file_num[fd].flags & FAT_FLAG_DIRTY) {
//     if(fat_flush(fd)) {
//       (*rerrno) = EIO;
//       return -1;
//     }
//   }
//   if(file_num[fd].flags & FAT_FLAG_FS_DIRTY) {
//     if(fat_flush_fileinfo(fd)) {
//       (*rerrno) = EIO;
//       return -1;
//     }
//   }
//   file_num[fd].flags = 0;
//   return 0;
}

int ext2_read(int fd, void *buffer, size_t count, int *rerrno, struct ext2context *context) {
//   uint32_t i=0;
//   uint8_t *bt = (uint8_t *)buffer;
//   /* make sure this is an open file and it can be read */
//   (*rerrno) = 0;
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if((~file_num[fd].flags) & (FAT_FLAG_OPEN | FAT_FLAG_READ)) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   
//   /* copy some bytes to the buffer requested */
//   while(i < count) {
//     if(((file_num[fd].cursor + file_num[fd].file_sector * 512)) >= file_num[fd].size) {
//       break;   /* end of file */
//     }
//     *bt++ = *(uint8_t *)(file_num[fd].buffer + file_num[fd].cursor);
//     file_num[fd].cursor++;
//     if(file_num[fd].cursor == 512) {
//       fat_next_sector(fd);
//     }
//     i++;
//   }
//   if(i > 0) {
//     fat_update_atime(fd);
//   }
//   return i;
}

int ext2_write(int fd, const void *buffer, size_t count, int *rerrno, struct ext2context *context) {
//   uint32_t i=0;
//   uint8_t *bt = (uint8_t *)buffer;
//   (*rerrno) = 0;
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if((~file_num[fd].flags) & (FAT_FLAG_OPEN | FAT_FLAG_WRITE)) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if(file_num[fd].flags & FAT_FLAG_APPEND) {
//     fat_lseek(fd, 0, SEEK_END, rerrno);
//   }
//   while(i < count) {
// //     printf("Written %d bytes\n", i);
//     if(((file_num[fd].cursor + file_num[fd].file_sector * 512)) == file_num[fd].size) {
//       file_num[fd].size++;
//       file_num[fd].flags |= FAT_FLAG_DIRTY;
//     }
//     file_num[fd].buffer[file_num[fd].cursor] = *bt++;
//     file_num[fd].cursor++;
//     if(file_num[fd].cursor == 512) {
//       if(fat_next_sector(fd)) {
//         (*rerrno) = EIO;
//         return -1;
//       }
//     }
//     i++;
//   }
//   if(i > 0) {
//     fat_update_mtime(fd);
//   }
//   return i;
}

int ext2_fstat(int fd, struct stat *st, int *rerrno, struct ext2context *context) {
//   (*rerrno) = 0;
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   st->st_dev = 0;
//   st->st_ino = 0;
//   if(file_num[fd].attributes & FAT_ATT_SUBDIR) {
//     st->st_mode = S_IFDIR;
//   } else {
//     st->st_mode = S_IFREG;
//   }
//   st->st_nlink = 1;   /* number of hard links to the file */
//   st->st_uid = 0;
//   st->st_gid = 0;     /* not implemented on FAT */
//   st->st_rdev = 0;
//   st->st_size = file_num[fd].size;
//   /* should be seconds since epoch. */
//   st->st_atime = file_num[fd].accessed;
//   st->st_mtime = file_num[fd].modified;
//   st->st_ctime = file_num[fd].created;
//   st->st_blksize = 512;
//   st->st_blocks = 1;  /* number of blocks allocated for this object */
//   return 0; 
}

int ext2_lseek(int fd, int ptr, int dir, int *rerrno, struct ext2context *context) {
//   unsigned int new_pos;
//   unsigned int old_pos;
//   int new_sec;
//   int i;
//   int file_cluster;
//   (*rerrno) = 0;
// 
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return ptr-1;
//   }
//   if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
//     (*rerrno) = EBADF;
//     return ptr-1;    /* tried to seek on a file that's not open */
//   }
//   
//   fat_flush(fd);
//   old_pos = file_num[fd].file_sector * 512 + file_num[fd].cursor;
//   if(dir == SEEK_SET) {
//     new_pos = ptr;
//   } else if(dir == SEEK_CUR) {
//     new_pos = file_num[fd].file_sector * 512 + file_num[fd].cursor + ptr;
//   } else {
//     new_pos = file_num[fd].size + ptr;
//   }
//   if(new_pos > file_num[fd].size) {
//     return ptr-1; /* tried to seek outside a file */
//   }
//   // optimisation cases
//   if((old_pos/512) == (new_pos/512)) {
//     // case 1: seekin  (*rerrno) = 0;
//   if(fd >= MAX_OPEN_FILES) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   if(!(file_num[fd].flags & FAT_FLAG_OPEN)) {
//     (*rerrno) = EBADF;
//     return -1;
//   }
//   st->st_dev = 0;
//   st->st_ino = 0;
//   if(file_num[fd].attributes & FAT_ATT_SUBDIR) {
//     st->st_mode = S_IFDIR;
//   } else {
//     st->st_mode = S_IFREG;
//   }
//   st->st_nlink = 1;   /* number of hard links to the file */
//   st->st_uid = 0;
//   st->st_gid = 0;     /* not implemented on FAT */
//   st->st_rdev = 0;
//   st->st_size = file_num[fd].size;
//   /* should be seconds since epoch. */
//   st->st_atime = file_num[fd].accessed;
//   st->st_mtime = file_num[fd].modified;
//   st->st_ctime = file_num[fd].created;
//   st->st_blksize = 512;
//   st->st_blocks = 1;  /* number of blocks allocated for this object */
//   return 0; g within a disk block
//     file_num[fd].cursor = new_pos & 0x1ff;
//     return new_pos;
//   } else if((new_pos / (fatfs.sectors_per_cluster * 512)) == (old_pos / (fatfs.sectors_per_cluster * 512))) {
//     // case 2: seeking within the cluster, just need to hop forward/back some sectors
//     file_num[fd].file_sector = new_pos / 512;
//     file_num[fd].sector = file_num[fd].sector + (new_pos/512) - (old_pos/512);
//     file_num[fd].sectors_left = file_num[fd].sectors_left + (new_pos/512) - (old_pos/512);
//     file_num[fd].cursor = new_pos & 0x1ff;
//     if(block_read(file_num[fd].sector, file_num[fd].buffer)) {
// //       iprintf("Bad block read.\r\n");
//       return ptr - 1;
//     }
//     return new_pos;
//   }
//   // otherwise we need to seek the cluster chain
//   file_cluster = new_pos / (fatfs.sectors_per_cluster * 512);
//   
//   file_num[fd].cluster = file_num[fd].full_first_cluster;
//   i = 0;
//   // walk the FAT cluster chain until we get to the right one
//   while(i<file_cluster) {
//     file_num[fd].cluster = fat_next_cluster(fd, rerrno);
//     i++;
//   }
//   file_num[fd].file_sector = new_pos / 512;
//   file_num[fd].cursor = new_pos & 0x1ff;
//   new_sec = new_pos - file_cluster * fatfs.sectors_per_cluster * 512;
//   new_sec = new_sec / 512;
//   file_num[fd].sector = file_num[fd].cluster * fatfs.sectors_per_cluster + fatfs.cluster0 + new_sec;
//   file_num[fd].sectors_left = fatfs.sectors_per_cluster - new_sec - 1;
//   if(block_read(file_num[fd].sector, file_num[fd].buffer)) {
//     return ptr-1;
// //     iprintf("Bad block read 2.\r\n");
//   }
//   return new_pos;
}

int ext2_get_next_dirent(int fd, struct dirent *out_de) {
//   direntS *de;
// //   int i,j;
// //   iprintf("%d\r\n", file_num[fd].cursor);
//   de = (direntS *)(file_num[fd].buffer + file_num[fd].cursor);
// 
//   /* first check the current entry isn't the end of the folder */
//   if(de->filename[0] == 0) {
//     return -1;
//   }
//   /* now keep looping past LFN entries until a valid one or the end of dir is found */
//   while(1) {
//     /* otherwise look for the next entry */
// //     iprintf("looping %d\r\n", file_num[fd].cursor);
//     if(file_num[fd].cursor + 32 == 512) {
//       if(fat_next_sector(fd) == -1) {
//         return -1;  /* there are no more sectors allocated to this directory */
//       }
//     } else {
//       file_num[fd].cursor += 32;
//     }
//     de = (direntS *)(file_num[fd].buffer + file_num[fd].cursor);
//     if(de->filename[0] == 0) {
//       return -1;
//     }
//     if(!((de->attributes == 0x0F) || (de->attributes & FAT_ATT_VOL))) {
//       /* if it's not an LFN and not a volume label it's a real file. */
//       fatname_to_str(out_de->d_name, de->filename);
//       if(fatfs.type == PART_TYPE_FAT16) {
//         out_de->d_ino = de->first_cluster;
//       } else {
//         out_de->d_ino = de->first_cluster + (de->high_first_cluster << 16);
//       }
//       return 0;
//     }
//   }
}
