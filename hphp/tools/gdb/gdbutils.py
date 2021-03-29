#!/usr/bin/env python3

"""
Assorted utilities for HHVM GDB bindings.
"""

from compatibility import *

import collections
import functools
import gdb
import re
import string
import struct
import traceback
import types

#------------------------------------------------------------------------------
# Memoization.

_all_caches = []


def memoized(func):
    """Simple memoization decorator that ignores **kwargs."""
    global _all_caches

    cache = {}
    _all_caches.append(cache)

    @functools.wraps(func)
    def memoizer(*args):
        if not isinstance(args, collections.Hashable):
            return func(*args)
        if args not in cache:
            cache[args] = func(*args)
        return cache[args]
    return memoizer


def invalidate_all_memoizers():
    global _all_caches

    for cache in _all_caches:
        cache.clear()


#------------------------------------------------------------------------------
# STL accessors.

def atomic_get(atomic):
    inner = rawtype(atomic.type).template_argument(0)

    if inner.code == gdb.TYPE_CODE_PTR:
        return atomic['_M_b']['_M_p']
    else:
        return atomic['_M_i']


#------------------------------------------------------------------------------
# Exception debugging.

def errorwrap(func):
    @functools.wraps(func)
    def wrapped(*args, **kwds):
        try:
            return func(*args, **kwds)
        except:
            print('')
            traceback.print_exc()
            print('')
            raise
    return wrapped


#------------------------------------------------------------------------------
# General-purpose helpers.

def parse_argv(args, limit=None):
    """Explode a gdb argument string, then eval all args up to `limit'."""
    if limit is None:
        limit = len(args)
    return [gdb.parse_and_eval(arg) if i < limit else arg
            for i, arg in enumerate(gdb.string_to_argv(args))]


def gdbprint(val, ty=None):
    if ty is None:
        ty = val.type
    # quote names with :: in case we're in a non-c++ frame
    ty = re.sub(r'\b(\w*(::\w+(\<.*\>)?)+)\b', r"'\1'", str(ty))
    gdb.execute('print (%s)%s' % (str(ty), str(val)))


def plural_suffix(num, suffix='s'):
    return '' if num == 1 else suffix


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


def crc32q(crc, quad):
    """Intel SSE4 CRC32 implementation."""

    crc = _bit_reflect(crc, 32)
    quad = _bit_reflect(quad, 64)

    msb = 1 << 63

    dividend = quad ^ (crc << 32)
    divisor = 0x11edc6f41 << 31

    for _i in xrange(64):
        if dividend & msb:
            dividend ^= divisor
        dividend <<= 1

    return _bit_reflect(dividend, 64)


#------------------------------------------------------------------------------
# String helpers.

def string_data_val(val, keep_case=True):
    """Convert an HPHP::StringData[*] to a Python string."""

    try:
        data = val['m_data']
    except gdb.error:
        data = (deref(val).address + 1).cast(T('char').pointer())

    try:
        s = data.string('utf-8', 'ignore', val['m_len'])
    except OverflowError:
        s = "<string with invalid length: {}>".format(val['m_len'])

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
        crc = crc32q(crc, _unpack(s[i : i + 8]))

    if tail_sz == 0:
        return crc >> 1

    shift = -((tail_sz - 8) << 3) & 0b111111
    tail = _unpack(s[size:].ljust(8, '\0'))

    crc = crc32q(crc, tail << shift)
    return crc >> 1


def strinfo(s, keep_case=True):
    """Return the Python string and HHVM hash for `s', or None if `s' is not a
    stringish gdb.Value."""

    data = None
    h = None

    try:
        t = rawtype(s.type)
    except:
        return None

    if (t == T('char').pointer()
          or re.match(r"char \[\d*\]$", str(t)) is not None):
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


def alt_form_enum(str, enum_name):
    """Convert gcc-style enum symbol to clang-style."""

    # for example: alt_form_enum("HPHP::ArrayData::kMixedKind", "KindOfArray")
    # -> "#HPHP::ArrayData::KindOfArray::kMixedKind"
    a = str.split("::")
    b = a[:-1] + [enum_name] + a[-1:]
    return "::".join(b)


