# Pack Rat Archival System #

Pack Rat is an archival format designed for permanent storage of large numbers of small files.

It's designed for maximum compactness with a simple design, without risking data integrity or compromising performance.

A Pack Rat archive consists of two files:
* .prd (Pack Rat Data), which stores the contents of the files, one after another.
* .pri (Pack Rat Index), which stores the locations where files start and end.

Stored files are referenced by ID numbers in the order they were added, starting from zero. Data is stored without compression or other modification. No filenames or metadata are stored.

The format is designed as append-only. Small changes may be possible, but major changes require recreating the archive.

## Archive types, settings ##

Two archive types exist: Zero (0) and Compact (C).

Zero is the full-featured format, getting its name from its ability to store 0-size (placeholder) entries which can later be replaced.

Compact lacks some features, but is more space-efficient.

Zero requires two settings:
* PosBits (position bits): determines the maximum archive (.prd) size.
* LenBits (length bits): determines the maximum size of an individual file inside the archive.

The limits can be calculated simply: 2 ^ [number of bits] = [maximum size in bytes]

Compact only uses the PosBits setting. It has no limit on individual file size.

Example settings (Zero-type):
* PosBits=40, which limits the .prd file to a maximum of 1 TiB.
* LenBits=16, which limits individual files to a maximum of 64 KiB.

These add up to 56 bits, which means each entry takes 7 bytes (56/8) in the index file.

Smaller numbers mean less space taken per entry, while larger numbers have the advantage of allowing more or larger files.

The number of bits should add up to a number divisible by 8, to avoid wasting bits.

## Usage ##

Create a new Pack Rat Zero archive under the files 'example.prd' and 'example.pri', using 40 bits for the position and 16 bits for the length:

`packrat --create --data=example.prd --index=example.pri --posbits=40 --lenbits=16 --type=0`

Write the file 'test.jpg' to 'example.prd' and 'example.pri':

`packrat --write --data=example.prd --index=example.pri --file=test.jpg`

Read file number 42 from 'example.prd' and 'example.pri' (starts from 0, hence this is the 43rd file):

`packrat --read --data=example.prd --index=example.pri --num=42 --file=test.jpg`

Replace file number 25 with 'test.jpg' in 'example.prd' and 'example.pri' (see Updating Data below for important details)

`packrat --update --data=example.prd --index=example.pri --num=25 --file=test.jpg`

## Updating data ##

Pack Rat Zero supports updating (replacing) files, with the following limitations:
* If the replacement data is larger: the old data will remain, but cannot be accessed through Pack Rat
* If the replacement data is smaller: part of the old data will remain, but cannot be accessed through Pack Rat
* If the sizes match: the old data will be replaced by the new

Recreating the entire archive can be done to bypass these limitations.

## Index (.pri) format ##

The first five bytes of the .pri (Pack Rat Index) file contain the archive header. The five bytes are:
1. 'P' (file signature)
2. 'r' (file signature)
3. '0' or 'C'  (archive type)
4. Number of bits to use for position
5. Number of bits to use for size (not used for Compact)

After this, the file entries begin. Each entry takes the same number of bytes. The size of entries depends on the settings (type, number of bits).
