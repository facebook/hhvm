"""
Assorted utilities for HHVM GDB bindings.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

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


def gdbprint(val, ty=None):
    if ty is None:
        ty = val.type
    gdb.execute('print (%s)%s' % (str(ty), str(val)))


#------------------------------------------------------------------------------
# String helpers.


def string_data_val(val):
    return val['m_data'].string('utf-8', 'ignore', val['m_len'])


def vstr(value):
    """Stringify a value without pretty-printing."""

    for pp in gdb.pretty_printers:
        try:
            pp.saved = pp.enabled
        except AttributeError:
            pp.saved = True

        pp.enabled = False

    try:
        ret = unicode(value)
    except:
        ret = str(value)

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


#------------------------------------------------------------------------------
# Type manipulations.

def template_type(t):
    """Get the unparametrized name of a template type."""
    return str(t).split('<')[0]


def is_ref(t):
    """Return whether a type `t' is a C++ pointer or reference type."""
    return (t.code == gdb.TYPE_CODE_PTR or
            t.code == gdb.TYPE_CODE_REF)


def deref(val):
    """Fully dereference a value, stripping away *, &, and all known smart
    pointer wrappers (as well as const/volatile qualifiers)."""

    while True:
        t = val.type.unqualified().strip_typedefs()

        if is_ref(t):
            val = val.referenced_value()
            continue

        name = template_type(t)

        if name == "HPHP::LowPtr":
            inner = t.template_argument(0)
            val = val['m_raw'].cast(inner.pointer()).dereference()
            continue

        if name == "HPHP::SmartPtr" or name == "HPHP::AtomicSmartPtr":
            val = val['m_px'].dereference()
            continue

        return val
