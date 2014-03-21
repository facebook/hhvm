"""
Python GDB macros for inspecting hhvm data types.

To use, add this to your .gdbinit:

   source path/to/hhvm/repo/tools/gdb/hhvm.py

Then just use "p foo" and "x foo" as normal, and if it's one of the
supported types it'll be prettier.  If the macros go off the rails,
you can use "p/r foo" to get back to raw printing.
"""
# @lint-avoid-python-3-compatibility-imports
import gdb
import re

def string_data_val(val):
    return val['m_data'].string("utf-8", 'ignore', val['m_len'])

class TypedValuePrinter:
    RECOGNIZE = '^HPHP::(TypedValue|VM::Cell|Variant|VarNR)$'
    def __init__(self, val):
        tv = gdb.lookup_type('HPHP::TypedValue')
        self.val = val.cast(tv)

    def to_string(self):
        itype = gdb.lookup_type('int')
        v = self.val['m_type'].cast(itype)

        # values from runtime/base/datatype.h
        if v == 0x00:
            return "Uninit"
        elif v == 0x08:
            return "Null"
        elif v == 0x09:
            if self.val['m_data']['num'] == 0:
                return "Tv: false"
            else:
                return "Tv: true"
        elif v == 0x0a:
            return "Tv: %d" % self.val['m_data']['num']
        elif v == 0x0b:
            return "Tv: %g" % self.val['m_data']['dbl']
        elif v == 0x0c or v == 0x14:
            return "Tv: '%s'" % string_data_val(self.val['m_data']['pstr'].
                                            dereference())
        elif v == 0x20:
            return "Tv: %s" % self.val['m_data']['parr'].dereference()
        elif v == 0x30:
            return "Tv: %s" % self.val['m_data']['pobj'].dereference()
        elif v == 0x40:
            return "Tv: %s" % self.val['m_data']['pres'].dereference()
        elif v == 0x50:
            return "Tv: %s" % self.val['m_data']['pref'].dereference()
        else:
            return "Type %d" % v

class StringDataPrinter:
    RECOGNIZE = '^HPHP::StringData$'
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "Str: '%s'" % string_data_val(self.val)

class _BaseIterator:
    """
    Base iterator for Python 2 compatibility (in Python 3, next() is renamed
    to __next__()). See http://legacy.python.org/dev/peps/pep-3114/
    """
    def next(self):
        return self.__next__()

class ArrayDataPrinter:
    RECOGNIZE = '^HPHP::(ArrayData|HphpArray)$'

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
            data = elt['data']
            packed = gdb.lookup_global_symbol('HPHP::ArrayData::kPackedKind') \
                .value()
            if self.kind == packed:
                key = '%d' % self.count
            elif data['m_aux']['u_hash'] == 0:
                key = '%d' % elt['ikey']
            else:
                key = '"%s"' % string_data_val(elt['key'].dereference())
            self.cur = self.cur + 1
            self.count = self.count + 1
            return (key, data)

    def __init__(self, val):
        self.kind = val['m_kind']
        if self.kind == 0 or self.kind == 1:
            self.val = val.cast(gdb.lookup_type('HPHP::HphpArray'))
        elif self.kind == self.proxyKind():
            self.val = val.cast(gdb.lookup_type('HPHP::ProxyArray'))
        else:
            self.val = val

    def children(self):
        # Only support kPackedKind or kMixedKind
        if self.kind == self.packedKind() or self.kind == self.mixedKind():
            data = self.val.address.cast(gdb.lookup_type('char').pointer()) + \
                self.val.type.sizeof
            pelm = data.cast(gdb.lookup_type('HPHP::HphpArray::Elm').pointer())
            return self._iterator(self.kind, pelm, pelm + self.val['m_size'])
        return self._iterator(0, 0, 0)

    def to_string(self):
        if self.kind == self.proxyKind():
            return "ProxyArr: %s" % (self.val['m_ad'].dereference())
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
        self.cls = val['m_cls']

    def children(self):
        dp = self.cls['m_declProperties']
        if not dp:
            return self._iterator(0,0,0,0)
        mv = dp['m_vec']
        if not mv:
            return self._iterator(0,0,0,0)

        return self._iterator(self.val, self.cls, mv, mv + dp['m_map']['m_extra'])

    def to_string(self):
        return "Object of class %s @ 0x%x" % (
            string_data_val(self.cls['m_preClass']['m_px']['m_name']),
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
        return "Class %s" % string_data_val(self.val['m_preClass']['m_px']['m_name'])

printer_classes = [
    TypedValuePrinter,
    StringDataPrinter,
    ArrayDataPrinter,
    ObjectDataPrinter,
    SmartPtrPrinter,
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
