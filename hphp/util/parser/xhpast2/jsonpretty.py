#!/usr/bin/python

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import fileinput
import sys

indent = -2

def process(c):
    global indent
    if c == '[':
        indent = indent + 2
        sys.stdout.write('\n')
        sys.stdout.write(' ' * indent)
    sys.stdout.write(c)
    if c == ']':
        indent = indent - 2

for line in fileinput.input():
    for c in line:
        process(c)
