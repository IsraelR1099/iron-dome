#!/bin/bash

if [ -z "$1" ]; then
	echo "Usage: $0 <path_to_monitor>"
	exit 1
fi

MONITORED_PATH="$1"
LOG_FILE="irondome.log"
IRON_DOME="./iron-dome.py"

if [ -f "$MONITORED_PATH" ]; then
	echo "Simulating disk read abuse on file $MONITORED_PATH..."
	for i in {1..20}; do
		cat "$MONITORED_PATH" > /dev/null
	done
elif [ -d "$MONITORED_PATH" ]; then
	echo "Simulating disk read abuse on directory $MONITORED_PATH..."
	for i in {1..20}; do
		find "$MONITORED_PATH" -type f -exec cat {} \; > /dev/null 2>&1
	done
else
	echo "Invalid path: $MONITORED_PATH"
	exit 1
fi

sleep 30

if grep -q "WARNING: Possible disk read abuse detected on file" "$LOG_FILE"; then
	echo -e "\033[0;32mTest passed: Disk read abuse detected in $MONITORED_PATH\033[0m"
else
	echo -e "\033[0;31mTest failed: Disk read abuse not detected in $MONITORED_PATH\033[0m"
fi

echo "Simulating changes in the entropy of $MONITORED_PATH..."
./Stockholm/stockholm.py

sleep 30

if grep -q "WARNING: entropy has changed on file" "$LOG_FILE"; then
	echo -e "\033[0;32mTest passed: Entropy changes detected in $MONITORED_PATH\033[0m"
else
	echo -e "\033[0;31mTest failed: Entropy changes not detected in $MONITORED_PATH\033[0m"
fi

sleep 30

echo "Simulating crypto usage and CPU abuse..."
./test_crypto.py

sleep 30

if grep -q "WARNING: The followed process is using a lot of CPU and crypto libraries" "$LOG_FILE"; then
	echo -e "\033[0;32mTest passed: Crypto usage detected\033[0m"
else
	echo -e "\033[0;31mTest failed: Crypto usage not detected\033[0m"
fi
