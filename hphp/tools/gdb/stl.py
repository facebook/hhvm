"""
Helpers for accessing C++ STL containers in GDB.
"""
# @lint-avoid-python-3-compatibility-imports

import gdb


#------------------------------------------------------------------------------

def vector_at(vec, idx):
    return vec['_M_impl']['_M_start'][idx]
