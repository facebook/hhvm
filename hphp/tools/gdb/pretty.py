"""
GDB pretty printers for HHVM types.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import gdb
import re
from gdbutils import *
from nameof import nameof


#------------------------------------------------------------------------------
# Legacy iteration.

class _BaseIterator:
    """Base iterator for Python 2 compatibility (in Python 3, next() is renamed
    to __next__()).  See http://legacy.python.org/dev/peps/pep-3114/.
    """
    def next(self):
        return self.__next__()


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
# ArrayData.

class ArrayDataPrinter:
    RECOGNIZE = '^HPHP::(ArrayData|MixedArray)$'

    class _iterator(_BaseIterator):
        def __init__(self, kind, begin, end):
            self.kind = kind
            self.cur = begin
            self.end = end
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration
            elt = self.cur
            packed = gdb.lookup_global_symbol('HPHP::ArrayData::kPackedKind') \
                .value()
            if self.kind == packed:
                data = elt.dereference()
            else:
                data = elt['data']
            if self.kind == packed:
                key = '%d' % self.count
            elif data['m_aux']['u_hash'] == 0:
                key = '%d' % elt['ikey']
            else:
                key = '"%s"' % string_data_val(elt['skey'].dereference())
            self.cur = self.cur + 1
            self.count = self.count + 1
            return (key, data)

    def __init__(self, val):
        self.kind = val['m_kind']
        if self.kind == self.mixedKind():
            self.val = val.cast(gdb.lookup_type('HPHP::MixedArray'))
        elif self.kind == self.proxyKind():
            self.val = val.cast(gdb.lookup_type('HPHP::ProxyArray'))
        else:
            self.val = val

    def children(self):
        if self.kind == self.packedKind():
            data = self.val.address.cast(gdb.lookup_type('char').pointer()) + \
                   self.val.type.sizeof
            pval = data.cast(gdb.lookup_type('HPHP::TypedValue').pointer())
            return self._iterator(self.kind, pval, pval + self.val['m_size'])
        elif self.kind == self.mixedKind():
            data = self.val.address.cast(gdb.lookup_type('char').pointer()) + \
                self.val.type.sizeof
            pelm = data.cast(gdb.lookup_type('HPHP::MixedArray::Elm').pointer())
            return self._iterator(self.kind, pelm, pelm + self.val['m_size'])
        return self._iterator(0, 0, 0)

    def to_string(self):
        if self.kind == self.proxyKind():
            return "ProxyArr: %s" % (
                self.val['m_ref'].dereference()['m_tv']
                        ['m_data']['parr'].dereference())
        else:
            return "%d elements (kind==%d)" % (self.val['m_size'], self.kind)

    def proxyKind(self):
        return gdb.lookup_global_symbol('HPHP::ArrayData::kProxyKind').value()

    def packedKind(self):
        return gdb.lookup_global_symbol('HPHP::ArrayData::kPackedKind').value()

    def mixedKind(self):
        return gdb.lookup_global_symbol('HPHP::ArrayData::kMixedKind').value()

objectDataCount = 100

class ObjectDataPrinter:
    RECOGNIZE = '^HPHP::(ObjectData|Instance)$'

    class _iterator(_BaseIterator):
        def __init__(self, val, cls, begin, end):
            self.cur = begin
            self.end = end
            if self.cur != self.end:
                addr = val.address.cast(gdb.lookup_type('char').pointer())
                addr = addr + val.type.sizeof + cls['m_builtinODTailSize']
                self.addr = addr.cast(gdb.lookup_type('HPHP::TypedValue').pointer())

        def __iter__(self):
            return self

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            elt = self.cur
            tv = self.addr
            global objectDataCount
            if objectDataCount > 0:
                objectDataCount = objectDataCount - 1
                tv = tv.dereference()

            self.addr = self.addr + 1
            self.cur = self.cur + 1
            return (string_data_val(elt['m_name']), tv)

    def __init__(self, val):
        self.dtype = val.dynamic_type
        self.val = val.cast(self.dtype)

        clstype = gdb.lookup_type('HPHP::Class').pointer()
        self.cls = val['m_cls']['m_raw'].cast(clstype)

    def children(self):
        dp = self.cls['m_declProperties']
        if not dp:
            return self._iterator(0,0,0,0)

        # FIXME: dp['m_vec'] no longer exists
        return self._iterator(0,0,0,0)

#        mv = dp['m_vec']
#        if not mv:
#            return self._iterator(0,0,0,0)
#
#        return self._iterator(self.val, self.cls, mv, mv + dp['m_map']['m_extra'])

    def to_string(self):
        ls = LowStringPtrPrinter(self.cls['m_preClass']['m_px']['m_name'])
        return "Object of class %s @ 0x%x" % (
            string_data_val(ls.to_string_data()),
            self.val.address)

class SmartPtrPrinter:
    RECOGNIZE = '^HPHP::((Static)?String|Array|Object|SmartPtr<.*>)$'

    class _iterator(_BaseIterator):
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __iter__(self):
            return self

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration
            key = self.cur
            elt = self.cur.dereference()
            self.cur = self.cur + 1
            return ("0x%x" % key.cast(gdb.lookup_type('long')), elt)

    def __init__(self, val):
        self.val = val

    def children(self):
        if self._pointer():
            return self._iterator(self._pointer(), self._pointer() + 1)
        return self._iterator(0, 0)

    def _pointer(self):
        return self.val['m_px']

    def to_string(self):
        tag = self._pointer().dereference().type.tag
        if self._pointer():
            return "SmartPtr<%s>" % tag
        return "SmartPtr<%s>(Null)" % tag

class LowStringPtrPrinter:
    RECOGNIZE = '^HPHP::(LowPtr<HPHP::StringData.*>|LowStringPtr)$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        ptr = self.to_string_data()
        if ptr:
            return "LowStringPtr '%s'@%s" % (string_data_val(ptr), ptr)
        else:
            return "LowStringPtr(Null)"

    def to_string_data(self):
        return self.val['m_raw'].cast(gdb.lookup_type('HPHP::StringData').pointer())

class LowClassPtrPrinter:
    RECOGNIZE = '^HPHP::(LowPtr<HPHP::Class.*>|LowClassPtr)$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        ptr = self.to_class()
        if ptr:
            return "LowClassPtr '%s'" % ptr
        return "LowClassPtr(Null)"

    def to_class(self):
        return self.val['m_raw'].cast(gdb.lookup_type('HPHP::Class').pointer())

class RefDataPrinter:
    RECOGNIZE = '^HPHP::RefData$'
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "Ref: %s" % self.val['m_tv']

class ResourceDataPrinter:
    RECOGNIZE = '^HPHP::ResourceData$'
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "Res #%d" % self.val['o_id']

class ClassPrinter:
    RECOGNIZE = '^HPHP::Class$'
    def __init__(self, val):
        self.val = val

    def to_string(self):
        ls = LowStringPtrPrinter(self.val['m_preClass']['m_px']['m_name'])
        return "Class %s" % string_data_val(ls.to_string_data())


#------------------------------------------------------------------------------
# Lookup function.

printer_classes = [
    TypedValuePrinter,
    StringDataPrinter,
    ArrayDataPrinter,
    ObjectDataPrinter,
    SmartPtrPrinter,
    LowStringPtrPrinter,
    LowClassPtrPrinter,
    RefDataPrinter,
    ResourceDataPrinter,
    ClassPrinter,
]
type_printers = {(re.compile(cls.RECOGNIZE), cls)
                     for cls in printer_classes}

def lookup_function(val):
    type = val.type
    if type.code == gdb.TYPE_CODE_REF:
        type = type.target ()

    type = type.unqualified ().strip_typedefs ()

    # Get the type name.
    typename = type.tag
    if typename == None:
        return None

    # Iterate over local dict of types to determine if a printer is
    # registered for that type.  Return an instantiation of the
    # printer if found.
    for recognizer_regex, func in type_printers:
        if recognizer_regex.search(typename):
            return func(val)

    # Cannot find a pretty printer.  Return None.
    return None

gdb.pretty_printers.append(lookup_function)
