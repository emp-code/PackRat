#!/bin/bash

# Simple automated test for Pack Rat

for i in {0..100}; do
	if [ $(( $RANDOM % 2 )) -eq 0 ]; then
		t="0"
		posBits=$(( $RANDOM % 57 + 8 ))
		lenBits=$(( $RANDOM % 57 + 8 ))

		let totalBits=$posBits+$lenBits
	else
		t="C"
		posBits=$(( $RANDOM % 57 + 8 ))
		lenBits=0
		totalBits=$posBits
	fi

	infoBytes=$(awk -vnumber=$totalBits -vdiv=8 'function ceiling(x){return x%1 ? int(x)+1 : x} BEGIN{ print ceiling(number/div) }')

	size1=$(( $RANDOM % 40 + 5 ))
	size2=$(( $RANDOM % 40 + 5 ))
	let "total = size1 + size2"

	echo "Testing with Type $t using $posBits for position and $lenBits for length and file sizes of $size1 and $size2"

	head -c $size1 /dev/urandom > "/tmp/packrat-$$.tmp.0"
	head -c $size2 /dev/urandom > "/tmp/packrat-$$.tmp.1"

	./packrat --create --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" --posbits=$posBits --lenbits=$lenBits --type=$t
	./packrat --write --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" -f "/tmp/packrat-$$.tmp.0"
	./packrat --write --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" -f "/tmp/packrat-$$.tmp.1"

	# .prd should match a combination of the two test files
	cat "/tmp/packrat-$$.tmp.0" "/tmp/packrat-$$.tmp.1" > "/tmp/packrat-$$.tmp"
	cmp "/tmp/packrat-$$.tmp" "/tmp/test-$$.prd"
	rm "/tmp/packrat-$$.tmp"

	# .pri size should be 5 + (infoBytes * 2)
	priSize=$(wc -c "/tmp/test-$$.pri" | sed 's/ .*//')
	let "priBytes = 5 + (infoBytes * 2)"
	if [ $priSize -ne $priBytes ]; then echo ".pri size ($priSize) does not match expected value ($priBytes)"; fi

	./packrat --read --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" --num=0 --file="/tmp/packrat-$$.out"
	cmp "/tmp/packrat-$$.tmp.0" "/tmp/packrat-$$.out"
	rm "/tmp/packrat-$$.out"

	./packrat --read --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" --num=1 --file="/tmp/packrat-$$.out"
	cmp "/tmp/packrat-$$.tmp.1" "/tmp/packrat-$$.out"
	rm "/tmp/packrat-$$.out"

	# Data replacement test
	if [ $t == "0" ]; then
		./packrat --update --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" --num=1 --file="/tmp/packrat-$$.tmp.0"

		./packrat --read --data="/tmp/test-$$.prd" --index="/tmp/test-$$.pri" --num=1 --file="/tmp/packrat-$$.out"
		cmp "/tmp/packrat-$$.tmp.0" "/tmp/packrat-$$.out"
		rm "/tmp/packrat-$$.out"
	fi

	rm "/tmp/packrat-$$.tmp.0" "/tmp/packrat-$$.tmp.1" "/tmp/test-$$.prd" "/tmp/test-$$.pri"
done
