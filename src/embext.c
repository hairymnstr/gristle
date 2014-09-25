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
#ifdef EXT_DEBUG
#include <inttypes.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "dirent.h"
#include <errno.h>
#include "block.h"
#include "partition.h"
#include "embext.h"

#ifdef EXT_DEBUG
void ext2_print_inode(struct inode *in) {
    int i;
    printf("i_mode = %" PRIu16 "\n", in->i_mode);
    printf("i_uid = %" PRIu16 "\n", in->i_uid);
    printf("i_size = %" PRIu32 "\n", in->i_size);
    printf("i_atime = %" PRIu32 "\n", in->i_atime);
    printf("i_ctime = %" PRIu32 "\n", in->i_ctime);
    printf("i_mtime = %" PRIu32 "\n", in->i_mtime);
    printf("i_dtime = %" PRIu32 "\n", in->i_dtime);
    printf("i_gid = %" PRIu16 "\n", in->i_gid);
    printf("i_links_count = %" PRIu16 "\n", in->i_links_count);
    printf("i_blocks = %" PRIu32 "\n", in->i_blocks);
    printf("i_flags = %" PRIu32 "\n", in->i_flags);
    printf("i_osd1 = %" PRIu32 "\n", in->i_osd1);
    printf("i_block = [\n");
    for(i=0;i<15;i++) {
        printf("  %" PRIu32 ",\n", in->i_block[i]);
    }
    printf("  ]\n");
    printf("i_generation = %" PRIu32 "\n", in->i_generation);
    printf("i_file_acl = %" PRIu32 "\n", in->i_file_acl);
    printf("i_dir_acl = %" PRIu32 "\n", in->i_dir_acl);
    printf("i_faddr = %" PRIu32 "\n", in->i_faddr);
    printf("i_osd2 = \"");
    for(i=0;i<12;i++) {
        if((in->i_osd2[i] < ' ') || (in->i_osd2[i] > '~')) {
            printf("\\x%02x", in->i_osd2[i]);
        } else {
            printf("%c", in->i_osd2[i]);
        }
    }
    printf("\"\n");
}
#endif

int ext2_flush(struct file_ent *fe) {
  if(fe->flags & EXT2_FLAG_DIRTY) {
    if(fe->sector == 0) {
      // new file
      printf("New file, not supported.\r\n");
    } else {
      if(block_write(fe->sector, fe->buffer)) {
        return -1;
      }
      fe->flags &= ~EXT2_FLAG_DIRTY;
    }
  }
  return 0;
}

int ext2_flush_inode(struct file_ent *fe) {
  uint32_t inode_block;
  uint32_t block_group = (fe->inode_number - 1) / fe->context->superblock.s_inodes_per_group;
  uint32_t inode_index = (fe->inode_number - 1) % fe->context->superblock.s_inodes_per_group;
  // now load the block group descriptor for that block group
  uint32_t bg_block = fe->context->superblock_block + 1;
  struct block_group_descriptor *block_table;

  if(fe->flags & EXT2_FLAG_FS_DIRTY) {
    ext2_flush(fe);
    //find the inode
  
    bg_block <<= (fe->context->superblock.s_log_block_size + 1);
    bg_block += ((block_group * 32) / block_get_block_size());
    
    block_read(bg_block + fe->context->part_start, fe->context->sysbuf);
    
    block_table = (struct block_group_descriptor *)&fe->context->sysbuf[(block_group * 32) % block_get_block_size()];
    
    inode_block = block_table->bg_inode_table;
    inode_block <<= (fe->context->superblock.s_log_block_size + 1);
    
    inode_block += (inode_index / (block_get_block_size() / fe->context->superblock.s_inode_size));
    
    // load the sector
    block_read(inode_block + fe->context->part_start, fe->context->sysbuf);
    
    memcpy(&fe->context->sysbuf[(inode_index % (block_get_block_size() / fe->context->superblock.s_inode_size)) * fe->context->superblock.s_inode_size], &fe->inode, sizeof(struct inode));
    
    // write the sector
    block_write(inode_block + fe->context->part_start, fe->context->sysbuf);
    
    fe->flags &= ~EXT2_FLAG_FS_DIRTY;
  }
  
  return 0;
}

int ext2_update_atime(struct file_ent *fe) {
  fe->inode.i_atime = time(NULL);
  fe->flags |= EXT2_FLAG_FS_DIRTY;
  return 0;
}

