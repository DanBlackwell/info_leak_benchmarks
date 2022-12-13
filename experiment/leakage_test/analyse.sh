#!/usr/bin/env bash

for F in */; do 
	if [[ -f $F/fuzz.sh ]]; then 
		cd $F
		echo $F 
		if ls OUT/default/crashes/id* 1>/dev/null 2>&1; then 
			echo "CRASH FOUND $(ls OUT/default/crashes/id* | head -n 1)"

			for CRASH in OUT/default/crashes/id*; do
				:
				# ./fuzz $CRASH
			done
	       	fi
	       	cd .. 
	fi 
done
