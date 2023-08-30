# Copyright 2022-present Facebook. All Rights Reserved

import abc
import argparse
import collections
import collections.abc
import functools
import lldb
import re
import shlex
import struct
import sys
import typing

class Command(abc.ABC):

    """ The name used to call the command """
    command: str

    """ A short, one-line description of the command """
    description: str

    """ Additional information to display after the usage """
    epilog: typing.Optional[str] = None

    def __init__(self, debugger: lldb.SBDebugger, _internal_dict):
        self.parser = self.create_parser()
        self.help_string = self.parser.format_help()

    @classmethod
    def register_lldb_command(cls, debugger, module_name, top_module="") -> None:
        parser = cls.create_parser()
        cls.__doc__ = parser.format_help()
        command = f"command script add -o -c {top_module + '.' if top_module else ''}{module_name}.{cls.__name__} {cls.command}"
        debugger.HandleCommand(command)

    @classmethod
    def default_parser(cls) -> argparse.ArgumentParser:
        parser = argparse.ArgumentParser(
            description=cls.description,
            prog=cls.command,
            epilog=cls.epilog,
            formatter_class=argparse.RawDescriptionHelpFormatter,
        )
        return parser

    def get_short_help(self) -> str:
        return self.description

    def get_long_help(self) -> str:
        return self.help_string

    @classmethod
    @abc.abstractmethod
    def create_parser(cls) -> argparse.ArgumentParser:
        """ Create and return an ArgumentParser object.

            Typical usage is to call .add_argument()
            as needed on the parser that default_parser() returns,
            and return that parser.
        """
        ...

    @abc.abstractmethod
    def __call__(self, debugger: lldb.SBDebugger, command: str, exe_ctx: lldb.SBExecutionContext, result: lldb.SBCommandReturnObject):
        ...

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
        if not isinstance(args, collections.abc.Hashable):
            return func(*args)
        if args not in cache:
            cache[args] = func(*args)
        return cache[args]
    return memoizer


#------------------------------------------------------------------------------
# Type and symbol lookup
#
# Note that we need to thread a lldb.Target around, unless
# we want to store it globally somewhere, because lldb.target returns None
# when within a script. I.e. the target object needs to come from the
# execution context that is passed when starting a command's execution.

#TODO(michristensen) Deal with the following lookup helpers not being hashable
def Type(name: str, target: lldb.SBTarget) -> lldb.SBType:
    """ Look up an HHVM type

    Raises an exception if the type cannot be found.

    Arguments:
        name: name of the type
        targ: optional target from the execution context

    Returns:
        An SBType wrapping the HHVM type
    """
    # T133615659: Using FindTypes(name).GetTypesAtIndex(0) because
    # it appears that sometimes FindFirstType(name) returns an empty type.
    ty = target.modules[0].FindTypes(name).GetTypeAtIndex(0)
    if not ty.IsValid():
        # If we can't find it in the first module (assuming it's the HHVM executable),
        # let's try it on the target, which might take longer but handles cases where
        # the type we want is in a different module. Looking in the first module first
        # let's us prioritize types defined in it.
        ty = target.FindTypes(name).GetTypeAtIndex(0)
    assert ty.IsValid(), f"couldn't find type '{name}'"
    return ty


def Global(name: str, target: lldb.SBTarget) -> lldb.SBValue:
    """ Look up the value of a global variable (including static)

    Falls back to evaluating the variable as an expression, which
    raises an exception on failure.

    Arguments:
        name: name of the global variable
        target: target from the execution context

    Returns:
        SBValue wrapping the global variable
    """
    # Search in hhvm module first to try and speed things up.
    g = target.modules[0].FindFirstGlobalVariable(target, name)
    if g.GetError().Fail():
        g = target.FindFirstGlobalVariable(name)
        if g.GetError().Fail():
            debug_print(f"couldn't find global variable '{name}'; attempting to find it by evaluating it")
            return Value(name, target)
    return g


