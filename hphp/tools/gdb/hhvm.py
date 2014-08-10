"""
Toplevel include for all HHVM GDB Python bindings.
"""
# @lint-avoid-python-3-compatibility-imports

from os import sys, path

# GDB and Python modules don't play well together.
_localdir = path.dirname(path.realpath(path.expanduser(__file__)))
if sys.path[0] != _localdir:
    sys.path.insert(0, _localdir)

import hhbc
import pretty
