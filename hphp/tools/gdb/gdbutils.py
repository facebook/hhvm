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
# General-purpose helpers.

def parse_argv(args):
    return [gdb.parse_and_eval(arg) for arg in gdb.string_to_argv(args)]


def vstr(value):
    """Stringify a value without pretty-printing."""

    for pp in gdb.pretty_printers:
        try:
            pp.saved = pp.enabled
        except AttributeError:
            pp.saved = True

        pp.enabled = False

    ret = unicode(value)

    for pp in gdb.pretty_printers:
        pp.enabled = pp.saved

    return ret


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