int ext2_update_mtime(struct file_ent *fe) {
  fe->inode.i_mtime = time(NULL);
  fe->flags |= EXT2_FLAG_FS_DIRTY;
  return 0;
}

int ext2_open_inode(struct file_ent *fe, int inode) {
  struct block_group_descriptor *block_table;
  uint32_t inode_block;
  uint32_t block_group = (inode - 1) / fe->context->superblock.s_inodes_per_group;
  uint32_t inode_index = (inode - 1) % fe->context->superblock.s_inodes_per_group;
  // now load the block group descriptor for that block group
  uint32_t bg_block = fe->context->superblock_block + 1;
  
  bg_block <<= (fe->context->superblock.s_log_block_size + 1);
  
  bg_block += ((block_group * 32) / block_get_block_size());
  
  block_read(bg_block + fe->context->part_start, fe->context->sysbuf);
  
  block_table = (struct block_group_descriptor *)&fe->context->sysbuf[(block_group * 32) % block_get_block_size()];
  
  inode_block = block_table->bg_inode_table;
  
  inode_block <<= (fe->context->superblock.s_log_block_size + 1);
  
  inode_block += (inode_index / (block_get_block_size() / fe->context->superblock.s_inode_size));
  
  block_read(inode_block + fe->context->part_start, fe->context->sysbuf);
  
  memcpy(&fe->inode, &fe->context->sysbuf[(inode_index % (block_get_block_size() / fe->context->superblock.s_inode_size)) * fe->context->superblock.s_inode_size], sizeof(struct inode));
  
  block_read((fe->inode.i_block[0] << (fe->context->superblock.s_log_block_size + 1)) + fe->context->part_start, fe->buffer);
  fe->inode_number = inode;
  fe->flags = EXT2_FLAG_READ;
  fe->cursor = 0;
  fe->sector = (fe->inode.i_block[0] << (fe->context->superblock.s_log_block_size + 1)) + fe->context->part_start;
  fe->file_sector = 0;
  fe->sectors_left = (1 << (fe->context->superblock.s_log_block_size + 1)) - 1;
  fe->block_index[0] = 0;
  fe->block_index[1] = 0;
  fe->block_index[2] = 0;
  
  return 0;
}

int ext2_lookup_path(struct file_ent *fe, const char *path, int *rerrno) {
  char local_path[MAX_PATH_LEN];
  char *elements[MAX_PATH_LEVELS];
  int levels = 0;
  uint32_t ino = EXT2_ROOT_INO;
  struct dirent *de;
  int i;
  
  strncpy(local_path, path, sizeof(local_path));
  local_path[MAX_PATH_LEN-1] = 0;
  
  if((elements[levels] = strtok(local_path, "/"))) {
    while(++levels < MAX_PATH_LEVELS) {
      if(!(elements[levels] = strtok(NULL, "/"))) {
        break;
      }
    }
  }
  
  for(i=0;i<levels;i++) {
    ext2_open_inode(fe, ino);
    de = ext2_readdir(fe, rerrno);
    while(de != NULL) {
      if(strcmp(de->d_name, elements[i]) == 0) {
        break;
      }
      de = ext2_readdir(fe, rerrno);
    }
    if(de == NULL) {
      *rerrno = ENOENT;
      return -1;
    }
    ino = de->d_ino;
  }

  // right, ino is now the inode of the target file/directory
  return ext2_open_inode(fe, ino);
}

int ext2_next_block(struct file_ent *fe) {
  
  if(fe->block_index[0] < 11) {
    fe->block_index[0]++;
    if(fe->inode.i_block[fe->block_index[0]] > 0) {
      fe->sectors_left = ((1 << (10 + fe->context->superblock.s_log_block_size)) / block_get_block_size()) - 1;
      fe->sector = (fe->inode.i_block[fe->block_index[0]] << (fe->context->superblock.s_log_block_size + 1)) + fe->context->part_start;
      fe->cursor = 0;
      fe->file_sector++;
      return block_read(fe->sector, fe->buffer);
    } else {
      return 1;
    }
  } else {
    printf("Oh dear, indirect block :(\n");
    return 1;
  }
  return 0;
}

