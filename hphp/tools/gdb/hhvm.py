"""
Toplevel include for all HHVM GDB Python bindings.

Due to some GDB Python API shenanigans, we have to munge the directory
containing these scripts into sys.path.  We only do this here, so attempting to
source the individual modules directly will fail.
"""
# @lint-avoid-python-3-compatibility-imports

from os import sys, path

# GDB doesn't add the script's dir to sys.path when sourcing.
localdir = path.dirname(path.realpath(path.expanduser(__file__)))
if sys.path[0] != localdir:
    sys.path.insert(0, localdir)

import hhbc
import idx
import lookup
import stack
import pretty
import unit
