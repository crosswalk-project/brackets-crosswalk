#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ ! -x "$1" -o ! -d "$2" ]; then
    echo "Run Brackets using Crosswalk"
    echo "Usage: $0 /path/to/xwalk/executable /path/to/brackets/directory"
    exit 1
fi

XWALK_EXECUTABLE="$1"
BRACKETS_PATH="$2"

if [ ! -e "$BRACKETS_PATH/src/index.html" ]; then
    echo "'$BRACKETS_PATH' does not look like a valid Brackets directory!"
    exit 1
fi

BRACKETS_EXT_PATH="$BRACKETS_PATH/src/extensions/brackets.so"

if [ ! -x "$BRACKETS_EXT_PATH" ]; then
    echo "Brackets directory does not contain extension. Please see README."
    exit 1
fi

if [ "brackets.so" -nt "$BRACKETS_EXT_PATH" ]; then
    echo "---------------------------------------------------------"
    echo "Running with an extension older than the currently built."
    echo "Please update $BRACKETS_EXT_PATH."
    echo "---------------------------------------------------------"
fi

exec $XWALK_EXECUTABLE \
	--remote-debugging-port=9234 \
	--disable-web-security \
	--allow-file-access-from-files \
	$BRACKETS_PATH/src/index.html
