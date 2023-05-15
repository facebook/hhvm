# Copyright 2022-present Facebook. All Rights Reserved

import abc
import argparse
import collections
import collections.abc
import functools
import lldb
import re
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

    Arguments:
        name: name of the type
        targ: optional target from the execution context

    Returns:
        An SBType wrapping the HHVM type
    """
    # T133615659: It appears that sometimes FindFirstType(name) returns an empty type,
    # even though FindTypes(name).GetTypesAtIndex(0) returns the type we want.
    ty = target.FindFirstType(name)
    if not ty.IsValid():
        ty = target.FindTypes(name).GetTypeAtIndex(0)
    assert ty.IsValid(), f"couldn't find type '{name}'"
    assert ty.name == name, f"type names don't match ({name} vs {ty.name})"
    return ty


def Global(name: str, target: lldb.SBTarget) -> lldb.SBValue:
    """ Look up the value of a global variable (including static)

    Arguments:
        name: name of the global variable
        targ: target from the execution context

    Returns:
        SBValue wrapping the global variable
    """
    g = target.FindFirstGlobalVariable(name)
    assert g.IsValid(), f"couldn't find global variable '{name}'"
    return g


def Enum(enum_name: str, elem: typing.Union[str, int], target: lldb.SBTarget) -> lldb.SBTypeEnumMember:
    """ Look up the value of an enum member

    Arguments:
        enum_name: name of the enumeration
        elem: name or index of the enumerator element

    Returns:
        SBTypeEnumMember wrapping the enumerator element
    """
    enum = Type(enum_name, target)
    assert enum.IsValid(), f"couldn't find enumeration '{enum_name}'"
    members = enum.GetEnumMembers()
    assert members.IsValid(), f"'{enum_name} is not an enumeration"
    val = members[elem]
    assert val.IsValid(), f"couldn't find enumerator '{elem}' in '{enum_name}'"
    return val


def Value(name: str, target: lldb.SBTarget) -> lldb.SBValue:
    """ Look up the value of a symbol """
    v = target.CreateValueFromExpression("(tmp)", name)
    assert v.IsValid(), f"couldn't find symbol {name}"
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

    assert struct.IsValid(), f"invalid struct '{struct.name}'"
    # Note: You can also do lldb.value(val).<name>
    v = struct.GetChildMemberWithName(field_names[0])
    assert v.IsValid(), f"couldn't find field '{field_names[0]}' in struct '{struct.name}' with type '{struct.type.name}'"

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


#------------------------------------------------------------------------------
# Pointer helpers


def nullptr(target: lldb.SBTarget):
    """ Return an SBValue wrapping a pointer to 0 """
    return target.CreateValueFromExpression("nullptr", "(void *)0")


def is_nullptr(ptr: lldb.SBValue):
    assert ptr.type.IsPointerType()
    return ptr.unsigned == 0


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
    if val.type.IsPointerType():
        return val
    elif val.type.IsReferenceType():
        return referenced_value(val).addr

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
        ptr = ptr.Cast(inner.GetPointerType())
    elif name == "HPHP::CompactTaggedPtr":
        inner = val.type.GetTemplateArgumentType(0)
        addr = get(val, "m_data").unsigned & 0xffffffffffff
        ptr = val.CreateValueFromAddress("(tmp)", addr, inner.GetPointerType())
    elif name == "HPHP::CompactSizedPtr":
        ptr = rawptr(get(val, "m_data"))
    elif name == "HPHP::LockFreePtrWrapper":
        ptr = rawptr(get(val, "val"))
    elif name == "HPHP::TokenOrPtr":
        compact = get(val, "m_compact")  # HPHP::CompactTaggedPtr
        data = val.CreateValueFromExpression("(tmp)", str(compact.unsigned >> 2))
        ptr = rawptr(data.Cast(compact.type.GetPointerType()))  # Stop the recursion by turning into pointer

    if ptr is not None:
        return rawptr(ptr)

    return None


def deref(val: lldb.SBValue) -> lldb.SBValue:
    """ Fully dereference a value, stripping away *, &, and all known smart
        pointer wrappers (as well as const/volatile qualifiers).

        Arguments:
            val: The value to derefence

        Returns:
            The fully dereferenced value.
    """
    p = rawptr(val)

    if p is None:
        return val.Cast(rawtype(val.type))
    else:
        return deref(referenced_value(p))


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
            cls = nameof(cls.Cast(Type("HPHP::Class", cls.target).GetPointerType())) + "::"
    return cls + string_data_val(deref(get(func, 'm_name')))


def nameof(val: lldb.SBValue) -> typing.Optional[str]:
    """ Get the name of various HPHP objects, like functions or classes.

    Arguments:
        val: the value to get the name of (a Func, Class, or ObjectData)
    Returns:
        A Python string, if there is name associated with the object; otherwise None
    """
    val = deref(val)
    try:
       t = val.type.name
    except Exception:
       return None

    sd = None

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
        cls = deref(get(val, "m_cls"))
        pre_class = deref(get(cls, "m_preClass"))
        sd = get(pre_class, "m_name")

    if sd is None:
       return None

    return string_data_val(deref(sd))


#------------------------------------------------------------------------------
# String helpers

def read_cstring(addr: typing.Union[int, lldb.SBValue], len: str, process: lldb.SBProcess, keep_case: bool = True) -> str:
    """ Read a null-terminated char * from memory at the given address or lldb.SBValue's load_addr

        LLDB can already format char * variables, like so:

            (char *) varname = 0x0000abcd "string value"

        This gets just the "string value" char array.
        We could probably also just parse the result of str(val) or val.summary.
    """
    if isinstance(addr, lldb.SBValue):
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
    assert addr != lldb.LLDB_INVALID_ADDRESS, f"invalid string address {val.load_addr}"
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


def pretty_tv(typ: lldb.SBValue, data: lldb.SBValue) -> str:
    """ Get the pretty string representation of a TypedValue (or its subclasses)

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
    elif typ.unsigned == DT("Object"):
        val = get(data, "pobj")
        name = nameof(val)
    elif typ.unsigned == DT("Resource"):
        val = deref(get(data, "pres"))
        val = pretty_resource_header(val)
    elif typ.unsigned == DT("Class"):
        val = get(data, "pclass")
        name = nameof(val)
    elif typ.unsigned == DT("LazyClass"):
        val = get(data, "plazyclass")
        name = nameof(val)
    elif typ.unsigned == DT("Func"):
        val = get(data, "pfunc")
        name = nameof(val)
    elif typ.unsigned == DT("ClsMeth"):
        # For non-lowptr, m_data is a pointer, so try and dereference first
        val = referenced_value(get(data, "pclsmeth", "m_data"))
        cls = get(val, "m_cls").Cast(Type("HPHP::Class", target).GetPointerType())
        func = get(val, "m_func").Cast(Type("HPHP::Func", target).GetPointerType())
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

def reg(name: str, frame: lldb.SBFrame) -> typing.Optional[lldb.SBValue]:
    """ Get the value of a register given its common name (e.g. "fp", "sp", etc.)

    Arguments:
        name: Name of the register
        frame: Current frame

    Returns:
        The value of the register, wrapped in a lldb.SBValue, or None if unrecognized
    """
    name = arch_regs(frame.thread.process.target)[name]
    return frame.register[name]