def Enum(enum_name: str, elem: typing.Union[str, int], target: lldb.SBTarget) -> lldb.SBTypeEnumMember:
    """ Look up the value of an enum member

    Raises an exception if the enum or member cannot be found.

    Arguments:
        enum_name: name of the enumeration
        elem: name or index of the enumerator element
        target: target from the execution context

    Returns:
        SBTypeEnumMember wrapping the enumerator element
    """
    enum = Type(enum_name, target)
    assert enum.IsValid(), f"couldn't find enumeration '{enum_name}'"
    members = enum.GetEnumMembers()
    assert members.IsValid(), f"'{enum_name} is not an enumeration"
    val = members[elem]
    assert val is not None and val.IsValid(), f"couldn't find enumerator '{elem}' in '{enum_name}'"
    return val


def Value(name: str, target: lldb.SBTarget) -> lldb.SBValue:
    """ Look up the value of a symbol, by evaluating it as an expression

    Raises an exception if the symbol cannot be found/expression evaluated.
    You typically should call Global(), which calls this as a fallback.

    Arguments:
        name: the symbol to evaluate
        target: target from the execution context

    Returns:
        SBValue wrapping the value
    """
    v = target.EvaluateExpression(name)
    assert v.GetError().Success(), f"couldn't find symbol {name}"
    return v


#------------------------------------------------------------------------------
# STL accessors


def atomic_get(atomic: lldb.SBValue) -> lldb.SBValue:
    inner = rawtype(atomic.type).GetTemplateArgumentType(0)

    if inner.IsPointerType():
        return get(atomic, "_M_b", "_M_p")
    else:
        return get(atomic, "_M_i")


#------------------------------------------------------------------------------
# Member access helpers.


def get(struct: lldb.SBValue, *field_names: str) -> lldb.SBValue:
    """ Get the value of struct.field_names[0][field_names[1]]...

    Arguments:
        struct: The struct to reach into
        field_names: Name of the fields to extract. If more than one field name is
                     given, then the rest are treated as child members such that this
                     function is called recursively.

    Returns:
        The value of the field. Throws an assertion is anything goes wrong
        in the retrieval.

    This is supposed to be semi-equivalent to gdb's struct[field_name] syntax.
    """

    assert struct.GetError().Success(), f"invalid struct '{struct.name}'"
    # Note: You can also do lldb.value(val).<name>
    v = struct.GetChildMemberWithName(field_names[0])
    assert v.GetError().Success(), f"couldn't find field '{field_names[0]}' in struct '{struct.name}' with type '{struct.type.name}'"

    if len(field_names) == 1:
        return v
    else:
        return get(v, *field_names[1:])


#------------------------------------------------------------------------------
# Type manipulations


def rawtype(t: lldb.SBType):
    """ Remove const, volatile, and typedefs

    Arguments:
        t: type to translate

    Returns:
        An SBType object with qualifiers and layers of typedefs removed

    For examples:
        typedef HPHP::Native::NativeDataInfo::InitFunc ... -> void (*)(HPHP::ObjectData *)
        const HPHP::Class * -> HPHP::Class *
    """
    # NOTE: It appears GetCanonicalType() does the same thing
    #       as GetTypedefedType() for our purposes.
    #       Also look at GetBasicType().
    return t.GetUnqualifiedType().GetCanonicalType()

def destruct(t: str) -> str:
    """ Drop the class-key from a type name """
    return re.sub(r'^(struct|class|union)\s+', '', t)

def template_type(t: lldb.SBType) -> str:
    """Get the unparameterized name of a template type.

    Arguments:
        t: type to drop template parameters from

    Return:
        The unparameterized name

    For example:
        HPHP::VMFixedVector<ObjectProps::quick_index> -> 'HPHP::VMFixedVector'
    """
    return destruct(rawtype(t).name.split('<')[0])