int ext2_next_sector(struct file_ent *fe) {
  if(fe->sectors_left > 0) {
    block_read(++fe->sector, fe->buffer);
    fe->sectors_left--;
    fe->cursor = 0;
    fe->file_sector++;
    return 0;
  }
  return ext2_next_block(fe);
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
  
//   (*context)->block_mask = (1 << ((*context)->superblock.s_log_block_size + 1)) - 1;
//   (*context)->same_block = part_start & (*context)->block_mask;
//   (*context)->inode_number = 0;
//   (*context)->inode_dirty = 0;
  
  if((*context)->superblock.s_log_block_size == 0) {
    (*context)->superblock_block = 1;
  } else {
    (*context)->superblock_block = 0;
  }
  
  (*context)->read_only = block_get_device_read_only();
    
//   printf("s_inodes_count %u\n", (*context)->superblock.s_inodes_count);
//   printf("s_blocks_count %u\n", (*context)->superblock.s_blocks_count);
//   printf("s_r_blocks_count %u\n", (*context)->superblock.s_r_blocks_count);
//   printf("s_free_blocks_count %u\n", (*context)->superblock.s_free_blocks_count);
//   printf("s_free_inodes_count %u\n", (*context)->superblock.s_free_inodes_count);
//   printf("s_first_data_block %u\n", (*context)->superblock.s_first_data_block);
//   printf("s_log_block_size %u\n", (*context)->superblock.s_log_block_size);
//   printf("s_log_frag_size %u\n", (*context)->superblock.s_log_frag_size);
//   printf("s_blocks_per_group %u\n", (*context)->superblock.s_blocks_per_group);
//   printf("s_frags_per_group %u\n", (*context)->superblock.s_frags_per_group);
//   printf("s_inodes_per_group %u\n", (*context)->superblock.s_inodes_per_group);
//   printf("s_mtime %u\n", (*context)->superblock.s_mtime);
//   printf("s_wtime %u\n", (*context)->superblock.s_wtime);
//   printf("s_mnt_count %u\n", (*context)->superblock.s_mnt_count);
//   printf("s_max_mnt_count %u\n", (*context)->superblock.s_max_mnt_count);
//   printf("s_magic %04X\n", (*context)->superblock.s_magic);
//   printf("s_state %u\n", (*context)->superblock.s_state);
//   printf("s_errors %u\n", (*context)->superblock.s_errors);
//   printf("s_minor_rev_level %u\n", (*context)->superblock.s_minor_rev_level);
//   printf("s_lastcheck %u\n", (*context)->superblock.s_lastcheck);
//   printf("s_checkinterval %u\n", (*context)->superblock.s_checkinterval);
//   printf("s_creator_os %u\n", (*context)->superblock.s_creator_os);
//   printf("s_rev_level %u\n", (*context)->superblock.s_rev_level);
//   printf("s_def_resuid %u\n", (*context)->superblock.s_def_resuid);
//   printf("s_def_resgid %u\n", (*context)->superblock.s_def_resgid);
//   printf("s_first_ino %u\n", (*context)->superblock.s_first_ino);
//   printf("s_inode_size %u\n", (*context)->superblock.s_inode_size);
//   printf("s_block_group_nr %u\n", (*context)->superblock.s_block_group_nr);
//   printf("s_feature_compat %u\n", (*context)->superblock.s_feature_compat);
//   printf("s_feature_incompat %u\n", (*context)->superblock.s_feature_incompat);
//   printf("s_feature_ro_compat %u\n", (*context)->superblock.s_feature_ro_compat);
//   printf("s_uuid[16] %s\n", (*context)->superblock.s_uuid);
//   printf("s_volume_name[16] %s\n", (*context)->superblock.s_volume_name);
//   printf("s_last_mounted[64] %s\n", (*context)->superblock.s_last_mounted);
//   printf("s_algo_bitmap %u\n", (*context)->superblock.s_algo_bitmap);
//   printf("s_prealloc_blocks %u\n", (*context)->superblock.s_prealloc_blocks);
//   printf("s_prealloc_dir_blocks %u\n", (*context)->superblock.s_prealloc_dir_blocks);
//   printf("s_journal_uuid[16] %s\n", (*context)->superblock.s_journal_uuid);
//   printf("s_journal_inum %u\n", (*context)->superblock.s_journal_inum);
//   printf("s_journal_dev %u\n", (*context)->superblock.s_journal_dev);
//   printf("s_last_orphan %u\n", (*context)->superblock.s_last_orphan);
//   printf("s_hash_seed[0] %u\n", (*context)->superblock.s_hash_seed[0]);
//   printf("s_hash_seed[1] %u\n", (*context)->superblock.s_hash_seed[1]);
//   printf("s_hash_seed[2] %u\n", (*context)->superblock.s_hash_seed[2]);
//   printf("s_hash_seed[3] %u\n", (*context)->superblock.s_hash_seed[3]);
//   printf("s_def_hash_version %u\n", (*context)->superblock.s_def_hash_version);
//   printf("s_default_mount_options %u\n", (*context)->superblock.s_default_mount_options);
//   printf("s_first_meta_bg %u\n", (*context)->superblock.s_first_meta_bg);
  
//   int block_size = (1 << ((*context)->superblock.s_log_block_size + 1));       // block size in disk blocks
//   block_read(block_size * 1, (*context)->sysbuf);
//   
//   struct block_group_descriptor * block_table;
//   int i;
//   for(i=0;i<5;i++) {
//     block_table = (struct block_group_descriptor *)(&(*context)->sysbuf[i*32]);
//     printf("== Blockno %d ==========\n", i);
//     printf("bg_block_bitmap %u\n", block_table->bg_block_bitmap);
//     printf("bg_inode_bitmap %u\n", block_table->bg_inode_bitmap);
//     printf("bg_inode_table %u\n", block_table->bg_inode_table);
//     printf("bg_free_blocks_count %u\n", block_table->bg_free_blocks_count);
//     printf("bg_free_inodes_count %u\n", block_table->bg_free_inodes_count);
//     printf("bg_used_dirs_count %u\n", block_table->bg_used_dirs_count);
//     printf("bg_pad %u\n", block_table->bg_pad);
//   }
  
//   ext2_select_inode(EXT2_ROOT_INO, (*context));
  
//   struct file_ent fe;
//   fe.inode_number = 0;
//   fe.sector = 0;
//   fe.flags = 0;
//   fe.cursor = 0;
//   fe.block_index[0] = 0;
//   ext2_open_inode(&fe, EXT2_ROOT_INO, (*context));
//   block_read(fe.inode.i_block[0] * 8, fe.buffer);
//   printf("block %d\n", fe.inode.i_block[0]);
//   char nm[256];
//   int ofs = 0;
//   struct ext2_dirent *de;
//   while(ofs < 512) {
//     de = (struct ext2_dirent *)(&fe.buffer[ofs]);
//     if(de->inode == 0) {
//       break;
//     }
//     memcpy(nm, &de->name, de->name_len);
//     nm[de->name_len] = 0;
//     printf("%s %d\n", nm, de->inode);
//     ofs += de->rec_len;
//   }
//   int j, i;
//   do {
//     for(i=0;i<16;i++) {
//       for(j=0;j<32;j++) {
//         printf("%02X ", fe.buffer[i*32 + j]);
//       }
//       printf("\n");
//     }
//     printf("-------------------------------------\n");
//   } while(!ext2_next_sector(&fe, (*context)));
  
  
  return 0;
}

