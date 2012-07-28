#!/usr/bin/env python

# Reads lines of the following form from stdin:
#   TCDump SomeName a3 bd 83
#
# This script then takes those bytes of machine code (expressed in hex) and uses
# gcc + objdump to print human-readable assembly to stdout.  Input lines that
# don't start with TCDump are ignored.
#
# These are generated from the translator by using SpaceRecorder, set to dump
# machine code. These lines will show up in hphp.log; then you can do stuff like
# this:
#
# src$ grep DecRef hphp.log | head -n 1 | ../bin/tcdump.py

import os
import sys
import tempfile


def process_line(line, counter, cfile):
    # Use the name of the label, with a unique int after it, as the fn name
    name = line[0] + str(counter)
    cfile.write("void %s() {" % name)

    # Put a nop before and after the machine code to distinguish it from the
    # function entry/exit stuff gcc puts in
    cfile.write('asm volatile (".byte 0x90,')
    cfile.write(",".join(["0x" + b for b in line[1:]]))
    cfile.write(',0x90");}')


def main():
    # delete=False because we need to use it after closing it
    cfile = tempfile.NamedTemporaryFile(delete=False)
    counter = 0

    # Take each relevant line in the input, and convert it into a C function
    for line in sys.stdin:
        fields = line.strip().split(' ')
        if fields[0] == "TCDump":
            process_line(fields[1:], counter, cfile)
            counter += 1

    cfile.close()

    # gcc -c it and then objdump -d it
    objfile = tempfile.NamedTemporaryFile()
    os.system("gcc -c -x c -o %s %s" % (objfile.name, cfile.name))
    os.system("objdump -d %s" % objfile.name)

    os.unlink(cfile.name)


if __name__ == '__main__':
    main()
