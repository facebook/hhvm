#!/usr/bin/env python3

"""
GDB pretty printers for HHVM types.
"""

from compatibility import *

import gdb
import gdb.types
import re

import gdbutils
from gdbutils import *
from lookup import lookup_func
from sizeof import sizeof
from asio import WaitHandle
import idx
from hhbc import as_idx


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


class SetTVRecurseCommand(gdb.Command):
    """How many levels to recurse into TypedValue data ptrs when pretty-printing.
    and an optional filter on keys to recurse on.
    """

    def __init__(self):
        super(SetTVRecurseCommand, self).__init__('set tv-recurse',
                                                  gdb.COMMAND_STATUS)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)

        argn = len(argv)
        depth = None
        regex = ""
        if argn == 1:
            depth = argv[0]
        if argn == 2:
            depth, regex = argv

        if depth is not None:
            gdbutils.tv_recurse_key = regex

            if depth == 'true':
                gdbutils.tv_recurse = True
                return

            if depth == 'false':
                gdbutils.tv_recurse = 0
                return

            try:
                gdbutils.tv_recurse = int(argv[0])
                if gdbutils.tv_recurse >= 0:
                    return
            except:
                pass

        print(
            'tv-recurse <depth> [<regex>]\n'
            '  Valid values for <depth> are true, false or an integer depth (>=0)\n'
            '  The optional <regex> determines which array keys or object properties '
            'to recurse on\n'
        )


SetTVRecurseCommand()


class SetTVBriefCommand(gdb.Command):
    """
    Select brief or verbose TV pretty printing
    """

    def __init__(self):
        super(SetTVBriefCommand, self).__init__('set tv-brief',
                                                  gdb.COMMAND_STATUS)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)

        argn = len(argv)
        if argn == 1:
            if argv[0] == 'true':
                gdbutils.tv_brief = True
                return

            if argv[0] == 'false':
                gdbutils.tv_brief = False
                return

        print(
            'tv-brief <arg>\n'
            '  Valid values for <arg> are true and false.\n'
        )


SetTVBriefCommand()


class TypedValuePrinter:
    RECOGNIZE = '^HPHP::(TypedValue|Cell|Ref|Variant|VarNR)$'

    def __init__(self, val):
        self.val = val.cast(T('HPHP::TypedValue'))

    def to_string(self):
        return pretty_tv(self.val['m_type'], self.val['m_data'])


#------------------------------------------------------------------------------
# Pointers.

class PtrPrinter:
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

        out = '(%s) %s' % (str(self._ptype()), str(self._pointer()))
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
        rtype = gdb.types.get_basic_type(self.val.type)
        inner = rtype.template_argument(0)
        try:
            storage = template_type(rawtype(rtype.template_argument(1)))
            if storage == 'HPHP::detail::AtomicStorage':
                return atomic_get(self.val['m_s']).cast(inner.pointer())
            else:
                return self.val['m_s'].cast(inner.pointer())
        finally:
            return self.val['m_s'].cast(inner.pointer())


#------------------------------------------------------------------------------
# folly::Optional

class OptionalPrinter:
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
# (HPHP::req|std)::vector


class ReqVectorPrinter:
    RECOGNIZE = '^(HPHP::req|std)::vector<.*>$'

    class _iterator(_BaseIterator):
        def __init__(self, obj, size):
            self.obj = obj
            self.cur = 0
            self.end = size

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            try:
                val = idx.vector_at(self.obj, self.cur)
            except gdb.MemoryError:
                val = '<unknown>'

            self.cur = self.cur + 1

            return (str(self.cur - 1), val)

    def __init__(self, val):
        self.val = val
        self.size = sizeof(val)

    def to_string(self):
        typ = self.val.type.template_argument(0)
        return "HPHP::req::vector [%s]: %d element(s)" % (
            typ,
            self.size,
        )

    def children(self):
        return self._iterator(self.val, self.size)


#------------------------------------------------------------------------------
# ArrayData.


