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

#ifndef BLOCK_H
#define BLOCK_H 1

/**
 * #BLOCK_SIZE is the number of bytes per logical block on the storage medium.  Only 512 has been
 * tested but other sizes are theoretically possible.  This affects both the amount of data read
 * and written in one block opperation and the addressing i.e. block index 2 is 2 * #BLOCK_SIZE
 * bytes into the volume.
 **/ 
#define BLOCK_SIZE 512

/**
 * \typedef typedef uint32_t blockno_t
 *
 * Block number type is defined here for portability, if you're expecting to have more than 2TB
 * with a 512 byte block size you need to use 64bit block numbers.  Default is 32 bit for speed
 * on 32 bit micros.
 **/
typedef uint32_t blockno_t;
#define MAX_BLOCK 0xFFFFFFFF

/**
 * \brief Any setup needed by the driver.
 * 
 * This function must be called before any others, this can be used to intialise any persistant
 * structures or counters and can be used to start the hardware driver (e.g. power up the SD card
 * and read the card geometry.)
 * 
 * \return 0 on success, anything else to indicate error.
 **/
int block_init();

/**
 * \brief Halt the block driver.
 * 
 * This function shall safely unmount and stop any block device.  Buffers will be flushed, and
 * hardware may be stopped (e.g. an SPI peripheral powered down).  Can be called from an unmount()
 * routine to put the device to sleep.  Any future calls to the block driver must be preceeded by
 * a call to block_init() again.
 * 
 * \return 0 on success, other values indicate an error.
 **/
int block_halt();

/**
 * \brief Read the specified block number into memory at the given address.
 * 
 * Reads a contiguous block of #BLOCK_SIZE bytes from the medium into a pre-allocated area of
 * memory passed to the function by the caller.
 * 
 * \param block is the block number, this is block * #BLOCK_SIZE bytes from the start of the volume
 * \param buf is a pointer to #BLOCK_SIZE bytes already allocated in memory
 * \return 0 on success, anything else may indicate an error.
 **/
int block_read(blockno_t block, void *buf);

/**
 * \brief Write a block from memory to the volume at the specified block address.
 * 
 * Writes #BLOCK_SIZE bytes from memory at the location indicated to the disk block indicated in
 * the call.
 * 
 * \param block is the block number to write to.
 * \param buf is a pointer to #BLOCK_SIZE bytes to be written to the volume
 * \return 0 on success, anything else to indicate an error.
 **/
int block_write(blockno_t block, void *buf);

/**
 * \brief Get the size of the volume which contains the filesystem in blocks.
 * 
 * Returns the total number of blocks in the device containing the filesystem.  Note this is the
 * device not the partition so it will return for example the total number of blocks on an SD
 * card.
 * 
 * \return Number of blocks in total on the disk.
 **/
blockno_t block_get_volume_size();

/**
 * \brief Returns the compiled value of #BLOCK_SIZE
 * 
 * \return The value of #BLOCK_SIZE
 **/
int block_get_block_size();

/**
 * \brief Find out if the volume is mounted as read only.
 * 
 * This depends on the driver implementation, for example full size SD/MMC cards have a read only
 * switch that can be read by hardware, otherwise it could be marked as read only at mount.
 * 
 * \return non zero to indicate true (i.e. read-only) zero to indicate false (writeable).
 **/
int block_get_device_read_only();

/**
 * \brief Get error description from the block driver layer.
 *
 * This is somewhat like errno at the filesystem layer but for the block device, can return
 * invalid card type etc.
 *
 * \return non zero to indicate an error, errors are block driver specific.
 **/
int block_get_error();

#endif /* ifndef BLOCK_H */
