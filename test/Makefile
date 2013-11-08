all:	test_gristle.c hash.c hash.h ../src/block.h ../src/block_drivers/block_pc.c ../src/block_drivers/block_pc.h ../src/gristle.c ../src/gristle.h ../src/partition.c ../src/partition.h
	gcc -Wall -g -I./ test_gristle.c hash.c ../src/block_drivers/block_pc.c ../src/gristle.c ../src/partition.c -o test_gristle

embext: test_embext.c ../src/embext.c ../src/block_drivers/block_pc.c hash.c ../src/embext.h ../src/block_drivers/block_pc.h hash.h
	gcc -Wall -g -I. -I../src test_embext.c ../src/embext.c ../src/block_drivers/block_pc.c hash.c -o test_embext

