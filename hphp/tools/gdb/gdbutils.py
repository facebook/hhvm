"""
Assorted utilities for HHVM GDB bindings.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import collections
import functools
import gdb
import re
import struct
import types


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
# Intel CRC32.

def _bit_reflect(num, nbits):
    """Perform bit reflection on the bottom `nbits' of `num."""
    out = 0
    mask = 1 << (nbits - 1)
    for i in xrange(nbits):
        if num & (1 << i):
            out |= mask
        mask >>= 1
    return out

def _crc32q(crc, quad):
    """Intel SSE4 CRC32 implementation."""

    crc = _bit_reflect(crc, 32)
    quad = _bit_reflect(quad, 64)

    msb = 1 << 63

    dividend = quad ^ (crc << 32)
    divisor = 0x11edc6f41 << 31

    for i in xrange(64):
        if dividend & msb:
            dividend ^= divisor
        dividend <<= 1

    return _bit_reflect(dividend, 64)


#------------------------------------------------------------------------------
# String helpers.

def string_data_val(val, keep_case=True):
    """Convert an HPHP::StringData[*] to a Python string."""

    s = val['m_data'].string('utf-8', 'ignore', val['m_len'])
    return s if keep_case else s.lower()


def _unpack(s):
    return 0xdfdfdfdfdfdfdfdf & struct.unpack('<Q', s)[0]

def hash_string(s):
    """Hash a string as in hphp/util/hash-crc.S."""

    size = len(s)
    tail_sz = size % 8
    size -= tail_sz

    crc = 0xffffffff

    for i in xrange(0, size, 8):
        crc = _crc32q(crc, _unpack(s[i:i+8]))

    if tail_sz == 0:
        return crc >> 1

    shift = -((tail_sz - 8) << 3) & 0b111111
    tail = _unpack(s[size:].ljust(8, '\0'))

    crc = _crc32q(crc, tail << shift)
    return crc >> 1


def strinfo(s, keep_case=True):
    """Return the Python string and HHVM hash for `s', or None if `s' is not a
    stringish gdb.Value."""

    data = None
    addr = None
    h = None

    t = rawtype(s.type)

    if (t == T('char').pointer() or
            re.match(r"char \[\d*\]$", str(t)) is not None):
        data = s.string()
    else:
        sd = deref(s)

        if rawtype(sd.type).name != 'HPHP::StringData':
            return None

        data = string_data_val(sd)

        if int(sd['m_hash']) != 0:
            h = sd['m_hash'] & 0x7fffffff

    if data is None:
        return None

    retval = {
        'data': data if keep_case else data.lower(),
        'hash': h if h is not None else hash_string(str(data)),
    }
    return retval


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
        return val.cast(rawtype(val.type))
    else:
        return deref(p.referenced_value())
