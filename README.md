# Pack Rat archive format #

Pack Rat is a compact archive format designed for storing large amounts of small files.

Stored files are referred to by their number, starting from zero. No filenames or other metadata is stored, nor is any kind of compression or other modification done.

The format is designed for permanent archival. The ability to modify existing data is limited, and is currently not implemented in this library. If extensive modification is required, recreating the entire archive is likely the best option.

## Variants ##

Pack Rat archives come in two variants: Zero and Compact.

The Zero variant is so called because it allows storing 0-size placeholder files. This may be useful if a particular file in a set is missing and may be added later.

The Compact variant lacks the placeholder-capability, but in addition to being slightly smaller it has the advantage of having no limit on the size of individual archived files.

## Settings ##

Size limits must be chosen for the archive during creation. The only way to change these limits later is by recreating the index.

Both formats require the bitsPos parameter. This limits the maximum overall size of the archive. Valid values are from 16 (64 KiB) to 47 (128 TiB).

The Zero format additionally requires the bitsLen parameter. This determines the maximum size of individual archived files. Valid values are from 2 (4 bytes) to 32 (4 GiB). A value of zero enables the Compact mode.

## Format ##

Each archive consists of two files: a small index file (.pri) and the data-holding file (.prd).

Both files begin with an 8-byte header:
- The file signature, consisting of the letters 'PR'
- A 32-bit individual archive identifier, helping ascertain two files belong to the same set
- A 5-bit unsigned integer: bitsPos - how many bits to use for the position value (0-31, interpreted as 16-47)
- A 5-bit unsigned integer: bitsLen - how many bits to use for the length value (0 for Compact; 1-31 for Zero, interpreted as 2-32)
- A 5-bit currently unused field
- A 1-bit field indicating whether to allow existing files to be modified (0: forbid, 1: allow; not currently implemented)

The index file records the position of each file. In the Zero variant, the size of each file is also recorded.

The data file holds the data of the archived files.