struct file_ent *ext2_open(struct ext2context *context, const char *name, int flags, int mode, 
                           int *rerrno) {
  int i;
  struct file_ent *fe = (struct file_ent *)malloc(sizeof(struct file_ent));
  if(fe == NULL) {
    (*rerrno) = ENOMEM;
    return NULL;
  }
  memset(fe, 0, sizeof(struct file_ent));
  fe->context = context;
  i = ext2_lookup_path(fe, name, rerrno);
  if((flags & O_RDWR)) {
    fe->flags |= (EXT2_FLAG_READ | EXT2_FLAG_WRITE);
  } else {
    if((flags & O_WRONLY) == 0) {
      fe->flags |= EXT2_FLAG_READ;
    } else {
      fe->flags |= EXT2_FLAG_WRITE;
    }
  }
  
  if(flags & O_APPEND) {
    fe->flags |= EXT2_FLAG_APPEND;
  }
  if((i == -1) && ((*rerrno) == ENOENT)) {
    /* file doesn't exist */
    if((flags & (O_CREAT)) == 0) {
      /* tried to open a non-existent file with no create */
      free(fe);
      (*rerrno) = ENOENT;
      return NULL;
    } else {
      /* opening a new file for writing */
      /* only create files in directories that aren't read only */
      if(fe->context->read_only) {
        free(fe);
        (*rerrno) = EROFS;
        return NULL;
      }
      /* create an empty file structure ready for use */
      printf("Not implemented, making a new file\r\n");
      free(fe);
      return NULL;
      
//       file_num[fd].flags |= FAT_FLAG_FS_DIRTY;
//       (*rerrno) = 0;    /* file not found but we're aloud to create it so success */
//       return fd;
    }
  } else if(i == 0) {
    /* file does exist */
    if(flags & (O_CREAT | O_EXCL)) {
      /* tried to force creation of an existing file */
      free(fe);
      (*rerrno) = EEXIST;
      return NULL;
    } else {
      if((flags & (O_WRONLY | O_RDWR)) == 0) {
        /* read existing file */
        fe->file_sector = 0;
        return fe;
      } else {
        /* file opened for write access, check permissions */
        if(fe->context->read_only) {
          /* requested write on read only filesystem */
          free(fe);
          (*rerrno) = EROFS;
          return NULL;
        }
        // TODO read only permissions
//         if(file_num[fd].attributes & FAT_ATT_RO) {
//           /* The file is read-only refuse permission */
//           free(fe);
//           (*rerrno) = EACCES;
//           return NULL;
//         }
        if(fe->inode.i_mode & EXT2_S_IFDIR) {
          /* Tried to open a directory for writing */
          free(fe);
          (*rerrno) = EISDIR;
          return NULL;
        }
        if(flags & O_TRUNC) {
          /* Need to truncate the file to zero length */
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
          printf("Not implemented, O_TRUNC\r\n");
          free(fe);
          return NULL;
        }
        fe->file_sector = 0;
        return fe;
      }
    }
  } else {
    free(fe);
    return NULL;
  }
}

