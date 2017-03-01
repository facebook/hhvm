#!/usr/bin/env python
#
# This script processes the output of 'perf script' and prints raw data in the
# the following format:
#
# BINARY_NAME EVENT NUM_IPS
# INSTRUCTION_PTR_1
# INSTRUCTION_PTR_2
# ...
# INSTRUCTION_PTR_NUM_IPS
#
# The perf data may be recorded with -g or not.
#

import string, sys

#
# Positions of various fields within a line.  Assumes lines look like this:
# BINARY  EVENT    IP     FUNCTION
# hhvm    cycles:  d87415 HPHP::ObjectData::destruct()
#
BINARY_NAME_FIELD = 0
EVENT_FIELD = 1
IP_FIELD = 2
FUNCTION_FIELD = 3

def remove_raw_annotation(header):
    """For non-generic events, perf adds a 'raw' annotation before the
       event id: remove it and return the modified header."""
    fields = header.split(None)
    if (fields[EVENT_FIELD] == 'raw'):
      del fields[4]
    return ' '.join(fields)

def extract_function(long_symbol):
    short_symbol = long_symbol.split('(', 1)[0]
    if short_symbol == "":
        return "unknown"
    return short_symbol

def process(record):
    if state == WITH_STACKTRACES:
        header = record[0]
    else:
        header = record

    header = remove_raw_annotation(header)

    fields            = header.split(None)
    binary_name       = fields[BINARY_NAME_FIELD]
    event             = fields[EVENT_FIELD][:-1]
    trace_entries     = []

    if state == NORMAL:
        # perf record was called without "-g"
        try:
            entry = (int(fields[IP_FIELD], 16),
                     extract_function(fields[FUNCTION_FIELD]))
            trace_entries.append(entry)
        except ValueError:
            # Sometimes you get weird garbage from perf script (This
            # fix is motivated by a record that had spaces in the
            # binary name, sigh).  Ignore and move on.
            return
    elif state == WITH_STACKTRACES:
        # perf record was called with "-g"
        if len(record) == 1:
            return
        for line in record[1:]:
            line_fields = line.split(None)
            entry = (int(line_fields[0], 16),
                     extract_function(line_fields[1]))
            trace_entries.append(entry)

    print("%s %s %u" % (binary_name, event, len(trace_entries)))
    for entry in trace_entries:
        print("0x%lx %s" % (entry[0], entry[1]))


NORMAL            = 0
WITH_STACKTRACES  = 1

def records():
    global state

    # Skip comment lines.
    while True:
        line = sys.stdin.readline()
        if (not line):
            return
        if (line[0] != '#'):
            break

    if len(line.split(None)) >= 4:
        state = NORMAL
    else:
        state = WITH_STACKTRACES

    if state == NORMAL:
        yield line
        for line in sys.stdin:
            if (line[0] == "#"):
                continue
            yield line
        return

    # state == WITH_STACKTRACES
    lines = [line]
    for line in sys.stdin:
        if (line[0] == "#"):
            continue
        if line == '\n':
            yield lines
            lines = []
        else:
            lines.append(line)
    if (len(lines) > 0):
        yield lines

if __name__ == '__main__':
    for record in records():
        process(record)
