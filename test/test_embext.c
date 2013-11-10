#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dirent.h"
#include "block.h"
#include "embext.h"

int main(int argc, char *argv[]) {
  int p = 0;
  int result;
  char buffer[256];
  struct stat st;
  struct ext2context *context;
  printf("Running FAT tests...\n\n");
  printf("[%4d] start block device emulation...", p++);
  printf("   %d\n", block_init());
  
  printf("[%4d] mount filesystem, FAT32", p++);
  
  result = ext2_mount(0, block_get_volume_size(), 0, &context);

  printf("   %d\n", result);
  
  struct file_ent *fe = ext2_open("/", O_RDONLY, 0777, &result, context);
  struct file_ent *fe2;
  struct dirent *de;
  
  while((de = ext2_readdir(fe, &result, context))) {
    snprintf(buffer, sizeof(buffer), "/%s", de->d_name);
    fe2 = ext2_open(buffer, O_RDONLY, 0777, &result, context);
    ext2_fstat(fe2, &st, &result, context);
    printf("%s %d\n", de->d_name, (int)st.st_size);
    if(st.st_mode & S_IFDIR) {
//       printf("Directory contents:\n");
//       printf("fe2->cursor: %d\n", fe2->cursor);
//       printf("fe2->file_sector: %d\n", fe2->file_sector);
      while((de = ext2_readdir(fe2, &result, context))) {
        printf("  %s [%d]\n", de->d_name, de->d_ino);
      }
    }
    ext2_close(fe2, &result, context);
  }
  ext2_close(fe, &result, context);
  
  FILE *fw = fopen("dump.png", "wb");
  fe = ext2_open("/static/Codec - OggBox Wiki.html", O_RDONLY, 0777, &result, context);
  printf("%p\n", fe);
  printf("fe->inode_number = %d\n", fe->inode_number);
  while((p = ext2_read(fe, &buffer, sizeof(buffer), &result, context)) == sizeof(buffer)) {
    fwrite(buffer, 1, p, fw);
  }
  if(p > 0) {
    fwrite(buffer, 1, p, fw);
  }
  fclose(fw);
  exit(0);
}
