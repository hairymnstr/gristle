#include <stdio.h>
#include <stdlib.h>
#include "block.h"
#include "embext.h"

int main(int argc, char *argv[]) {
  int p = 0;
  int result;
  struct ext2context *context;
  printf("Running FAT tests...\n\n");
  printf("[%4d] start block device emulation...", p++);
  printf("   %d\n", block_init());
  
  printf("[%4d] mount filesystem, FAT32", p++);
  
  result = ext2_mount(0, block_get_volume_size(), 0, &context);

  printf("   %d\n", result);

  exit(0);
}
