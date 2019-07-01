"""
GDB pretty printers for HHVM types.
"""

from compatibility import *

import gdb
import re

from gdbutils import *
from lookup import lookup_func
from nameof import nameof
from sizeof import sizeof
import idx


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

_tv_recurse = False


class SetTVRecurseCommand(gdb.Command):
    """Whether to recurse into TypedValue data ptrs when pretty-printing."""

    def __init__(self):
        super(SetTVRecurseCommand, self).__init__('set tv-recurse',
                                                  gdb.COMMAND_STATUS)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)

        if len(argv) != 1:
            print('Requires an argument. Valid arguments are true, false.')
            return

        global _tv_recurse

        if argv[0] == 'true':
            _tv_recurse = True
        elif argv[0] == 'false':
            _tv_recurse = False
        else:
            print('Undefined item: "{}"'.format(argv[0]))


SetTVRecurseCommand()


def DT(kind):
    return V(kind, 'DataType')


class TypedValuePrinter(object):
    RECOGNIZE = '^HPHP::(TypedValue|Cell|Ref|Variant|VarNR)$'

    def __init__(self, val):
        self.val = val.cast(T('HPHP::TypedValue'))

    def to_string(self):
        global _tv_recurse

        data = self.val['m_data']
        t = self.val['m_type']
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
            val = data['num']

        elif t == DT('HPHP::KindOfDouble'):
            val = data['dbl']

        elif (t == DT('HPHP::KindOfString') or
              t == DT('HPHP::KindOfPersistentString')):
            val = data['pstr'].dereference()

        elif (t == V('HPHP::KindOfArray') or
              t == V('HPHP::KindOfPersistentArray') or
              t == V('HPHP::KindOfDict') or
              t == V('HPHP::KindOfPersistentDict') or
              t == V('HPHP::KindOfVec') or
              t == V('HPHP::KindOfPersistentVec') or
              t == V('HPHP::KindOfKeyset') or
              t == V('HPHP::KindOfPersistentKeyset')):
            val = data['parr']
            if _tv_recurse:
                val = val.dereference()

        elif t == DT('HPHP::KindOfObject'):
            val = data['pobj']
            if _tv_recurse:
                val = val.dereference()
            name = nameof(val)

        elif t == DT('HPHP::KindOfResource'):
            val = data['pres']

        elif t == DT('HPHP::KindOfRef'):
            val = data['pref'].dereference()

        else:
            t = 'Invalid(%d)' % t.cast(T('int8_t'))
            val = "0x%x" % int(data['num'])

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
        ptr = self._pointer()
        if ptr == nullptr():
            return None

        inner = self._pointer().dereference()
        inner_type = rawtype(inner.type)

        if inner_type.tag == 'HPHP::StringData':
            return string_data_val(inner)
        return nameof(inner)

    def to_string(self):
        try:
            s = self._string()
        except gdb.MemoryError:
            s = None

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
        storage = template_type(rawtype(self.val.type.template_argument(1)))

        if storage == 'HPHP::detail::AtomicStorage':
            return idx.atomic_get(self.val['m_s']).cast(inner.pointer())
        else:
            return self.val['m_s'].cast(inner.pointer())


#------------------------------------------------------------------------------
# folly::Optional

class OptionalPrinter(object):
    RECOGNIZE = '^(HPHP::req|folly)::Optional<.*>$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        if not self.val['storage_']['hasValue']:
            return 'folly::none'
        inner = self.val.type.template_argument(0)
        ptr = self.val['storage_']['value'].address.cast(inner.pointer())
        return ptr.dereference()

#------------------------------------------------------------------------------
# ArrayData.

