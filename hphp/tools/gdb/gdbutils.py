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

def string_data_val(val, keep_case=True):
    s = val['m_data'].string('utf-8', 'ignore', val['m_len'])
    return s if keep_case else s.lower()


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

def rawtype(t):
    return t.unqualified().strip_typedefs()


def template_type(t):
    """Get the unparametrized name of a template type."""
    return str(t).split('<')[0]


def rawptr(val):
    """Fully strip a smart pointer type to a raw pointer.  References are
    re-cast as pointers."""

    t = rawtype(val.type)

    if t.code == gdb.TYPE_CODE_PTR:
        return val
    elif t.code == gdb.TYPE_CODE_REF:
        return val.referenced_type().address

    name = template_type(rawtype(val.type))

    if name == 'std::unique_ptr':
        return val['_M_t']['_M_head_impl']

    if name == 'HPHP::default_ptr':
        return val['m_p']

    if name == 'HPHP::SmartPtr' or name == 'HPHP::AtomicSmartPtr':
        return val['m_px']

    if name == 'HPHP::LowPtr' or name == 'HPHP::LowPtrImpl':
        inner = t.template_argument(0)
        return val['m_raw'].cast(inner.pointer())

    if name == 'HPHP::CompactTaggedPtr':
        inner = t.template_argument(0)
        return (val['m_data'] & 0xffffffffffff).cast(inner.pointer())

    if name == 'HPHP::CompactSizedPtr':
        return rawptr(val['m_data'])

    return None


def deref(val):
    """Fully dereference a value, stripping away *, &, and all known smart
    pointer wrappers (as well as const/volatile qualifiers)."""

    p = rawptr(val)

    if p is None:
        return val
    else:
        return deref(p.referenced_value())
