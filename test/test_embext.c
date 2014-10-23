#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dirent.h"
#include "block_pc.h"
#include "block.h"
#include "embext.h"

int main(int argc, char *argv[]) {
  int p = 0;
  int result;
  char buffer[256];
  struct stat st;
  struct ext2context *context;
  printf("Running EXT2 tests...\n\n");
  block_pc_set_image_name("testext.img");
  printf("[%4d] start block device emulation...", p++);
  result = block_init();
  printf("   %d\n", result);
  if(result != 0) {
      exit(0);
  }
  
  printf("[%4d] mount filesystem, FAT32", p++);
  
  result = ext2_mount(0, block_get_volume_size(), 0, &context);

  printf("   %d\n", result);
  
  struct file_ent *fe = ext2_open(context, "/", O_RDONLY, 0777, &result);
  struct file_ent *fe2;
  struct dirent *de;
  
  while((de = ext2_readdir(fe, &result))) {
    snprintf(buffer, sizeof(buffer), "/%s", de->d_name);
    fe2 = ext2_open(context, buffer, O_RDONLY, 0777, &result);
    ext2_fstat(fe2, &st, &result);
    printf("%s %d\n", de->d_name, (int)st.st_size);
    if((st.st_mode & S_IFDIR) && (strcmp(de->d_name, ".") != 0) &&
       (strcmp(de->d_name, "..") != 0)) {
//       printf("Directory contents:\n");
//       printf("fe2->cursor: %d\n", fe2->cursor);
//       printf("fe2->file_sector: %d\n", fe2->file_sector);
      while((de = ext2_readdir(fe2, &result))) {
        printf("  %s [%d]\n", de->d_name, de->d_ino);
      }
    }
    ext2_close(fe2, &result);
  }
  ext2_close(fe, &result);
  
  FILE *fw = fopen("dump.png", "wb");
  fe = ext2_open(context, "/static/test_image.png", O_RDONLY, 0777, &result);
  printf("%p\n", fe);
  printf("fe->inode_number = %d\n", fe->inode_number);
  while((p = ext2_read(fe, &buffer, sizeof(buffer), &result)) == sizeof(buffer)) {
    fwrite(buffer, 1, p, fw);
  }
  if(p > 0) {
    fwrite(buffer, 1, p, fw);
  }
  fclose(fw);
  ext2_close(fe, &result);
  
  printf("\nWrite test...\n\n");
  
  fe = ext2_open(context, "/logs/test.txt", O_WRONLY | O_APPEND, 0777, &result);
  if(fe == NULL) {
      printf("Open for writing failed, errno=%d (%s)\r\n", result, strerror(result));
      exit(-1);
  }
  
  ext2_write(fe, "Hello world\r\n", 13, &result);
  
  ext2_close(fe, &result);
  
  ext2_print_bg1_bitmap(context);
  
  ext2_umount(context);
  
  block_pc_snapshot_all("writenfs.img");
  
  block_halt();
  
  exit(0);
}
