#!/bin/bash

# Limit Tester for Pack Rat v3

# Creates a full Pack Rat archive and verifies its integrity.

#cmd="valgrind -q --track-origins=yes --leak-check=full --exit-on-first-error=yes --error-exitcode=1 ./packrat"
cmd="./packrat"

if [ $(( $RANDOM % 2 )) -eq 1 ]; then
	bitsLen=$(shuf -i 14-16 -n 1)
	bitsPos=$(shuf -i 17-24 -n 1)

	totalSize=$(echo "2 ^ $bitsPos" | bc)
	baseFileSize=$(echo "(2 ^ $bitsLen) - 1" | bc)
	lastFileSize=$(echo "$totalSize % $baseFileSize" | bc)
	baseFileCount=$(echo "($totalSize - $lastFileSize) / $baseFileSize" | bc)

	if [ $(echo "($baseFileCount * $baseFileSize) + $lastFileSize" | bc) -ne $totalSize ]; then
		echo "Zero: Size miscalculated"
		exit
	fi

	echo "Testing bitsPos=$bitsPos; bitsLen=$bitsLen; $baseFileCount files @ $baseFileSize bytes each + one file @ $lastFileSize."
else
	bitsLen=0
	bitsPos=$(shuf -i 16-24 -n 1)
	fileCount=8
	fileSize=$(echo "(2 ^ $bitsPos) / $fileCount" | bc)

	if [ $(echo "$fileCount * $fileSize" | bc) -ne $(echo "2 ^ $bitsPos" | bc) ]; then
		echo "Compact: Size miscalculated"
		exit
	fi

	echo "Testing bitsPos=$bitsPos; bitsLen=$bitsLen; $fileCount files @ $fileSize bytes each."
fi


if [ $(( $RANDOM % 2 )) -eq 1 ]; then
	$cmd --create --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --posbits=$bitsPos --lenbits=$bitsLen
else
	$cmd -c -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -p $bitsPos -l $bitsLen
fi

head -c 8 "/tmp/packrat-$$.prd" > "/tmp/packrat-$$.cmp"

if [ $bitsLen -eq 0 ]; then
	# Compact
	for i in $(seq 1 $fileCount); do
		head -c $fileSize /dev/urandom > "/tmp/packrat-$$.tmp.$i"

		if [ $(( $RANDOM % 2 )) -eq 1 ]; then
			$cmd --add --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --file="/tmp/packrat-$$.tmp.$i"
		else
			$cmd -a -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -f "/tmp/packrat-$$.tmp.$i"
		fi

		cat "/tmp/packrat-$$.tmp.$i" >> "/tmp/packrat-$$.cmp"
	done
else
	# Zero
	for i in $(seq 1 $baseFileCount); do
		head -c $baseFileSize /dev/urandom > "/tmp/packrat-$$.tmp.$i"

		if [ $(( $RANDOM % 2 )) -eq 1 ]; then
			$cmd --add --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --file="/tmp/packrat-$$.tmp.$i"
		else
			$cmd -a -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -f "/tmp/packrat-$$.tmp.$i"
		fi

		cat "/tmp/packrat-$$.tmp.$i" >> "/tmp/packrat-$$.cmp"
	done

	head -c $lastFileSize /dev/urandom > "/tmp/packrat-$$.tmp.z"
	cat "/tmp/packrat-$$.tmp.z" >> "/tmp/packrat-$$.cmp"

	if [ $(( $RANDOM % 2 )) -eq 1 ]; then
		$cmd --add --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --file="/tmp/packrat-$$.tmp.z"
	else
		$cmd -a -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -f "/tmp/packrat-$$.tmp.z"
	fi

	fileCount=$baseFileCount
fi

if ! cmp -s "/tmp/packrat-$$.cmp" "/tmp/packrat-$$.prd"; then
	ls -l "/tmp/packrat-$$.cmp" "/tmp/packrat-$$.prd"
	echo "FAIL: PRD does not match"
else
	# Test each file individually
	for i in $(seq 1 $fileCount); do
		let n=$i-1

		if [ $(( $RANDOM % 2 )) -eq 1 ]; then
			$cmd --get --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --num=$n --file="/tmp/packrat-$$.chk.$i"
		else
			$cmd -g -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -n $n -f "/tmp/packrat-$$.chk.$i"
		fi

		if ! cmp -s "/tmp/packrat-$$.chk.$i" "/tmp/packrat-$$.tmp.$i"; then
			echo "FAIL: extracted file $i does not match"
			ls -l "/tmp/packrat-$$.chk.$i" "/tmp/packrat-$$.tmp.$i"
			break;
		fi
	done

	if [ $bitsLen -ne 0 ]; then
		if [ $(( $RANDOM % 2 )) -eq 1 ]; then
			$cmd --get --data="/tmp/packrat-$$.prd" --index="/tmp/packrat-$$.pri" --num=$fileCount --file="/tmp/packrat-$$.chk.z"
		else
			$cmd -g -d "/tmp/packrat-$$.prd" -i "/tmp/packrat-$$.pri" -n $fileCount -f "/tmp/packrat-$$.chk.z"
		fi

		if ! cmp -s "/tmp/packrat-$$.chk.z" "/tmp/packrat-$$.tmp.z"; then
			echo "FAIL: last extracted file does not match"
			ls -l "/tmp/packrat-$$.chk.z" "/tmp/packrat-$$.tmp.z"
		fi

		rm "/tmp/packrat-$$.chk.z" "/tmp/packrat-$$.tmp.z"
	fi
fi

# Cleanup
rm "/tmp/packrat-$$.pri"
rm "/tmp/packrat-$$.prd"
rm "/tmp/packrat-$$.cmp"

for i in $(seq 0 $count); do
	rm "/tmp/packrat-$$.chk.$i" "/tmp/packrat-$$.tmp.$i"
done