class ArrayDataPrinter:
    RECOGNIZE = '^HPHP::(ArrayData|VanillaDict)$'

    class _vec_iterator(_BaseIterator):
        def __init__(self, base, size):
            self.base = base
            self.size = size
            self.cur = 0

        def __next__(self):
            if self.cur == self.size:
                raise StopIteration

            key = '%d' % self.cur
            try:
                val = idx.vec_at(self.base, self.cur)
            except gdb.MemoryError:
                data = '<invalid>'

            self.cur = self.cur + 1

            return (key, val)

    class _dict_iterator(_BaseIterator):
        def __init__(self, begin, end):
            self.cur = begin
            self.end = end

        def __next__(self):
            if self.cur == self.end:
                raise StopIteration

            elt = self.cur

            try:
                if elt['data']['m_type'] == -128:
                    rawkey = key = '<deleted>'
                elif elt['data']['m_aux']['u_hash'] < 0:
                    rawkey = key = '%d' % elt['ikey']
                else:
                    rawkey = string_data_val(elt['skey'].dereference())
                    key = '"%s"' % rawkey

            except gdb.MemoryError:
                rawkey = key = '<invalid>'

            gdbutils.current_key = rawkey
            try:
                data = elt['data'].cast(T('HPHP::TypedValue'))
            except gdb.MemoryError:
                data = '<invalid>'
            finally:
                gdbutils.current_key = None

            self.cur = self.cur + 1
            return (key, data)

    class _keyset_iterator(_BaseIterator):
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

        if self.kind == self._kind('Dict'):
            self.val = val.cast(T('HPHP::VanillaDict'))
        elif self.kind == self._kind('Keyset'):
            self.val = val.cast(T('HPHP::VanillaKeyset'))
        else:
            self.val = val

    def to_string(self):
        kind_int = int(self.kind.cast(T('uint8_t')))

        if kind_int >= 14:
            return 'Invalid ArrayData (kind=%d)' % kind_int

        kind = str(self.kind)[len('HPHP::ArrayData::'):]

        return "ArrayData[%s]: %d element(s) refcount=%d" % (
            kind,
            self.val['m_size'],
            self.val['m_count']
        )

    def children(self):
        data = (self.val.address.cast(T('char').pointer())
                + self.val.type.sizeof)

        if self.kind == self._kind('Vec'):
            pelm = data.cast(T('char').pointer())
            return self._vec_iterator(pelm, self.val['m_size'])
        if self.kind == self._kind('Dict'):
            pelm = data.cast(T('HPHP::VanillaDictElm').pointer())
            return self._dict_iterator(pelm, pelm + self.val['m_used'])
        if self.kind == self._kind('Keyset'):
            pelm = data.cast(T('HPHP::VanillaKeysetElm').pointer())
            return self._keyset_iterator(pelm, pelm + self.val['m_used'])
        return self._packed_iterator(0, 0)

    def _kind(self, kind):
        return K('HPHP::ArrayData::k' + kind + 'Kind', 'ArrayKind')


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

            try:
                name = idx.indexed_string_map_at(decl_props, self.cur)['name']
            except gdb.MemoryError:
                name = '<invalid>'

            gdbutils.current_key = strinfo(name)['data']
            try:
                val = idx.object_data_at(self.obj, self.cls, self.cur)
            except gdb.MemoryError:
                val = '<unknown>'
            finally:
                gdbutils.current_key = None

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
# HHBC ops.
#
# Helpful for converting raw signed integers that GDB would typically
# print for enum values of type Op


class HhbcOpsPrinter:
    RECOGNIZE = '^HPHP::Op$'

    def __init__(self, val):
        int2enum = {
            field.enumval: field.name for field in T('HPHP::Op').fields()
        }
        self.val = int2enum[int(as_idx(val))]

    def to_string(self):
        return str(self.val)

#------------------------------------------------------------------------------
# HHBBC::Bytecode


class HhbbcBytecodePrinter:
    RECOGNIZE = '^HPHP::HHBBC::Bytecode$'

    def __init__(self, val):
        self.op = ("%s" % val['op']).replace("HPHP::Op::", "")
        self.val = val[self.op]

    def to_string(self):
        return 'bc::%s { %s }' % (self.op, self.val)

#------------------------------------------------------------------------------
# Lookup function.


class CompactVectorPrinter:
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
            self.elems = (val['m_data'].cast(T('char').pointer())
                          + val['elems_offset']).cast(inner.pointer())

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

class SrcKeyPrinter:
    RECOGNIZE = '^HPHP::SrcKey$'

    def __init__(self, val):
        self.val = val

    def to_string(self):
        func_id = self.val['m_s']['m_funcID']
        # Complicated cast to overcome "Invalid type combination in equality test"
        if int(func_id.cast(gdb.lookup_type('uint32'))) == -1:
            return '<invalid SrcKey>'

        func = nameof(lookup_func(func_id))
        offset = self.val['m_s']['m_offsetOrNumArgs']

        rmp = self.val['m_s']['m_resumeModeAndTags']
        mode = ''
        if rmp == 0:
            # ResumeMode::None
            pass
        elif rmp == 1:
            mode = 'ra'
        elif rmp == 2:
            mode = 'rg'
        elif rmp == 3:
            mode = 'p'
        elif rmp == 4:
            mode = 'fe'

        return '%s@%d%s' % (func, offset, mode)


#------------------------------------------------------------------------------
# AsioBlockable.


class AsioBlockablePrinter:
    RECOGNIZE = '^HPHP::AsioBlockable$'

    def __init__(self, val):
        self.val = val

    def kind(self):
        kind = (self.val['m_bits'] & 0x7).cast(T('HPHP::AsioBlockable::Kind'))
        return str(kind).split('::')[-1]

    def next(self):
        ty = T('HPHP::AsioBlockable').pointer()
        return (self.val['m_bits'] & ~0x7).cast(ty)

    def to_string(self):
        wh = WaitHandle.from_blockable(self.val.address)
        return 'AsioBlockable kind=%s next=%s wh=%s' % (
            self.kind(),
            self.next(),
            wh.to_string() if wh is not None else 'unresolved'
        )


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
    HhbcOpsPrinter,
    HhbbcBytecodePrinter,
    CompactVectorPrinter,
    SrcKeyPrinter,
    AsioBlockablePrinter,
    OptionalPrinter,
    ReqVectorPrinter,
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
    if typename is None:
        return None

    # Iterate over local dict of types to determine if a printer is registered
    # for that type.  Return an instantiation of the printer if found.
    for recognizer_regex, func in type_printers:
        if recognizer_regex.search(typename):
            return func(val)

    # Cannot find a pretty printer.  Return None.
    return None


gdb.pretty_printers.append(lookup_function)