def unsigned_cast(v: lldb.SBValue, t: lldb.SBValue) -> lldb.SBValue:
    """ Perform a cast of `v` to `t` with C compatible unsigned widening.

    Use this in place of SBValue::Cast for cases where you are widening a
    value (such as lowptr -> ptr conversion).  SBValue::Cast performs only a
    reinterpretation of the bits so for a widening conversion it potentially
    add garbage in the upper bits.

    Arguments:
        v: the value to cast

    Return:
        The value v casted to type `t` using C style semantics unsigned
        widening.
    """

    assert not v.type.IsAggregateType(), "Cannot cast aggregate type"
    assert v.GetByteSize() <= 8, "Value is too wide"

    # In theory we should use `v.path` to access value v in an expression.  In
    # practice this doesn't work because there are bugs in lldb.  Such as the
    # one that was being fixed in https://reviews.llvm.org/D132734,
    # which is reverted.  If this worked properly we could have this cast
    # operation follow C style semantics according to the types in play (as
    # opposed to forcing no sign extension).
    ret = v.target.EvaluateExpression(f"({t.name}){v.unsigned}")

    assert ret is not None and ret.GetError().Success(), f"Failed to cast {v} ({v.unsigned}) to {t.name}"
    return ret


#------------------------------------------------------------------------------
# Pointer helpers


def nullptr(target: lldb.SBTarget):
    """ Return an SBValue wrapping a pointer to 0 """
    return target.CreateValueFromExpression("nullptr", "(void *)0")


def is_nullptr(val: lldb.SBValue):
    return val.TypeIsPointerType() and val.unsigned == 0


def referenced_value(val: lldb.SBValue) -> lldb.SBValue:
    """ Get the value referenced by a pointer/reference, or the value itself otherwise """
    if val.type.IsReferenceType() or val.type.IsPointerType():
        return val.Dereference()
    return val


def rawptr(val: lldb.SBValue) -> typing.Optional[lldb.SBValue]:
    """Fully strip a smart pointer to a raw pointer. References are re-cast as pointers.

        Arguments:
            val: A smart pointer

        Returns:
            The stripped pointer, or None if it's not yet supported
    """
    #debug_print(f"rawptr(val=0x{val.unsigned:x})")

    if val.type.IsPointerType():
        return val
    elif val.type.IsReferenceType():
        return referenced_value(val)

    name = template_type(val.type)
    ptr = None

    if name == "std::unique_ptr":
        # This is a synthetic value, so we can just use its synthesized child.
        # We could also do utils.get(val.GetNonSyntheticValue(), "_M_t", "_M_t", "_M_head_impl")
        ptr = get(val, "pointer")
    elif name == "HPHP::req::ptr" or name == "HPHP::AtomicSharedPtrImpl":
        ptr = get(val, "m_px")
    elif name == "HPHP::LowPtr" or name == "HPHP::detail::LowPtrImpl":
        inner = val.type.GetTemplateArgumentType(0)
        ptr = get(val, "m_s")
        if name == "HPHP::detail::LowPtrImpl":
            storage_type = template_type(val.type.GetTemplateArgumentType(1))
            # Currently only used by AtomicLowPtr alias
            assert storage_type == "HPHP::detail::AtomicStorage"
            ptr = get(ptr, "_M_i")
        ptr = unsigned_cast(ptr, inner.GetPointerType())
    elif name == "HPHP::CompactTaggedPtr":
        inner = val.type.GetTemplateArgumentType(0)
        addr = get(val, "m_data").unsigned & 0xffffffffffff
        ptr = val.CreateValueFromExpression("(tmp)", f"({inner.GetPointerType()}) {addr}")
    elif name == "HPHP::CompactSizedPtr":
        ptr = rawptr(get(val, "m_data"))
    elif name == "HPHP::LockFreePtrWrapper":
        ptr = rawptr(get(val, "val"))
    elif name == "HPHP::TokenOrPtr":
        ptr = rawptr(TokenOrPtr.get_ptr(val))

    if ptr is not None:
        return rawptr(ptr)

    return None

kMaxTagSize = 16
kShiftAmount = 64 - kMaxTagSize  # 64 = std::numeric_limits<uintptr_t>::digits