#------------------------------------------------------------------------------
# Caching lookups.

@memoized
def T(name):
    return gdb.lookup_type(name)


@memoized
def K(name, enumName=''):
    try:
        result = gdb.lookup_global_symbol(name).value()
    except:
        result = gdb.lookup_global_symbol(alt_form_enum(name, enumName)).value()
    return result


@memoized
def V(name, enumName=''):
    try:
        result = TL(name)
    except:
        result = TL(alt_form_enum(name, enumName))
    return result


@memoized
def nullptr():
    return gdb.Value(0).cast(T('void').pointer())


def TL(name):
    try:
        return gdb.lookup_symbol(name)[0].value()
    except gdb.error:
        return gdb.lookup_symbol(name)[0].value(gdb.selected_frame())


#------------------------------------------------------------------------------
# Type manipulations.

def destruct(t):
    return re.sub(r'^(struct|class|union)\s+', '', t)


def rawtype(t):
    return t.unqualified().strip_typedefs()


def template_type(t):
    """Get the unparametrized name of a template type."""
    return destruct(str(t).split('<')[0])


def rawptr(val):
    """Fully strip a smart pointer type to a raw pointer.  References are
    re-cast as pointers."""

    t = rawtype(val.type)

    if t.code == gdb.TYPE_CODE_PTR:
        return val
    elif t.code == gdb.TYPE_CODE_REF:
        return val.referenced_value().address

    name = template_type(t)
    ptr = None

    if name == 'std::unique_ptr':
        try:
            ptr = val['_M_t']['_M_t']['_M_head_impl']
        except:
            ptr = val['_M_t']['_M_head_impl']

    if name == 'HPHP::default_ptr':
        ptr = val['m_p']

    if name == 'HPHP::req::ptr' or name == 'HPHP::AtomicSharedPtrImpl':
        ptr = val['m_px']

    if name == 'HPHP::LowPtr' or name == 'HPHP::detail::LowPtrImpl':
        inner = t.template_argument(0)
        try:
            # Unwrap the std::atomic in AtomicLowPtr. (LowPtr is templated on
            # the m_s field's type, and for AtomicLowPtr, it's an atomic.)
            ptr = val['m_s']['_M_i'].cast(inner.pointer())
        except:
            ptr = val['m_s'].cast(inner.pointer())

    if name == 'HPHP::CompactTaggedPtr':
        inner = t.template_argument(0)
        ptr = (val['m_data'] & 0xffffffffffff).cast(inner.pointer())

    if name == 'HPHP::CompactSizedPtr':
        ptr = rawptr(val['m_data'])

    if ptr is not None:
        return rawptr(ptr)

    return None


def deref(val):
    """Fully dereference a value, stripping away *, &, and all known smart
    pointer wrappers (as well as const/volatile qualifiers)."""

    p = rawptr(val)

    if p is None:
        return val.cast(rawtype(val.type))
    else:
        return deref(p.referenced_value())


#------------------------------------------------------------------------------
# Name accessor.

def _full_func_name(func):
    attrs = atomic_get(func['m_attrs']['m_attrs'])
    if attrs & V('HPHP::AttrIsMethCaller'):
        cls = ""
    else:
        cls = atomic_get(func['m_u']['m_u'])['m_cls']
        if int(cls) == 0:
            cls = ""
        else:
            cls = nameof(cls.cast(T('HPHP::Class').pointer())) + "::"
    return cls + string_data_val(deref(func['m_name']))

def nameof(val):
    val = deref(val)
    try:
        t = val.type.name
    except:
        return None

    sd = None

    if t == 'HPHP::Func':
        sd = val['m_fullName']
        if int(rawptr(sd)) == 1:
            return _full_func_name(val)
    elif t == 'HPHP::Class':
        sd = deref(val['m_preClass'])['m_name']
    elif t == 'HPHP::ObjectData':
        cls = deref(val['m_cls'])
        sd = deref(cls['m_preClass'])['m_name']

    if sd is None:
        return None

    return string_data_val(deref(sd))


