#
# Python GDB macros for inspecting hhvm data types.
#
#
#  To use, add this to your .gdbinit:
#
#    source path/to/hhvm/repo/tools/gdb/hhvm.py
#
# Then just use "p foo" as normal, and if it's one of the supported
# types it'll be prettier.  If the macros go off the rails, you can
# use "p/r foo" to get back to raw printing.
#
#

import gdb
import re

def StringDataVal(val):
    return val['m_data'].string("utf-8", 'ignore', val['m_len'])

class TypedValuePrinter:
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
            return "Tv: '%s'" % StringDataVal(self.val['m_data']['pstr'].
                                            dereference())
        elif v == 0x20:
            return "Tv: %s" % self.val['m_data']['parr'].dereference()
        elif v == 0x30:
            return "Tv: %s" % self.val['m_data']['pobj'].dereference()
        elif v == 0x50:
            return "Ref: %s" % self.val['m_data']['pref'].dereference()
        else:
            return "Type %d" % v

class StringDataPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "Str: '%s'" % StringDataVal(self.val)

class ArrayDataPrinter:
    class _iterator:
        def __init__(self, kind, begin, end):
            self.kind = kind
            self.cur = begin
            self.end = end
            self.count = 0

        def __iter__(self):
            return self

        def next(self):
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
                key = '"%s"' % StringDataVal(elt['key'].dereference())
            self.cur = self.cur + 1
            self.count = self.count + 1
            return (key, data)

    def __init__(self, val):
        self.kind = val['m_kind']
        if self.kind == 0 or self.kind == 1:
            self.val = val.cast(gdb.lookup_type('HPHP::HphpArray'))
        else:
            self.val = val

    def children(self):
        packed = gdb.lookup_global_symbol('HPHP::ArrayData::kPackedKind') \
            .value()
        mixed = gdb.lookup_global_symbol('HPHP::ArrayData::kMixedKind') \
            .value()
        # Only support kPackedKind or kMixedKind
        if self.kind == packed or self.kind == mixed:
            return self._iterator(self.kind,
                                  self.val['m_data'],
                                  self.val['m_data'] + self.val['m_size'])
        return self._iterator(0, 0, 0)

    def to_string(self):
        return "%d elements (kind==%d)" % (self.val['m_size'], self.kind)

objectDataCount = 100

class ObjectDataPrinter:
    class _iterator:
        def __init__(self, val, cls, begin, end):
            self.cur = begin
            self.end = end
            if self.cur != self.end:
                addr = val.address.cast(gdb.lookup_type('char').pointer())
                addr = addr + val.type.sizeof + cls['m_builtinPropSize']
                self.addr = addr.cast(gdb.lookup_type('HPHP::TypedValue').pointer())

        def __iter__(self):
            return self

        def next(self):
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
            return (StringDataVal(elt['m_name']), tv)

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

        return self._iterator(self.val, self.cls, mv, mv + dp['m_size'])

    def to_string(self):
        return "%s of class %s" % (
            self.dtype.tag,
            StringDataVal(self.cls['m_preClass']['m_px']['m_name']))

class SmartPtrPrinter:
    class _iterator:
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __iter__(self):
            return self

        def next(self):
            if self.cur == self.end:
                raise StopIteration
            key = self.cur
            elt = self.cur.dereference()
            self.cur = self.cur + 1
            return ("0x%x" % key.cast(gdb.lookup_type('long')), elt)

    def __init__(self, val):
        self.val = val

    def children(self):
        if self.val['m_px']:
            return self._iterator(self.val['m_px'],
                                  self.val['m_px'] + 1)
        return self._iterator(0, 0)

    def to_string(self):
        if self.val['m_px']:
            return "SmartPtr<%s>" % self.val['m_px'].dereference().type.tag
        return "SmartPtr<%s>(Null)" % self.val['m_px'].dereference().type.tag

dict = {}
dict[re.compile('^HPHP::TypedValue|HPHP::VM::Cell|HPHP::Variant|HPHP::VarNR$')] = lambda val: TypedValuePrinter(val)
dict[re.compile('^HPHP::StringData$')] = lambda val: StringDataPrinter(val)
dict[re.compile('^HPHP::(ArrayData|HphpArray)$')] = lambda val: ArrayDataPrinter(val)
dict[re.compile('^HPHP::(ObjectData|Instance)$')] = lambda val: ObjectDataPrinter(val)
dict[re.compile('^HPHP::((Static)?String|Array|Object|SmartPtr<.*>)$')] = lambda val: SmartPtrPrinter(val)

def lookup_function(val):
    type = val.type
    if type.code == gdb.TYPE_CODE_REF:
        type = type.target ()

    type = type.unqualified ().strip_typedefs ()

    # Get the type name.
    typename = type.tag
    if typename == None:
        return None

    # Iterate over local dictionary of types to determine
    # if a printer is registered for that type.  Return an
    # instantiation of the printer if found.
    for function in dict:
        if function.search (typename):
            return dict[function] (val)

    # Cannot find a pretty printer.  Return None.
    return None

gdb.pretty_printers.append(lookup_function)