class TokenOrPtr:
    @staticmethod
    def get_compact(token_or_ptr) -> int:
        compact = get(token_or_ptr, "m_compact")  # HPHP::CompactTaggedPtr
        data = compact.unsigned >> 2
        return data

    @staticmethod
    def get_tag(token_or_ptr) -> int:
        data = TokenOrPtr.get_compact(token_or_ptr)
        tag = data >> kShiftAmount
        return tag

    @staticmethod
    def is_ptr(token_or_ptr) -> bool:
        return TokenOrPtr.get_tag(token_or_ptr) == 0

    @staticmethod
    def is_token(token_or_ptr) -> bool:
        return TokenOrPtr.get_tag(token_or_ptr) == 1

    @staticmethod
    def get_ptr(token_or_ptr) -> lldb.SBValue:
        data = TokenOrPtr.get_compact(token_or_ptr)
        ptr = data & (-1 >> kMaxTagSize)
        return token_or_ptr.CreateValueFromExpression("tmp", f"(uintptr_t *){ptr}")

    @staticmethod
    def get_token(token_or_ptr) -> lldb.SBValue:
        ptr = TokenOrPtr.get_ptr(token_or_ptr)
        repo_token_type = Type("HPHP::RepoFile::Token", token_or_ptr.target)
        return unsigned_cast(ptr, repo_token_type)


def deref(val: lldb.SBValue) -> lldb.SBValue:
    """ Fully dereference a value, stripping away *, &, and all known smart
        pointer wrappers (as well as const/volatile qualifiers).

        Arguments:
            val: The value to dererefence

        Returns:
            The fully dereferenced value.
    """
    p = rawptr(val)

    if p is None:
        return val.Cast(rawtype(val.type))
    else:
        return deref(referenced_value(p))


def ptr_add(ptr: lldb.SBValue, n: typing.Union[int, lldb.SBValue], sizeof=True) -> lldb.SBValue:
    """ Create new a new pointer, pointing to value of ptr+(n*sizeof(*ptr))

    When sizeof=False, just add n, i.e. treating sizeof(*ptr) == 1

    Example:
        >>> pc
        (HPHP::PC) bc = 0x00007ffff1c00b00 "\U00000014\U00000003\U00000010"
        >>> pc1 = utils.ptr_add(pc, 1)
        >>> pc1
        (HPHP::PC) pc+1 = 0x00007ffff1c00b01 "\U00000003\U00000010"
    """

    assert ptr.TypeIsPointerType(), f"Expected ptr type, got {ptr.type.name}"

    if isinstance(n, lldb.SBValue):
        n = n.unsigned
    if sizeof:
        n *= ptr.type.GetPointeeType().size

    # TODO(michristensen) Creating the new pointer using the following two
    # commented-out lines was adding extraneous MSB bytes in some cases:
    #data = lldb.SBData.CreateDataFromInt(ptr.unsigned + n)
    #val = ptr.CreateValueFromData(f"{ptr.name}+{str(n)}", data, ptr.type)
    val = ptr.CreateValueFromExpression(
            f"{ptr.name}+{str(n)}"[-10:],
            f"({ptr.type.name}){ptr.unsigned + n}"
    )
    return val
    

#------------------------------------------------------------------------------
# Name accessor


def _full_func_name(func: lldb.SBValue) -> str:
    attrs = atomic_get(get(func, 'm_attrs', 'm_attrs'))
    if attrs.unsigned & Enum("HPHP::Attr", "AttrIsMethCaller", func.target).unsigned:
        cls = ""
    else:
        m_u = atomic_get(get(func, "m_u", "m_u"))
        cls = get(m_u, 'm_cls')
        if cls.unsigned == 0:
            cls = ""
        else:
            cls = nameof(unsigned_cast(cls, Type("HPHP::Class", cls.target).GetPointerType())) + "::"
    return cls + string_data_val(deref(get(func, 'm_name')))


