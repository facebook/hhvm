"""
GDB pretty printers for HHVM types.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import gdb
import re
from gdbutils import *
import idx
from nameof import nameof
from sizeof import sizeof


#------------------------------------------------------------------------------
# Legacy iteration.

class _BaseIterator:
    """Base iterator for Python 2 compatibility (in Python 3, next() is renamed
    to __next__()).  See http://legacy.python.org/dev/peps/pep-3114/.
    """
    def next(self):
        return self.__next__()

    def __iter__(self):
        return self


#------------------------------------------------------------------------------
# StringData.

class StringDataPrinter:
    RECOGNIZE = '^HPHP::StringData$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return string_data_val(self.val)

    def display_hint(self):
        return 'string'


#------------------------------------------------------------------------------
# TypedValue.

class TypedValuePrinter:
    RECOGNIZE = '^HPHP::(TypedValue|Cell|Ref|Variant|VarNR)$'

    def __init__(self, val):
        self.val = val.cast(T('HPHP::TypedValue'))

    def to_string(self):
        data = self.val['m_data']
        t = self.val['m_type']
        val = None
        name = None

        if t == V('HPHP::KindOfUninit') or t == V('HPHP::KindOfNull'):
            pass

        elif t == V('HPHP::KindOfBoolean'):
            if data['num'] == 0:
                val = False
            elif data['num'] == 1:
                val = True
            else:
                val = data['num']

        elif t == V('HPHP::KindOfInt64'):
            val = data['num']

        elif t == V('HPHP::KindOfDouble'):
            val = data['dbl']

        elif (t == V('HPHP::KindOfString') or
              t == V('HPHP::KindOfStaticString')):
            val = data['pstr'].dereference()

        elif t == V('HPHP::KindOfArray'):
            val = data['parr']

        elif t == V('HPHP::KindOfObject'):
            val = data['pobj']
            name = nameof(val)

        elif t == V('HPHP::KindOfResource'):
            val = data['pres']

        elif t == V('HPHP::KindOfRef'):
            val = data['pref'].dereference()

        else:
            t = "Invalid(%d)" % t.cast(T('int8_t'))
            val = "0x%x" % data['num']

        if val is None:
            out = "{ %s }" % t
        elif name is None:
            out = "{ %s, %s }" % (t, str(val))
        else:
            out = "{ %s, %s (%s) }" % (t, str(val), name)

        return out


#------------------------------------------------------------------------------
# Pointers.

class PtrPrinter:
    class _iterator(_BaseIterator):
        """Pretty printer `children()` iterator for pointer ranges."""
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration
            key = self.cur
            elt = self.cur.dereference()
            self.cur = self.cur + 1
            return ("0x%x" % key.cast(T('long')), elt)

    def _string(self):
        inner = self._pointer().dereference()
        inner_type = rawtype(inner.type)

        if inner_type.tag == 'HPHP::StringData':
            return string_data_val(inner)
        return None

    def to_string(self):
        s = self._string()

        if s is not None:
            return '0x%s "%s"' % (str(self._pointer()), s)
        return self._pointer()

    def children(self):
        ptr = self._pointer()
        s = self._string()

        if ptr and s is None:
            return self._iterator(ptr, ptr + 1)
        return self._iterator(0, 0)


class SmartPtrPrinter(PtrPrinter):
    RECOGNIZE = '^HPHP::(SmartPtr<.*>|(Static)?String|Array|Object)$'

    def __init__(self, val):
        self.val = val

    def _pointer(self):
        return self.val['m_px']

class LowPtrPrinter(PtrPrinter):
    RECOGNIZE = '^HPHP::(LowPtr<.*>|LowPtrImpl<.*>)$'

    def __init__(self, val):
        self.val = val

    def _pointer(self):
        inner = self.val.type.template_argument(0)
        return self.val['m_raw'].cast(inner.pointer())


#------------------------------------------------------------------------------
# ArrayData.

class ArrayDataPrinter:
    RECOGNIZE = '^HPHP::(ArrayData|MixedArray|ProxyArray)$'

    class _packed_iterator(_BaseIterator):
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end
            self.count = 0

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            elt = self.cur
            key = '%d' % self.count
            data = elt.dereference()

            self.cur = self.cur + 1
            self.count = self.count + 1
            return (key, data)

    class _mixed_iterator(_BaseIterator):
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            elt = self.cur

            if elt['data']['m_aux']['u_hash'] == 0:
                key = '%d' % elt['ikey']
            else:
                key = '"%s"' % string_data_val(elt['skey'].dereference())

            data = elt['data'].cast(T('HPHP::TypedValue'))

            self.cur = self.cur + 1
            return (key, data)


    def __init__(self, val):
        self.kind = val['m_kind']

        if self.kind == self._kind('Mixed'):
            self.val = val.cast(T('HPHP::MixedArray'))
        elif self.kind == self._kind('Proxy'):
            self.val = val.cast(T('HPHP::ProxyArray'))
        else:
            self.val = val

    def to_string(self):
        kind_int = int(self.kind.cast(T('uint8_t')))

        if kind_int > 9:
            return 'Invalid ArrayData (kind=%d)' % kind_int

        if self.kind == self._kind('Proxy'):
            return 'ProxyArray { %s }' % (
                self.val['m_ref'].dereference()['m_tv']
                        ['m_data']['parr'].dereference())

        kind = str(self.kind)[len('HPHP::ArrayData::'):]

        return "ArrayData[%s]: %d element(s)" % (kind, self.val['m_size'])

    def children(self):
        data = self.val.address.cast(T('char').pointer()) + \
               self.val.type.sizeof

        if self.kind == self._kind('Packed'):
            pelm = data.cast(T('HPHP::TypedValue').pointer())
            iter_class = self._packed_iterator
        elif self.kind == self._kind('Mixed'):
            pelm = data.cast(T('HPHP::MixedArray::Elm').pointer())
            iter_class = self._mixed_iterator
        else:
            return self._packed_iterator(0, 0)

        return iter_class(pelm, pelm + self.val['m_size'])

    def _kind(self, kind):
        return K('HPHP::ArrayData::k' + kind + 'Kind')


#------------------------------------------------------------------------------
# ObjectData.

class ObjectDataPrinter:
    RECOGNIZE = '^HPHP::(ObjectData)$'

    class _iterator(_BaseIterator):
        def __init__(self, obj):
            self.obj = obj
            self.cls = rawptr(obj['m_cls'])
            self.end = sizeof(self.cls['m_declProperties'])
            self.cur = gdb.Value(0).cast(self.end.type)

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            decl_props = self.cls['m_declProperties']

            name = idx.indexed_string_map_at(decl_props, self.cur)['m_name']
            val = idx.object_data_at(self.obj, name)

            self.cur = self.cur + 1

            if val is None:
                val = '<unknown>'

            return (str(deref(name)), val)

    def __init__(self, val):
        self.val = val.cast(val.dynamic_type)
        self.cls = deref(val['m_cls'])

    def to_string(self):
        return "Object of class %s @ %s" % (
            nameof(self.cls),
            self.val.address)

    def children(self):
        return self._iterator(self.val)


#------------------------------------------------------------------------------
# RefData.

class RefDataPrinter:
    RECOGNIZE = '^HPHP::RefData$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "RefData { %s }" % self.val['m_tv']


#------------------------------------------------------------------------------
# Lookup function.

printer_classes = [
    TypedValuePrinter,
    SmartPtrPrinter,
    LowPtrPrinter,
    StringDataPrinter,
    ArrayDataPrinter,
    ObjectDataPrinter,
    RefDataPrinter,
]
type_printers = {(re.compile(cls.RECOGNIZE), cls)
                  for cls in printer_classes}

def lookup_function(val):
    t = val.type
    if t.code == gdb.TYPE_CODE_REF:
        t = t.target()

    t = rawtype(t)

    # Get the type name.
    typename = t.tag
    if typename == None:
        return None

    # Iterate over local dict of types to determine if a printer is registered
    # for that type.  Return an instantiation of the printer if found.
    for recognizer_regex, func in type_printers:
        if recognizer_regex.search(typename):
            return func(val)

    # Cannot find a pretty printer.  Return None.
    return None

gdb.pretty_printers.append(lookup_function)
