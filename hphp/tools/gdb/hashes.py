"""
Hash functions for use with container accessors.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# Hash implementations.

def hash_int64(key):
    ull = T('unsigned long long')
    key = key.cast(T('long long'))

    key = (~key) + (key << 21)
    key = key ^ (key >> 24).cast(ull);
    key = (key + (key << 3)) + (key << 8)
    key = key ^ (key >> 14).cast(ull);
    key = (key + (key << 2)) + (key << 4)
    key = key ^ (key >> 28).cast(ull);
    key = key + (key << 31);

    return -key if key < 0 else key;

def hash_ctca(ctca):
    return ctca.cast(T('uintptr_t'))


#------------------------------------------------------------------------------
# Hash dispatcher.

hashes = {
    'int64_t':          hash_int64,
    'uint64_t':         hash_int64,
    'long long':        hash_int64,
    'HPHP::jit::CTCA':  hash_ctca,
}

def hash_of(value):
    t = value.type

    for (htype, hfunc) in hashes.iteritems():
        try:  # Skip over nonexistent types.
            if t == T(htype):
                return hfunc(value)
        except: pass

    return value.cast(T('size_t'))