int ext2_close(struct file_ent *fe, int *rerrno) {
  if(fe == NULL) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(fe->flags & EXT2_FLAG_DIRTY) {
    if(ext2_flush(fe)) {
      (*rerrno) = EIO;
      return -1;
    }
  }
  if(fe->flags & EXT2_FLAG_FS_DIRTY) {
    if(ext2_flush_inode(fe)) {
      (*rerrno) = EIO;
      return -1;
    }
  }
  
  free(fe);
  return 0;
}

int ext2_read(struct file_ent *fe, void *buffer, size_t count, int *rerrno) {
  uint32_t i=0;
  uint8_t *bt = (uint8_t *)buffer;
  /* make sure this is an open file and it can be read */  
  if(fe == NULL) {
    (*rerrno) = EBADF;
    return -1;
  }
  /* copy some bytes to the buffer requested */
  while(i < count) {
    if(((fe->cursor + fe->file_sector * block_get_block_size())) >= fe->inode.i_size) {
      break;   /* end of file */
    }
    *bt++ = *(uint8_t *)(fe->buffer + fe->cursor);
    fe->cursor++;
    if(fe->cursor == block_get_block_size()) {
      ext2_next_sector(fe);
    }
    i++;
  }
  if(i > 0) {
    ext2_update_atime(fe);
  }
  return i;
}

int ext2_write(struct file_ent *fe, const void *buffer, size_t count, 
               int *rerrno) {
  uint32_t i=0;
  uint8_t *bt = (uint8_t *)buffer;
  if(fe == NULL) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(!(fe->flags & EXT2_FLAG_WRITE)) {
    (*rerrno) = EBADF;
    return -1;
  }
  if(fe->flags & EXT2_FLAG_APPEND) {
    if(ext2_lseek(fe, 0, SEEK_END, rerrno) == -1) {
        return -1;
    }
  }
  while(i < count) {
    if(((fe->cursor + fe->file_sector * 512)) == fe->inode.i_size) {
      fe->inode.i_size++;
      fe->flags |= EXT2_FLAG_DIRTY;
    }
    fe->buffer[fe->cursor] = *bt++;
    fe->cursor++;
    if(fe->cursor == 512) {
      if(ext2_next_sector(fe)) {
        (*rerrno) = EIO;
        return -1;
      }
    }
    i++;
  }
  if(i > 0) {
    ext2_update_mtime(fe);
  }
  return i;
}

int ext2_fstat(struct file_ent *fe, struct stat *st, 
               int *rerrno) {
  (*rerrno) = 0;
  if(fe == NULL) {
    (*rerrno) = EBADF;
    return -1;
  }
  st->st_dev = 0;
  st->st_ino = fe->inode_number;
  st->st_mode = fe->inode.i_mode;
  st->st_nlink = fe->inode.i_links_count;   /* number of hard links to the file */
  st->st_uid = fe->inode.i_uid;
  st->st_gid = fe->inode.i_gid;     /* not implemented on FAT */
  st->st_rdev = 0;
  st->st_size = fe->inode.i_size;
  /* should be seconds since epoch. */
  st->st_atime = fe->inode.i_atime;
  st->st_mtime = fe->inode.i_mtime;
  st->st_ctime = fe->inode.i_ctime;
  st->st_blksize = 1 << (fe->context->superblock.s_log_block_size + 10);
  st->st_blocks = (fe->inode.i_size / (1 << (fe->context->superblock.s_log_block_size + 10)));  /* number of blocks allocated for this object */
  if(fe->inode.i_size % (1 << (fe->context->superblock.s_log_block_size + 10))) {
    st->st_blocks++;
  }
  return 0; 
}