def nameof(val: lldb.SBValue) -> typing.Optional[str]:
    """ Get the name of various HPHP objects, like functions or classes.

    Arguments:
        val: the value to get the name of (a Func, Class, or ObjectData)
    Returns:
        A Python string, if there is name associated with the object; otherwise None
    """
    val = deref(val)
    debug_print(f"nameof(val=0x{val.load_addr:x} (type={val.type.name}))")
    try:
       t = val.type.name
    except Exception:
       return None

    sd = None

    def _od_name(od):
        cls = deref(get(od, "m_cls"))
        pre_class = deref(get(cls, "m_preClass"))
        sd = get(pre_class, "m_name")
        return sd

    if t == "HPHP::Func":
        sd = get(val, "m_fullName")
        if rawptr(sd).unsigned == 1:
            return _full_func_name(val)
    elif t == "HPHP::Class":
        pre_class = deref(get(val, "m_preClass"))
        sd = get(pre_class, "m_name")
    elif t == "HPHP::LazyClassData":
        sd = get(val, "className")
    elif t == "HPHP::ObjectData":
        sd = _od_name(val)
    elif t == "HPHP::Object":
        sd = _od_name(deref(get(val, "m_obj")))

    if sd is None:
       return None

    return string_data_val(deref(sd))


#------------------------------------------------------------------------------
# Intel CRC32

def _bit_reflect(num, nbits):
    """ Perform bit reflection on the bottom 'nbits' of 'num' """

    out = 0
    mask = 1 << (nbits - 1)
    for i in range(nbits):
        if num & (1 << i):
            out |= mask
        mask >>= 1
    return out


def crc32q(crc, quad):
    """ Intel SSE4 CRC32 implementation """

    crc = _bit_reflect(crc, 32)
    quad = _bit_reflect(quad, 64)

    msb = 1 << 63

    dividend = quad ^ (crc << 32)
    divisor = 0x11edc6f41 << 31

    for _ in range(64):
        if dividend & msb:
            dividend ^= divisor
        dividend <<= 1

    return _bit_reflect(dividend, 64)


#------------------------------------------------------------------------------
# String helpers

def read_cstring(addr: typing.Union[int, lldb.SBValue], len: str, process: lldb.SBProcess, keep_case: bool = True) -> str:
    """ Read a null-terminated char * from memory at the given address or lldb.SBValue's load_addr

        LLDB can already format char * variables, like so:

            (char *) varname = 0x0000abcd "string value"

        This gets just the "string value" char array.
    """
    # If it's already a known char *, just parse its default LLDB summary,
    # which appears to be of the form '"some_str"'.
    if isinstance(addr, lldb.SBValue):
        if rawtype(addr.type).name == "char *":
            s = addr.summary
            if s and s[0] == '"':
                s = s[1:]
            if s and s[-1] == '"':
                s = s[:-1]
            if s:
                return s
            else:
                # Empty string, so let's try reading it from memory
                addr = addr.unsigned
        else:
            addr = addr.load_addr

    err = None

    try:
        error = lldb.SBError()
        cstring = process.ReadCStringFromMemory(addr, len, error)
        if error.Success():
            return cstring if keep_case else cstring.lower()
        else:
            err = error
    except SystemError as error:
        err = error

    print(f"error while trying to get string: {err}", file=sys.stderr)
    return f"<invalid string with addr 0x{addr:0x} and length {len}>"


def _unpack(s):
    return 0xdfdfdfdfdfdfdfdf & struct.unpack('<Q', bytes(s, encoding='utf-8'))[0]


def hash_string(s: str):
    """ Hash a string as in hphp/util/hash-crc-x64.S """

    size = len(s)
    tail_sz = size % 8
    size -= tail_sz

    crc = 0xffffffff

    for i in range(0, size, 8):
        crc = crc32q(crc, _unpack(s[i : i + 8]))

    if tail_sz == 0:
        return crc >> 1

    shift = -((tail_sz - 8) << 3) & 0b111111
    tail = _unpack(s[size:].ljust(8, '\0'))

    crc = crc32q(crc, tail << shift)
    return crc >> 1


