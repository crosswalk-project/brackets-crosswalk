# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SOURCES = extension.cc platform_gtk.cc
OBJECTS = extension.o platform_gtk.o
CFLAGS = -Wall -Wextra -fPIC `pkg-config gtk+-2.0 --cflags`
LDFLAGS = `pkg-config gtk+-2.0 --libs`
EXTENSION_BASE = brackets
EXTENSION = $(EXTENSION_BASE).so
JSAPI = $(EXTENSION_BASE)_api.h
JSAPISRC = $(EXTENSION_BASE)_api.js

all: $(SOURCES) $(EXTENSION)

$(EXTENSION): $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) -shared -o $(EXTENSION) $(LDFLAGS)

.cc.o:
	$(CXX) $(CFLAGS) $< -c -o $@

extension.o: $(JSAPI)

$(JSAPI): $(JSAPISRC)
	hexdump -v -e '10/1 "0x%02x, "' -e '"\n"' $(JSAPISRC) | sed 's/0x  ,//g' > $(JSAPI).tmp
	echo "static const char kGeneratedSource[] = {" > $(JSAPI)
	cat $(JSAPI).tmp >> $(JSAPI)
	rm -f $(JSAPI).tmp
	echo "};" >> $(JSAPI)

clean:
	rm -f $(EXTENSION) $(OBJECTS) $(JSAPI) *~
