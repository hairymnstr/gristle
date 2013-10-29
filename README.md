Gristle
=======

An open source FAT16/32 filesystem driver for small, bare-metal systems especially microcontrollers.
This library is licensed under a BSD license, you may use it under the terms found in COPYING.

Structure
---------

The filesystem driver itself is contained within the a single source file ``gristle.c``.  This
relies upon a generic block driver which can provide a 512 byte block from the volume containing
the filesystem.  This driver may for example request a block from an SD card.  See ``block.h`` for 
the common function definitions that must be provided by the block driver.

There are two examples of block drivers in the ``src/block_driver`` folder, ``block_sd.c`` is an 
implementation of an SD card block driver designed to run an STM32F103 microcontroller using the 
[libopencm3](http://libopencm3.org) hardware library. ``block_pc.c`` is an implementation mainly
used for testing on a Linux host, it is designed to allow reading/writing from a FAT filesystem
image in a file on the host.  The PC driver also contains some tools to snapshot and generate MD5
hashes for testing.

The library is designed to be called from a UNIX style C library for example 
[newlib](http://www.sourceware.org/newlib/) where there are POSIX compliant ``_open()`` and 
``_write()`` calls etc.  The binding between Gristle and the C library can be seen in a typical
``syscalls.c`` file in the 
[oggbox project](https://github.com/hairymnstr/tree/master/src/syscalls.c).

There is also a handler for MBR type primary partition tables in ``partition.c`` which can be used
in an embedded system to identify partitions within a volume.

History
-------

This driver has been developed for the [OggBox](http://oggbox.nathandumont.com) project and as such 
has focused on fast file reading and has some unusual optimisations (seeking backwards in a file is 
particularly fast for example) which are to do with opperations on media files (in that case finding
the play time of an ogg file).  Some effort has been made to do unit testing using the PC driver
which is the basis of the writing routines, but this is still sparse.

Author
------

Gristle is written by Nathan Dumont <nathan@nathandumont.com>.
