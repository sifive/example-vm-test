# Copyright 2019 SiFive, Inc #
# SPDX-License-Identifier: Apache-2.0 #

PROGRAM ?= example-vm-test

$(PROGRAM): $(wildcard *.c) $(wildcard *.h) $(wildcard *.S)

clean:
	rm -f $(PROGRAM) $(PROGRAM).hex