int ext2_lseek(struct file_ent *fe, int ptr, int dir,
               int *rerrno) {
  unsigned int new_pos;
  unsigned int old_pos;
  int new_sec;
  uint32_t block;
  (*rerrno) = 0;

  if(fe == NULL) {
    (*rerrno) = EBADF;
    return ptr-1;
  }
  old_pos = fe->file_sector * block_get_block_size() + fe->cursor;
  
  if(dir == SEEK_SET) {
    new_pos = ptr;
  } else if(dir == SEEK_CUR) {
    new_pos = fe->file_sector * block_get_block_size() + fe->cursor + ptr;
  } else {
    new_pos = fe->inode.i_size + ptr;
  }
  if(old_pos == new_pos) {
    // if the offset was, or effectively would be zero, just say where we are
    return old_pos;
  }
  
  // TODO: support seeking past the end on writeable files
  if(new_pos > fe->inode.i_size) {
    return ptr-1; /* tried to seek outside a file */
  }
  // optimisation cases
  if((old_pos/block_get_block_size()) == (new_pos/block_get_block_size())) {
    // case 1: seekin  (*rerrno) = 0;
    fe->cursor = new_pos % block_get_block_size();
    return new_pos;
  } else if((new_pos / (1 << (fe->context->superblock.s_log_block_size + 10))) == (old_pos / (1 << (fe->context->superblock.s_log_block_size + 10)))) {
    // case 2: seeking within the cluster, just need to hop forward/back some sectors
    ext2_flush(fe);       // need to flush before loading a new sector
    fe->file_sector = new_pos / block_get_block_size();
    fe->sector = fe->sector + (new_pos/block_get_block_size()) - (old_pos/block_get_block_size());
    fe->sectors_left = fe->sectors_left + (new_pos/block_get_block_size()) - (old_pos/block_get_block_size());
    fe->cursor = new_pos % block_get_block_size();
    if(block_read(fe->sector, fe->buffer)) {
      return ptr - 1;
    }
    return new_pos;
  }
  ext2_flush(fe);
  // otherwise we need to seek the cluster chain
  block = new_pos / (1 << (fe->context->superblock.s_log_block_size + 10));
  
  if(block > 11) {
    printf("Uh oh, indirect block :(\r\n");
    return ptr-1;
  }
  fe->block_index[0] = block;
  
  fe->file_sector = new_pos / block_get_block_size();
  fe->cursor = new_pos % block_get_block_size();
  new_sec = new_pos - block * (1 << (fe->context->superblock.s_log_block_size + 10));
  new_sec = new_sec / block_get_block_size();
  fe->sector = fe->inode.i_block[fe->block_index[0]] * (1 << (fe->context->superblock.s_log_block_size + 1)) + fe->context->part_start + new_sec;
  fe->sectors_left = (1 << (fe->context->superblock.s_log_block_size + 1)) - new_sec - 1;
  if(block_read(fe->sector, fe->buffer)) {
    return ptr-1;
//     iprintf("Bad block read 2.\r\n");
  }
  return new_pos;
}

int ext2_isatty(struct file_ent *fe, int *rerrno) {
    if(fe == NULL) {
        *rerrno = EBADF;
    } else {
        *rerrno = ENOTTY;
    }
    return 0;
}

struct dirent *ext2_readdir(struct file_ent *fe, int *rerrno) {
  uint16_t rec_len;
  uint8_t name_len;
  uint8_t file_type;
  static struct dirent de;
  
  if(ext2_read(fe, &de.d_ino, 4, rerrno) < 4) {
    return NULL;
  }
  if(ext2_read(fe, &rec_len, 2, rerrno) < 2) {
    return NULL;
  }
  if(ext2_read(fe, &name_len, 1, rerrno) < 1) {
    return NULL;
  }
  if(ext2_read(fe, &file_type, 1, rerrno) < 1) {
    return NULL;
  }
  
  if(de.d_ino > 0) {
    ext2_read(fe, de.d_name, name_len, rerrno);
    de.d_name[name_len] = 0;
  
    ext2_lseek(fe, rec_len - 8 - name_len, SEEK_CUR, rerrno);
    return &de;
  } else {
    return NULL;
  }
}
