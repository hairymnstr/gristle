CFLAGS	+= -Wall -Wextra -g -Os -I./ -I../src -I../src/block_drivers/

all:	test_gristle test_embext show_info

test_gristle:	test_gristle.c hash.c hash.h ../src/block.h ../src/block_drivers/block_pc.c ../src/block_drivers/block_pc.h \
		../src/gristle.c ../src/gristle.h ../src/partition.c ../src/partition.h Makefile
	gcc $(CFLAGS) test_gristle.c hash.c ../src/block_drivers/block_pc.c ../src/gristle.c ../src/partition.c -o test_gristle

test_embext: 	test_embext.c ../src/embext.c ../src/block_drivers/block_pc.c hash.c ../src/embext.h \
		../src/block_drivers/block_pc.h hash.h Makefile
	gcc $(CFLAGS) -DEXT_DEBUG test_embext.c ../src/embext.c ../src/block_drivers/block_pc.c hash.c -o test_embext

show_info:	show_info.c ../src/block.h ../src/block_drivers/block_pc.c ../src/block_drivers/block_pc.h ../src/gristle.c \
		../src/gristle.h ../src/partition.c ../src/partition.h Makefile
	gcc $(CFLAGS) show_info.c hash.c ../src/block_drivers/block_pc.c ../src/gristle.c ../src/partition.c -o show_info

