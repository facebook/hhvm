"""
Assorted utilities for HHVM GDB bindings.
"""
# @lint-avoid-python-3-compatibility-imports

import collections
import functools
import gdb


#------------------------------------------------------------------------------
# Memoization.

def memoized(func):
    """Simple memoization decorator that ignores **kwargs."""

    cache = {}

    @functools.wraps(func)
    def memoizer(*args):
        if not isinstance(args, collections.Hashable):
            return func(*args)
        if args not in cache:
            cache[args] = func(*args)
        return cache[args]
    return memoizer


#------------------------------------------------------------------------------
# Caching lookups.

@memoized
def T(name):
    return gdb.lookup_type(name)

@memoized
def V(name):
    return gdb.lookup_symbol(name)[0].value()

@memoized
def K(name):
    return gdb.lookup_global_symbol(name).value()