def strinfo(s: lldb.SBValue, keep_case: bool = True):
    """ Return the Python string and HHVM hash for `s`, or None if `s` is not a stringish lldb.Value """

    data = None
    h = None

    try:
        t = rawtype(s.type)
    except Exception as err:
        print(f"error while trying to get the type of what we believe is a string-like value: {err}", file=sys.stderr)
        return None

    if (t == Type("char", s.target).GetPointerType()
          or re.match(r"char \[\d*\]$", t.name) is not None):
        # Note: 1024 is very arbitrary; the string may
        # very well be longer than this.
        data = read_cstring(s.deref.load_addr, 1024, s.process)
    else:
        sd = deref(s)
        ty_name = rawtype(sd.type).name
        if ty_name in ("HPHP::String", "HPHP::StaticString"):
            sd = rawptr(get(sd, "m_str"))
            sd = deref(sd)
        elif ty_name == "HPHP::StrNR":
            sd = deref(get(sd, "m_px"))

        if rawtype(sd.type).name != 'HPHP::StringData':
            return None

        data = string_data_val(sd)

        m_hash = get(sd.children[1], "m_hash").signed
        if m_hash != 0:
            h = m_hash & 0x7fffffff

    if data is None:
        return None

    assert isinstance(data, str)
    retval = {
        'data': data if keep_case else data.lower(),
        'hash': h if h is not None else hash_string(data),
    }
    return retval


def string_data_val(val: lldb.SBValue, keep_case=True) -> str:
    """ Convert an HPHP::StringData[*] to a Python string

    Arguments:
        val: A StringData* wrapped by an lldb.SBValue
        keep_case: If False, lowercase the returned string

    Returns:
        A Python string with the string value stored by the StringData
    """

    if val.type.IsPointerType():
        val = deref(val)
    assert val.type.name == "HPHP::StringData"

    addr = val.load_addr
    assert addr != lldb.LLDB_INVALID_ADDRESS, f"invalid string address 0x{val.load_addr:x}"
    addr += val.size
    m_len = val.children[1].GetChildMemberWithName("m_len").unsigned
    return read_cstring(addr, m_len + 1, val.process)

#------------------------------------------------------------------------------
# Resource helpers

def pretty_resource_data(data: lldb.SBValue, print_header=False) -> str:
    """ Convert an HPHP::ResourceData[*] to a Python string

    Arguments:
        val: A ResourceData* wrapped by an lldb.SBValue
        print_header: If True, also print the ResourceHeader the precedes it

    Returns:
        A Python string representing the contents of the ResourceData object
    """

    if data.type.IsPointerType():
        data = deref(data)

    assert data.type.name == "HPHP::ResourceData"
    return str(hex(data.load_addr))


def pretty_resource_header(header: lldb.SBValue, print_data=True) -> str:
    """ Convert an HPHP::ResourceHdr[*] to a Python string

    Arguments:
        val: A ResourceHdr* wrapped by an lldb.SBValue
        print_data: If True, also print the ResourceData that follows

    Returns:
        A Python string representing the contents of the ResourceHdr object
    """

    if header.type.IsPointerType():
        header = deref(header)

    assert header.type.name == "HPHP::ResourceHdr", f"Expected HPHP::ResourceHdr, got {header.type.name}"
    data = None
    if print_data:
        addr = header.load_addr
        addr += header.size
        data = header.CreateValueFromAddress("data", addr, Type("HPHP::ResourceData", header.target))
        data = pretty_resource_data(data)

    header = str(hex(header.load_addr))

    return f"(hdr = {header}" + (f", data = {data}" if data else "") + ")"


#------------------------------------------------------------------------------
# TypedValue helpers

_Current_key = None