#------------------------------------------------------------------------------
# TV helpers.

tv_brief = False
tv_recurse = 0
tv_recurse_key = ""
current_key = None
_tv_recurse_depth = 0
_tv_recurse_seen = set()


def DT(kind):
    return V(kind, 'DataType')


def should_recurse():
    global _tv_recurse_depth
    global tv_recurse_key

    if tv_recurse is not True and _tv_recurse_depth >= tv_recurse:
        return False
    if tv_recurse_key == "" or current_key is None:
        return True
    return re.match(tv_recurse_key, current_key) is not None


def pretty_tv(t, data):
    t = t.cast(T("HPHP::DataType"))

    global tv_recurse
    global _tv_recurse_depth
    global tv_brief

    recurse = False

    val = None
    name = None

    if t == DT('HPHP::KindOfUninit') or t == DT('HPHP::KindOfNull'):
        pass

    elif t == DT('HPHP::KindOfBoolean'):
        if data['num'] == 0:
            val = False
        elif data['num'] == 1:
            val = True
        else:
            val = data['num']

    elif t == DT('HPHP::KindOfInt64'):
        val = int(data['num'])

    elif t == DT('HPHP::KindOfDouble'):
        val = float(data['dbl'])

    elif (t == DT('HPHP::KindOfString')
          or t == DT('HPHP::KindOfPersistentString')):
        val = '"%s"' % string_data_val(data['pstr'])

    elif (t == V('HPHP::KindOfDict')
          or t == V('HPHP::KindOfPersistentDict')
          or t == V('HPHP::KindOfVec')
          or t == V('HPHP::KindOfPersistentVec')
          or t == V('HPHP::KindOfKeyset')
          or t == V('HPHP::KindOfPersistentKeyset')):
        val = data['parr']
        if should_recurse():
            recurse = True

    elif t == DT('HPHP::KindOfObject'):
        val = data['pobj']
        if should_recurse():
            recurse = True
        name = nameof(val)

    elif t == DT('HPHP::KindOfResource'):
        val = data['pres']

    else:
        t = 'Invalid(%d)' % t.cast(T('int8_t'))
        val = "0x%x" % int(data['num'])

    if recurse:
        num = int(data['num'])
        if num not in _tv_recurse_seen:
            _tv_recurse_seen.add(num)
            _tv_recurse_depth += 1
            try:
                val = str(val.dereference())
                indent = 2 * _tv_recurse_depth
                val = re.sub(r"^", ' ' * indent, val, flags=re.M)[indent:]
            finally:
                _tv_recurse_seen.remove(num)
                _tv_recurse_depth -= 1
        else:
            val = "RECURSION: " + str(val)

    if tv_brief:
        if val is None:
            return str(t)
        elif not isinstance(val, gdb.Value):
            if name is None:
                return str(val)
            else:
                return '{ %s ("%s") }' % (str(val), name)

    if val is None:
        out = '{ %s }' % t
    elif name is None:
        out = '{ %s, %s }' % (t, str(val))
    else:
        out = '{ %s, %s ("%s") }' % (t, str(val), name)

    return out


#------------------------------------------------------------------------------
# Architecture.

@memoized
def arch():
    try:
        return gdb.newest_frame().architecture().name()
    except:
        return None


@memoized
def arch_regs():
    a = arch()

    if a == 'aarch64':
        return {
            'fp': 'x29',
            'sp': 'sp',
            'ip': 'pc',
            'cross_jit_save': ['x19', 'x20', 'x21', 'x22', 'x23',
                               'x24', 'x25', 'x26', 'x27', 'x28',
                               'd8', 'd9', 'd10', 'd11', 'd12',
                               'd13', 'd14', 'd15'
            ],
        }
    else:
        return {
            'fp': 'rbp',
            'sp': 'rsp',
            'ip': 'rip',
            'cross_jit_save': ['rbx', 'r12', 'r13', 'r14', 'r15'],
        }
