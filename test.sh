#!/bin/bash

# Tester for Pack Rat v3

# 1. Asks Pack Rat to create an archive with randomly chosen parameters
# 2. Asks Pack Rat to add randomly generated files to the archive
# 3. Compares the Pack Rat PRD file against the correct/expected data
# 4. Asks Pack Rat to extract each individual file, comparing them against the originals

#cmd="valgrind -q --track-origins=yes --leak-check=full --exit-on-first-error=yes --error-exitcode=1 ./packrat"
cmd="./packrat"

count=$(shuf -i 1-30 -n 1)

if [ $(( $RANDOM % 2 )) -eq 1 ]; then
	while true; do
		bitsLen=$(shuf -i 8-32 -n 1)
		bitsPos=$(shuf -i 16-47 -n 1)
		if [ $bitsPos -gt $bitsLen ]; then break; fi
	done
	maxSize=$(echo "2 ^ ($bitsLen) - 4" | bc)
else
	bitsLen=0
	bitsPos=$(shuf -i 16-47 -n 1)
	maxSize=$(echo "(2 ^ ($bitsPos - 4))" | bc)
fi

if [ $maxSize -gt 1234567 ]; then maxSize=1234567; fi

echo "Testing bitsPos=$bitsPos; bitsLen=$bitsLen; $count files."

$cmd --create --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --posbits=$bitsPos --lenbits=$bitsLen

head -c 8 "/tmp/packrat-$$.pri" > "/tmp/packrat-$$.cmp"

for i in $(seq 0 $count); do
	size=$(shuf -i 1-$maxSize -n 1)
	head -c $size /dev/urandom > "/tmp/packrat-$$.tmp.$i"
	$cmd --write --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" -f "/tmp/packrat-$$.tmp.$i"
	cat "/tmp/packrat-$$.tmp.$i" >> "/tmp/packrat-$$.cmp"
done

cmp "/tmp/packrat-$$.cmp" "/tmp/packrat-$$.prd"
if [ $? -ne 0 ]; then
	ls -l "/tmp/packrat-$$.cmp" "/tmp/packrat-$$.prd"
	echo "FAIL: data does not match file contents"
else
	# Test each file individually
	for i in $(seq 0 $count); do
		$cmd --read --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" -n $i -f "/tmp/packrat-$$.chk.$i"

		cmp "/tmp/packrat-$$.chk.$i" "/tmp/packrat-$$.tmp.$i"
		if [ $? -ne 0 ]; then
			echo "FAIL: extracted file $i does not match"
			ls -l "/tmp/packrat-$$.chk.$i" "/tmp/packrat-$$.tmp.$i"
			break;
		fi
	done
fi

# Cleanup
rm "/tmp/packrat-$$.pri"
rm "/tmp/packrat-$$.prd"
rm "/tmp/packrat-$$.cmp"

for i in $(seq 0 $count); do
	rm "/tmp/packrat-$$.tmp.$i"
done