def pretty_tv(typ: lldb.SBValue, data: lldb.SBValue) -> str:
    """ Get the pretty string representation of a TypedValue (or its subclasses)

    Note that calling str(val) on an SBValue will automatically use the
    pretty printer for that value, if present.

    Arguments:
        typ: A HPHP::DataType wrapped by an lldb.SBValue
        data: A HPHP::Value wrapped by an lldb.SBValue

    Returns:
        A Python string representing the TypedValue
    """

    target = typ.target
    typ = typ.Cast(Type("HPHP::DataType", target))

    def DT(elem):
        return Enum("HPHP::DataType", elem, target).unsigned

    val = None
    name = None

    if typ.unsigned in [DT("Uninit"), DT("Null")]:
        pass
    elif typ.unsigned == DT("Boolean"):
        val = get(data, "num").unsigned
        val = bool(val) if val in (0, 1) else val
    elif typ.unsigned == DT("Int64"):
        val = get(data, "num").signed
    elif typ.unsigned == DT("Double"):
        val = float(get(data, "dbl").value)
    elif typ.unsigned in (DT("String"), DT("PersistentString")):
        pstr = get(data, "pstr")
        val = '\"%s\"' % string_data_val(pstr)
    elif typ.unsigned in (
            DT("Dict"), DT("PersistentDict"),
            DT("Vec"), DT("PersistentVec"),
            DT("Keyset"), DT("PersistentKeyset")):
        val = deref(get(data, "parr"))
    elif typ.unsigned == DT("Object"):
        val = get(data, "pobj")
    elif typ.unsigned == DT("Resource"):
        val = deref(get(data, "pres"))
        val = pretty_resource_header(val)
    elif typ.unsigned == DT("Class"):
        val = get(data, "pclass")
    elif typ.unsigned == DT("LazyClass"):
        val = get(data, "plazyclass")
    elif typ.unsigned == DT("Func"):
        val = get(data, "pfunc")
    elif typ.unsigned == DT("ClsMeth"):
        # For non-lowptr, m_data is a pointer, so try and dereference first
        val = referenced_value(get(data, "pclsmeth", "m_data"))
        cls = unsigned_cast(get(val, "m_cls"), Type("HPHP::Class", target).GetPointerType())
        func = unsigned_cast(get(val, "m_func"), Type("HPHP::Func", target).GetPointerType())
        name = f"{nameof(cls)}::{nameof(func)}"
    elif typ.unsigned == DT("RFunc"):
        val = get(data, "prfunc")
        func = get(val, "m_func")
        #arr = get(val, "m_arr")  # TODO use to print array contents
        name = nameof(func)
    elif typ.unsigned == DT("RClsMeth"):
        val = get(data, "prclsmeth")
        cls = get(val, "m_cls")
        func = get(val, "m_func")
        #arr = get(val, "m_arr")  # TODO use to print array contents
        name = f"{nameof(cls)}::{nameof(func)}"
    else:
        typ_as_int8_t = typ.Cast(Type("int8_t", target))
        num = get(data, "num").signed
        typ = 'Invalid Type (%d)' % typ_as_int8_t.signed
        val = '0x%x' % num

    if isinstance(typ, lldb.SBValue):
        typ = typ.value

    if val is None:
        out = '{ %s }' % typ
    elif name is None:
        out = '{ %s, %s }' % (typ, str(val))
    else:
        out = '{ %s, %s ("%s") }' % (typ, str(val), name)

    return out


#------------------------------------------------------------------------------
# Array helpers

def has_array_kind(array_data: lldb.SBValue, *kinds: str) -> bool:
    """ Determine if array_data has a particular kind (e.g. 'Vec', 'Keyset', etc.)

    Arguments:
        array_data: an SBValue wrapping an ArrayData
        kinds: one or more ArrayKind members
    
    Returns:
        True if the array data's kind is one of those specified in kinds
    """

    heap_obj = array_data.children[0].children[0]  # HPHP::HeapObject
    m_kind = get(heap_obj, "m_kind")

    for kind in kinds:
        kind = Enum("HPHP::ArrayData::ArrayKind", "k" + kind + "Kind", array_data.target).unsigned
        if m_kind.unsigned == kind:
            return True
    return False


