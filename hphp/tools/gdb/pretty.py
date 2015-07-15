"""
GDB pretty printers for HHVM types.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import re
from gdbutils import *
import idx
from nameof import nameof
from sizeof import sizeof


#------------------------------------------------------------------------------
# Legacy iteration.

class _BaseIterator(object):
    """Base iterator for Python 2 compatibility (in Python 3, next() is renamed
    to __next__()).  See http://legacy.python.org/dev/peps/pep-3114/.
    """
    def next(self):
        return self.__next__()

    def __iter__(self):
        return self


#------------------------------------------------------------------------------
# StringData.

class StringDataPrinter(object):
    RECOGNIZE = '^HPHP::StringData$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return string_data_val(self.val)

    def display_hint(self):
        return 'string'


#------------------------------------------------------------------------------
# TypedValue.

class TypedValuePrinter(object):
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
            t = 'Invalid(%d)' % t.cast(T('int8_t'))
            val = "0x%x" % data['num']

        if val is None:
            out = '{ %s }' % t
        elif name is None:
            out = '{ %s, %s }' % (t, str(val))
        else:
            out = '{ %s, %s ("%s") }' % (t, str(val), name)

        return out


#------------------------------------------------------------------------------
# Pointers.

class PtrPrinter(object):
    def _string(self):
        inner = self._pointer().dereference()
        inner_type = rawtype(inner.type)

        if inner_type.tag == 'HPHP::StringData':
            return string_data_val(inner)
        return nameof(inner)

    def to_string(self):
        s = self._string()

        out = '(%s) %s'  % (str(self._ptype()), str(self._pointer()))
        return '%s "%s"' % (out, s) if s is not None else out


class ReqPtrPrinter(PtrPrinter):
    RECOGNIZE = '^HPHP::(req::ptr<.*>)$'

    def __init__(self, val):
        self.val = val

    def _ptype(self):
        return self.val.type

    def _pointer(self):
        return self.val['m_px']

class StringPrinter(ReqPtrPrinter):
    RECOGNIZE = '^HPHP::(Static)?String$'

    def __init__(self, val):
        super(StringPrinter, self).__init__(val['m_str'])

class ArrayPrinter(ReqPtrPrinter):
    RECOGNIZE = '^HPHP::Array$'

    def __init__(self, val):
        super(ArrayPrinter, self).__init__(val['m_arr'])

class ObjectPrinter(ReqPtrPrinter):
    RECOGNIZE = '^HPHP::Object$'

    def __init__(self, val):
        super(ObjectPrinter, self).__init__(val['m_obj'])

class ResourcePrinter(ReqPtrPrinter):
    RECOGNIZE = '^HPHP::Resource$'

    def __init__(self, val):
        super(ResourcePrinter, self).__init__(val['m_res'])


class LowPtrPrinter(PtrPrinter):
    RECOGNIZE = '^HPHP::(LowPtr<.*>|detail::LowPtrImpl<.*>)$'

    def __init__(self, val):
        self.val = val

    def _ptype(self):
        return self.val.type

    def _pointer(self):
        inner = self.val.type.template_argument(0)
        return self.val['m_s'].cast(inner.pointer())


#------------------------------------------------------------------------------
# ArrayData.

class ArrayDataPrinter(object):
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

            try:
                data = elt.dereference()
            except gdb.MemoryError:
                data = '<invalid>'

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

            try:
                if elt['data']['m_type'] == -1:
                    key = '<deleted>'
                elif elt['data']['m_aux']['u_hash'] < 0:
                    key = '%d' % elt['ikey']
                else:
                    key = '"%s"' % string_data_val(elt['skey'].dereference())
            except gdb.MemoryError:
                key = '<invalid>'

            try:
                data = elt['data'].cast(T('HPHP::TypedValue'))
            except gdb.MemoryError:
                data = '<invalid>'

            self.cur = self.cur + 1
            return (key, data)


    def __init__(self, val):
        kind_ty = T('HPHP::ArrayData::ArrayKind')
        self.kind = val['m_hdr']['kind'].cast(kind_ty)

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

        return "ArrayData[%s]: %d element(s) refcount=%d" % (
            kind,
            self.val['m_size'],
            self.val['m_hdr']['count']
        )

    def children(self):
        data = self.val.address.cast(T('char').pointer()) + \
               self.val.type.sizeof

        if self.kind == self._kind('Packed'):
            pelm = data.cast(T('HPHP::TypedValue').pointer())
            return self._packed_iterator(pelm, pelm + self.val['m_size'])
        if self.kind == self._kind('Mixed'):
            pelm = data.cast(T('HPHP::MixedArray::Elm').pointer())
            return self._mixed_iterator(pelm, pelm + self.val['m_used'])
        return self._packed_iterator(0, 0)

    def _kind(self, kind):
        return K('HPHP::ArrayData::k' + kind + 'Kind')


#------------------------------------------------------------------------------
# ObjectData.

class ObjectDataPrinter(object):
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

            try:
                name = idx.indexed_string_map_at(decl_props, self.cur)['m_name']
                try:
                    val = idx.object_data_at(self.obj, name)
                except gdb.MemoryError:
                    val = None
            except gdb.MemoryError:
                name = '<invalid>'

            self.cur = self.cur + 1

            if val is None:
                val = '<unknown>'

            return (str(deref(name)), val)

    def __init__(self, val):
        self.val = val.cast(val.dynamic_type)
        self.cls = deref(val['m_cls'])

    def to_string(self):
        return 'Object of class "%s" @ %s' % (
            nameof(self.cls),
            self.val.address)

    def children(self):
        return self._iterator(self.val)


#------------------------------------------------------------------------------
# RefData.

class RefDataPrinter(object):
    RECOGNIZE = '^HPHP::RefData$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "RefData { %s }" % self.val['m_tv']


#------------------------------------------------------------------------------
# Lookup function.

printer_classes = [
    TypedValuePrinter,
    ReqPtrPrinter,
    ArrayPrinter,
    ObjectPrinter,
    StringPrinter,
    ResourcePrinter,
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
