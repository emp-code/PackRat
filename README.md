Introduction
====

Pack Rat is an archival format designed for permanent storage of large numbers of small files.

The original usecase is storing hundreds of millions of small images relating to database entries.

The design priorities are, in rough order: compactness, simplicity and speed.

A Pack Rat archive consists of two files:
* .prd (Pack Rat Data), which stores the contents of the files, one after another.
* .pri (Pack Rat Index), which stores the locations where files start and end.

Stored files are referenced by ID numbers in the order they were added, starting from zero. No filenames or metadata are stored.

No seeking is done. Only bytes relating to the requested file are read.

No compression is done, the files are stored as they are.

The format is designed as append-only. Small changes may be possible, but major changes require recreating the archive.

Archive Types & Settings
====

Two archive types exist: Zero (0) and Compact (C).

Zero is universal and full-featured. It's fairly well tested and is under active use.

Compact lacks some features and is poorly tested, but is even more compact than Zero, and also supports files of any size.

Zero requires two settings:
* PosBits (position bits): determines the maximum archive (.prd) size.
* LenBits (length bits): determines the maximum size of an individual file inside the archive.

The limits can be calculated simply: 2 ^ [number of bits] = [maximum size in bytes]

Compact only uses the PosBits setting. It has no limit on individual file size.

Example settings:
* PosBits=40, which limits the .prd file to a maximum of 1 TiB.
* LenBits=16, which limits individual files to a maximum of 64 KiB.

These add up to 56 bits, which means each entry takes 7 bytes (56/8) in the index file.

Smaller numbers mean less space taken per entry, while larger numbers have the advantage of allowing more or larger files.

The number of bits should add up to a number divisible by 8, to avoid wasting bits.

Usage Examples
====

Create a new Pack Rat Zero archive under the files 'example.prd' and 'example.pri', using 40 bits for the position and 16 bits for the length:

`packrat --create --data=example.prd --index=example.pri --posbits=40 --lenbits=16 --type=0`

Write the file 'example.jpg' to 'example.prd' and 'example.pri':

`packrat --write --data=example.prd --index=example.pri --file=example.jpg`

Read file number 42 from 'example.prd' and 'example.pri' (starts from 0, hence this is the 43rd file):

`packrat --read --data=example.prd --index=example.pri --num=42 --file=test.jpg`

Replace file number 25 with 'sample.jpg' in 'example.prd' and 'example.pri' (see Updating Data below for important details)

`packrat --update --data=example.prd --index=example.pri --num=25 --file=sample.jpg`

Updating Data
====

Pack Rat Zero supports updating (replacing) files, with the following limitations:
* If the replacement data is larger: the old data will remain, but cannot be accessed through Pack Rat
* If the replacement data is smaller: part of the old data will remain, but cannot be accessed through Pack Rat
* If the sizes match: the old data will be replaced by the new

Recreating the entire archive can be done to bypass these limitations.

Index (.pri) Format
====

The first five bytes of the .pri (Pack Rat Index) file contain the archive header. The five bytes are:
1. 'P' (file signature)
2. 'R' (file signature)
3. '0' or 'C'  (archive type)
4. Number of bits to use for position
5. Number of bits to use for size (not used for Compact)

After this, the file entries begin. Each entry takes the same number of bytes. The size of entries depends on the settings (type, number of bits).
