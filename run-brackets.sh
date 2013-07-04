#!/bin/bash
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [ ! -x "$1" -o ! -d "$2" ]; then
    echo "Run Brackets using Crosswalk"
    echo "Usage: $0 /path/to/xwalk/executable /path/to/brackets/directory"
    exit 1
fi

CURRENT_DIRECTORY=$(realpath $(dirname $0))
XWALK_EXECUTABLE="$1"
BRACKETS_PATH="$2"

if [ ! -e "$BRACKETS_PATH/src/index.html" ]; then
    echo "'$BRACKETS_PATH' does not look like a valid Brackets directory!"
    exit 1
fi

if [ ! -x "$CURRENT_DIRECTORY/brackets.so" ]; then
    echo "Brackets extension not found, trying to build."
    if ! make; then
        echo "Couldn't build extension, please file a bug report at"
        echo "  https://github.com/otcshare/brackets-crosswalk"
        exit 1
    fi
fi

exec $XWALK_EXECUTABLE \
	--remote-debugging-port=9234 \
	--disable-web-security \
	--allow-file-access-from-files \
	--external-extensions-path=$CURRENT_DIRECTORY \
	$BRACKETS_PATH/src/index.html
