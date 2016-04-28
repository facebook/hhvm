"""
Hash functions for use with container accessors.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# Hash implementations.

def hash_id(key):
    return key.cast(T('size_t'))

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

hash_funcs = {
    'id':   { 'cast': None,                 'func': hash_id },
    'int':  { 'cast': 'int64_t',            'func': hash_int64 },
    'ctca': { 'cast': 'HPHP::jit::CTCA',    'func': hash_ctca },
}

hashes = {
    'int64_t':          hash_int64,
    'uint64_t':         hash_int64,
    'long long':        hash_int64,
    'HPHP::jit::CTCA':  hash_ctca,
}

def hash_of(value, hasher=None):
    t = value.type

    if hasher is not None and hasher in hash_funcs:
        h = hash_funcs[hasher]
        if h['cast'] is not None:
            value = value.cast(T(h['cast']))
        return h['func'](value)

    for (htype, hfunc) in hashes.items():
        try:  # Skip over nonexistent types.
            if t == T(htype):
                return hfunc(value)
        except: pass

    return value.cast(T('size_t'))