def cast_as_specialized_array_data_kind(array_data: lldb.SBValue) -> lldb.SBValue:
    # Currently does *not* take in a pointer to an ArrayData, but rather the value itself
    debug_print(f"cast_as_specialized_array_data_kind(array_data=0x{array_data.load_addr:x} (type={array_data.type.name}))")
    heap_obj = array_data.children[0].children[0]  # HPHP::HeapObject
    m_kind = get(heap_obj, "m_kind")
    if has_array_kind(array_data, 'Vec'):
        pass
    elif has_array_kind(array_data, 'Dict'):
        array_data = array_data.address_of.Cast(Type("HPHP::VanillaDict", array_data.target).GetPointerType()).deref
    elif has_array_kind(array_data, 'Keyset'):
        array_data = array_data.address_of.Cast(Type("HPHP::VanillaKeyset", array_data.target).GetPointerType()).deref
    elif has_array_kind(array_data, 'BespokeVec', 'BespokeDict', 'BespokeKeyset'):
        raise Exception(f"Unsupported bespoke array type ('{m_kind}')! Run `expression -R -- {array_data.path}` to see its raw form")
    else:
        raise Exception(f"Invalid array type ('{m_kind}')! Run `expression -R -- {array_data.path}` to see its raw form")

    if array_data.GetError().Fail():
        raise Exception(f"Unable to properly cast array data to specialized type: {array_data.GetError().GetCString()}")
    return array_data


#------------------------------------------------------------------------------
# Architecture


def arch(target: lldb.SBTarget) -> str:
    """ Get the current architecture (e.g. "x86_64" or "aarch64") """
    try:
        return target.triple.split("-")[0]
    except Exception:
        return None


def arch_regs(target: lldb.SBTarget) -> typing.Dict[str, str]:
    """ Get a mapping from common register names to the actual register names
        as used in either x86 or ARM

    Arguments:
        target: the target associated with the current debugging session

    Returns:
        A mapping from register name to register name
    """
    a = arch(target)

    # TODO check that this is the architecture string returned from the first part
    #      of the `arch()` triple when running on ARM
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

def reg(name: str, frame: lldb.SBFrame) -> lldb.SBValue:
    """ Get the value of a register given its common name (e.g. "fp", "sp", etc.)

    Arguments:
        name: Name of the register
        frame: Current frame

    Returns:
        The value of the register, wrapped in a lldb.SBValue. If unrecognized,
        the returned SBValue will be invalid (check with .isValid()).
    """
    name = arch_regs(frame.thread.process.target)[name]
    return frame.register[name]


#------------------------------------------------------------------------------
# General-purpose helpers

def parse_argv(args: str, target: lldb.SBTarget, limit=None) -> typing.List[lldb.SBValue]:
    """ Explode a LLDB argument string, then evaluate all args up to `limit`.
    
        It assumes that each arg is space-separated, meaning e.g. "bc+bclen" is one argument,
        but "bc + bclen" is three.
    """

    # I can't figure out a way to successfully catch lldb parse errors using normal
    # Python exception handling, so this is best effort.

    if limit is None:
        limit = len(args)
    return [target.EvaluateExpression(arg) if i < limit else arg
            for i, arg in enumerate(shlex.split(args))]


#------------------------------------------------------------------------------
# Debugging

_Debug = False

class DebugCommand(Command):
    command = "debug"
    description = "Enable/disable printing information to aid in debugging LLDB scripts"

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        subparsers = parser.add_subparsers(dest='cmd')
        subparsers.add_parser('on', help='Enable LLDB script debugging information')
        subparsers.add_parser('off', help='Disable LLDB script debugging information')
        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args)
        except SystemExit:
            result.SetError("option parsing failed")
            return

        global _Debug
        if options.cmd == 'on':
            _Debug = True
        elif options.cmd == 'off':
            _Debug = False
        else:
            result.SetError(f"Unexpected command {options.cmd}")


def debug_print(message: str, file=sys.stderr) -> None:
    if _Debug:
        print(message)


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    DebugCommand.register_lldb_command(debugger, __name__, top_module)