class ArrayDataPrinter(object):
    RECOGNIZE = '^HPHP::(ArrayData|MixedArray)$'

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
                if elt['data']['m_type'] == -128:
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

    class _set_iterator(_BaseIterator):
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            elt = self.cur

            try:
                if elt['tv']['m_type'] == -128:
                    key = '<deleted>'
                else:
                    key = "%s" % elt['tv'].cast(T('HPHP::TypedValue'))
            except gdb.MemoryError:
                key = '<invalid>'

            self.cur = self.cur + 1
            return (key, key)


    def __init__(self, val):
        kind_ty = T('HPHP::ArrayData::ArrayKind')
        self.kind = val['m_kind'].cast(kind_ty)

        if self.kind == self._kind('Mixed') or self.kind == self._kind('Dict'):
            self.val = val.cast(T('HPHP::MixedArray'))
        elif self.kind == self._kind('Keyset'):
            self.val = val.cast(T('HPHP::SetArray'))
        else:
            self.val = val

    def to_string(self):
        kind_int = int(self.kind.cast(T('uint8_t')))

        if kind_int > 9:
            return 'Invalid ArrayData (kind=%d)' % kind_int

        kind = str(self.kind)[len('HPHP::ArrayData::'):]

        return "ArrayData[%s]: %d element(s) refcount=%d" % (
            kind,
            self.val['m_size'],
            self.val['m_count']
        )

    def children(self):
        data = self.val.address.cast(T('char').pointer()) + \
               self.val.type.sizeof

        if self.kind == self._kind('Packed') or self.kind == self._kind('Vec'):
            pelm = data.cast(T('HPHP::TypedValue').pointer())
            return self._packed_iterator(pelm, pelm + self.val['m_size'])
        if self.kind == self._kind('Mixed') or self.kind == self._kind('Dict'):
            pelm = data.cast(T('HPHP::MixedArrayElm').pointer())
            return self._mixed_iterator(pelm, pelm + self.val['m_used'])
        if self.kind == self._kind('Keyset'):
            pelm = data.cast(T('HPHP::SetArrayElm').pointer())
            return self._set_iterator(pelm, pelm + self.val['m_used'])
        return self._packed_iterator(0, 0)

    def _kind(self, kind):
        return K('HPHP::ArrayData::k' + kind + 'Kind', 'ArrayKind')


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
                name = idx.indexed_string_map_at(decl_props, self.cur)['name']
            except gdb.MemoryError:
                name = '<invalid>'

            try:
                val = idx.object_data_at(self.obj, self.cur)
            except gdb.MemoryError:
                val = '<unknown>'

            self.cur = self.cur + 1

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
# HHBBC::Bytecode

class HhbbcBytecodePrinter(object):
    RECOGNIZE = '^HPHP::HHBBC::Bytecode$'

    def __init__(self, val):
        self.op = ("%s" % val['op']).replace("HPHP::Op::", "")
        self.val = val[self.op]

    def to_string(self):
        return 'bc::%s { %s }' % (self.op, self.val)

#------------------------------------------------------------------------------
# Lookup function.
class CompactVectorPrinter(object):
    RECOGNIZE = '^HPHP::CompactVector(<.*>)$'

    class _iterator(_BaseIterator):
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


    def __init__(self, val):
        inner = val.type.template_argument(0)
        self.inner = inner
        if val['m_data'] == nullptr():
            self.len = 0
            self.cap = 0
        else:
            self.len = val['m_data']['m_len']
            self.cap = val['m_data']['m_capacity']
            self.elems = (val['m_data'].cast(T('char').pointer()) +
                          val['elems_offset']).cast(inner.pointer())

    def to_string(self):
        return "CompactVector<%s>: %d element(s) capacity=%d" % (
            self.inner.name,
            self.len,
            self.cap
        )

    def children(self):
        if self.len == 0:
            return self._iterator(0, 0)
        else:
            return self._iterator(self.elems, self.elems + self.len)


#------------------------------------------------------------------------------
# SrcKey.

class SrcKeyPrinter(object):
    RECOGNIZE = '^HPHP::SrcKey$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        func_id = self.val['m_funcID']
        if func_id == -1:
            return '<invalid SrcKey>'

        func = nameof(lookup_func(func_id))
        offset = self.val['m_offset']
        this = 't' if self.val['m_hasThis'] else ''

        rmp = self.val['m_resumeModeAndPrologue']
        resume = prologue = ''
        if rmp == 0:
            # ResumeMode::None
            pass
        elif rmp == 1:
            resume = 'ra'
        elif rmp == 2:
            resume = 'rg'
        elif rmp == 3:
            prologue = 'p'

        return '%s@%d%s%s%s' % (func, offset, resume, this, prologue)

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
    HhbbcBytecodePrinter,
    CompactVectorPrinter,
    SrcKeyPrinter,
    OptionalPrinter,
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
