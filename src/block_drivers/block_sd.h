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

#ifndef BLOCK_SD_H
#define BLOCK_SD_H 1

/**
 *  Platform independent definitions
 */

/* Constants used to define card types */
#define SD_CARD_NONE  0    /* No SD card found */
#define SD_CARD_MMC   1    /* MMC card */
#define SD_CARD_SC    2    /* Standard capacity SD card (up to 2GB)*/
#define SD_CARD_HC    3    /* High capacity SD card (4GB to 32GB)*/
#define SD_CARD_XC    4    /* eXtended Capacity SD card (up to 2TB  - Untested may work if FAT32) */
#define SD_CARD_ERROR 99   /* An error occured during setup */

/* SPI commands */
#define CMD0          0
#define CMD1          1
#define CMD8          8
#define CMD9          9
#define CMD10         10
#define CMD12         12
#define CMD17         17
#define CMD18         18
#define CMD24         24
#define ACMD41        0x80 + 41

/* Error status codes returned in the SD info struct */
#define SD_ERR_NO_PART      1
#define SD_ERR_NOT_PRESENT  2
#define SD_ERR_NO_FAT       3

#define SD_RETRIES 1000

/* SD card info struct */
typedef struct {
  uint16_t  card_type;
  uint8_t   read_only;
  uint32_t  size;
  uint8_t   error;
} SDCard;

#endif /* ifndef BLOCK_SD_H */
