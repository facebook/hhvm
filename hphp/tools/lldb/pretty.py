# Copyright 2022-present Facebook. All Rights Reserved.

# pyre-strict

"""Pretty printers for HPHP types"""

import re
import traceback
import typing

import lldb

# pyre-fixme[21]: Could not find module `sizeof`.
import sizeof

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    # pyre-fixme[21]: Could not find module `idx`.
    import idx

    # pyre-fixme[21]: Could not find module `utils`.
    import utils
except ModuleNotFoundError:
    import hphp.tools.lldb.idx as idx
    import hphp.tools.lldb.utils as utils


Formatters: typing.List[typing.Callable[[str], str]] = []


class FormatError(Exception):
    """Raised by a pretty-printer when it cannot produce a useful summary.

    The @format wrapper catches this and falls back to the unformatted display.
    """

    pass


def _default_summary(val: lldb.SBValue) -> str:
    """Reconstruct the default LLDB display for a value whose formatter failed."""
    v = val.GetNonSyntheticValue()
    if v.value is not None:
        return v.value
    parts = []
    for i in range(v.num_children):
        child = v.GetChildAtIndex(i)
        child_repr = child.summary or child.value or ""
        parts.append(f"{child.name} = {child_repr}")
    if parts:
        return "(" + ", ".join(parts) + ")"
    return ""


# noqa: C901
def format(
    datatype: str,
    regex: bool = False,
    skip_pointers: bool = False,
    skip_references: bool = False,
    synthetic_children: bool = False,
) -> typing.Callable[[typing.Any], typing.Any]:
    """Wrapper for pretty printer functions.

    Add the command needed to register the pretty printer with the LLDB debugger
    session once started, to the global Formatters list.

    Will wrap the function to skip trying to print null pointers,
    and will revert to using the default (unformatted) version if pretty printers
    fail for some reason.

    Arguments:
        datatype: the name of the data type being formatted
        regex: whether the datatype in string is a regex

    Returns:
        The original function
    """

    def inner(func_or_class: typing.Any) -> typing.Any:
        extra_options = []
        if regex:
            extra_options.append("-x")
        if skip_pointers:
            extra_options.append("-p")
        if skip_references:
            extra_options.append("-r")
        extra_options = " ".join(extra_options)

        if synthetic_children:
            assert isinstance(func_or_class, type), (
                "Can only use synthetic_children=True in @format decorator on classes"
            )
            Formatters.append(
                lambda top_module: f"type synthetic add {extra_options} "
                f'--python-class {top_module + "." if top_module else ""}pretty.{func_or_class.__name__} "{datatype}"'
            )
            # Modify the top-level summary of this type
            if hasattr(func_or_class, "summary"):
                Formatters.append(
                    lambda top_module: f"type summary add --expand {extra_options} "
                    f'--summary-string "{func_or_class.summary()}" "{datatype}"'
                )
            return func_or_class
        else:
            Formatters.append(
                lambda top_module: f"type summary add {extra_options} "
                f'--python-function {top_module + "." if top_module else ""}pretty.{func_or_class.__name__} "{datatype}"'
            )

            def wrapper(
                val_obj: lldb.SBValue, internal_dict: typing.Dict[str, typing.Any]
            ) -> typing.Any:
                # When given a nullptr, just print the address, rather than try and probably fail
                # to get its contents in whatever pretty printers would normally be called.
                if utils.is_nullptr(val_obj):
                    return "0x0"

                # When the pretty printer for this value fails for some reason,
                # just show the unformatted version.
                try:
                    result = func_or_class(val_obj, internal_dict)
                    if result is None:
                        raise FormatError("formatter returned None")
                    return result
                except FormatError:
                    pass
                except Exception:
                    utils.debug_print(
                        f"Failed to pretty print '{val_obj.name}' in {func_or_class.__name__}()"
                    )
                    if utils._Debug:
                        traceback.print_exc()
                return _default_summary(val_obj)

            return wrapper

    return inner


# ------------------------------------------------------------------------------
# NOTE: the functions prefixed with "pp_" all have the following signature:
#
#   def pp_<TypeName>(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
#      """ Pretty print HPHP::<TypeName>
#          Arguments:
#              val_obj: an SBValue wrapping an HPHP::<TypeName>
#              internal_dict: an LLDB support object not to be used
#
#          Returns:
#              A string representing the <TypeName>.
#          Raises:
#              FormatError if the value cannot be formatted.
#      """

# ------------------------------------------------------------------------------
# TypedValues and its subtypes


@format("^HPHP::((Unaligned)?TypedValue|Variant|VarNR)$", regex=True)
def pp_TypedValue(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        m_type = utils.get(val_obj, "m_type")
        m_data = utils.get(val_obj, "m_data")
    except AssertionError as e:
        raise FormatError(str(e))
    return utils.pretty_tv(m_type, m_data)


# ------------------------------------------------------------------------------
# Pointers


def pretty_ptr(val: lldb.SBValue) -> str:
    utils.debug_print(f"pretty_ptr(val=0x{val.unsigned:x})")

    ptr = utils.rawptr(val)

    if ptr is None:
        raise FormatError("rawptr returned None")
    if utils.is_nullptr(ptr):
        return "0x0"

    inner = utils.deref(ptr)
    inner_type = utils.rawtype(inner.type)

    try:
        if inner_type.name == "HPHP::StringData":
            s = utils.string_data_val(inner)
        else:
            s = utils.nameof(inner)
    except AssertionError as e:
        raise FormatError(str(e))
    if s is None:
        raise FormatError("nameof returned None")
    return '"' + s + '"'


@format("^HPHP::req::ptr<.*>$", regex=True)
def pp_ReqPtr(val_obj: lldb.SBValue, _internal_dict) -> str:
    return pretty_ptr(val_obj)


@format("^HPHP::(PackedPtr<.*>|ptrimpl::PtrImpl<.*>)$", regex=True)
def pp_PackedPtr(val_obj: lldb.SBValue, _internal_dict) -> str:
    return pretty_ptr(val_obj)


@format("^HPHP::FuncId$", regex=True)
def pp_FuncId(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        m_id = utils.get(val_obj, "m_id")
    except AssertionError as e:
        raise FormatError(str(e))
    m_s = m_id.GetChildMemberWithName("m_s")
    if m_s.IsValid():
        return str(m_s.unsigned)
    return str(m_id.unsigned)


# ------------------------------------------------------------------------------
# Resource


@format("^HPHP::OptResource$", regex=True)
def pp_Resource(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        inner = utils.get(val_obj, "m_res")
    except AssertionError as e:
        raise FormatError(str(e))
    val = utils.rawptr(inner)
    if val is None:
        raise FormatError("rawptr returned None for OptResource")
    try:
        return utils.pretty_resource_header(val)
    except AssertionError as e:
        raise FormatError(str(e))


# ------------------------------------------------------------------------------
# Strings


@format("^HPHP::StringData$", regex=True)
def pp_StringData(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        return '"' + utils.string_data_val(val_obj) + '"'
    except AssertionError as e:
        raise FormatError(str(e))


@format("^HPHP::(Static)?String$", regex=True)
def pp_String(val_obj: lldb.SBValue, _internal_dict) -> str:
    # Note: SBValue.GetChildMemberWithName(), used by utils.get(),
    # will get the members of both pointers and the pointed-to values themselves
    try:
        inner = utils.get(val_obj, "m_str")
    except AssertionError as e:
        raise FormatError(str(e))
    val = utils.rawptr(inner)
    if val is None:
        raise FormatError("rawptr returned None for String")
    try:
        return '"' + utils.string_data_val(val) + '"'
    except AssertionError as e:
        raise FormatError(str(e))


@format("^HPHP::StrNR$", regex=True)
def pp_StrNR(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        val = utils.get(val_obj, "m_px")
    except AssertionError as e:
        raise FormatError(str(e))
    try:
        return '"' + utils.string_data_val(val) + '"'
    except AssertionError as e:
        raise FormatError(str(e))


# ------------------------------------------------------------------------------
# Optional


@format("^HPHP(::req)?::Optional<.*>$", regex=True)
def pp_Optional(val_obj: lldb.SBValue, _internal_dict) -> str:
    """See:
    * hphp/runtime/base/req-optional.h
    * hphp/util/optional.h
    """
    try:
        val = utils.get(val_obj, "m_opt")
    except AssertionError as e:
        raise FormatError(str(e))
    val = val.children[0] if val.children else None
    return str(val)


# ------------------------------------------------------------------------------
# Arrays


@format("^HPHP::ArrayData$", regex=True, synthetic_children=True)
class pp_ArrayData:
    """This conforms to the SyntheticChildrenProvider interface"""

    @staticmethod
    def summary() -> str:
        # Ideally we'd print the refcount and kind (e.g. Vec/Dict/Keyset),
        # but there's no easy way to do that with the synthetic children API
        # (we have access to a limited set of formatting summary elements),
        # and it doesn't look like we can supply a summary function when
        # supplying a synthetic children provider.
        # return f"ArrayData[{self.m_kind.value}]: {self.m_size} element(s) refcount={self.m_count}"
        return "${svar%#} element(s)"

    def __init__(
        self, val_obj: lldb.SBValue, _internal_dict: typing.Dict[str, typing.Any]
    ) -> None:
        # We use this class for both the synthetic children and for the summary.
        # For the summary, we will be given the synthetic lldb.SBValue so we
        # must make sure to get the non-synthetic lldb.SBValue.
        utils.debug_print(
            f"pp_ArrayData::__init__ with val_obj (load_addr: 0x{val_obj.load_addr:x}, type: {val_obj.type.name})"
        )
        self.val_obj: lldb.SBValue = val_obj.GetNonSyntheticValue()
        self.size: int | None = None
        self.func: None = None
        self.at_func: typing.Callable[[int], lldb.SBValue | None] | None = None
        self.update()

    def num_children(self) -> int:
        if self.size is None:
            utils.debug_print(
                "Unable to determine number of children of ArrayData object, returning 0"
            )
            return 0
        return self.size

    def get_child_index(self, name: str) -> int:
        try:
            return int(name.lstrip("[").rstrip("]"))
        except ValueError:
            return -1

    def get_child_at_index(self, index: int) -> lldb.SBValue | None:
        utils.debug_print(f"pp_ArrayData::get_child_at_index with index {index}")
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        if self.at_func is None:
            utils.debug_print("Invalid array type!")
            return None
        try:
            return self.at_func(index)
        except Exception:
            utils.debug_print(f"Exception in get_child_at_index for index {index}")
            if utils._Debug:
                traceback.print_exc()
            return None

    def update(self) -> bool:
        try:
            return self._update()
        except Exception:
            utils.debug_print("Exception while pretty printing ArrayData")
            if utils._Debug:
                traceback.print_exc()
            return False

    def _update(self) -> bool:
        # Doing this in here, rather than __init__(), because the API
        # says we should be re-updating internal state as much as possible (since the
        # state of variables can change since the last invocation).
        self.size = sizeof.array_data_size(self.val_obj)

        specialized_obj = utils.cast_as_specialized_array_data_kind(self.val_obj)
        utils.debug_print(
            f"pp_ArrayData::_update() with specialized_obj (type {specialized_obj.type.name}); "
            f"specialized_obj.load_addr: 0x{specialized_obj.load_addr:x} + specialized_obj.type.size: {specialized_obj.type.size}"
        )
        char_ptr_type = utils.Type("char", self.val_obj.target).GetPointerType()
        base = specialized_obj.CreateValueFromAddress(
            "tmp", specialized_obj.load_addr + specialized_obj.type.size, char_ptr_type
        )
        assert base.GetError().Success(), "Couldn't get base address of array"

        if utils.has_array_kind(self.val_obj, "Vec"):
            self.at_func = lambda ix: idx.vec_at(base, ix)
        elif utils.has_array_kind(self.val_obj, "Dict"):
            self.at_func = lambda ix: idx.dict_at(base, ix)
        elif utils.has_array_kind(self.val_obj, "Keyset"):
            self.at_func = lambda ix: idx.keyset_at(base, ix)
        else:
            utils.debug_print("Could not determine array kind")

        # Return false to make sure we always update this object every time we
        # stop. If we return True, then the value will never update again.
        return False


@format("^HPHP::Array$", regex=True, synthetic_children=True)
class pp_Array(pp_ArrayData):
    def __init__(
        self, val_obj: lldb.SBValue, _internal_dict: typing.Dict[str, typing.Any]
    ) -> None:
        if val_obj.GetError().Fail():
            utils.debug_print(
                f"Invalid array. Error: {val_obj.GetError().GetCString()}"
            )
            return

        try:
            val = utils.deref(utils.get(val_obj, "m_arr"))
        except Exception:
            utils.debug_print("Exception getting m_arr from Array")
            if utils._Debug:
                traceback.print_exc()
            return
        super().__init__(val, _internal_dict)


# ------------------------------------------------------------------------------
# Classes, Functions, and Objects


@format("^HPHP::(Class|LazyClassData|ObjectData)$", regex=True)
def pp_NamedValue(val_obj: lldb.SBValue, _internal_dict) -> str:
    s = utils.nameof(val_obj)
    if s is None:
        raise FormatError("nameof returned None")
    return '"' + s + '"'


_FUNC_ATTRS: typing.List[typing.Tuple[int, str]] = [
    (0x1, "HasInheritedReturnTypes"),
    (0x2, "Public"),
    (0x4, "Protected"),
    (0x8, "Private"),
    (0x10, "Static"),
    (0x20, "Abstract"),
    (0x40, "Final"),
    (0x80, "SupportsAsyncEagerReturn"),
    (0x100, "Trait"),
    (0x200, "NoInjection"),
    (0x800, "Interceptable"),
    (0x1000, "SplatParam"),
    (0x2000, "NoOverride"),
    (0x4000, "ReadonlyThis"),
    (0x8000, "ReadonlyReturn"),
    (0x10000, "Internal"),
    (0x20000, "InternalSoft"),
    (0x40000, "Persistent"),
    (0x80000, "DynamicallyCallable"),
    (0x100000, "Builtin"),
    (0x200000, "HasAttributionData"),
    (0x1000000, "IsMethCaller"),
    (0x2000000, "HasCoeffectRules"),
    (0x4000000, "IsFoldable"),
    (0x8000000, "NoFCallBuiltin"),
    (0x10000000, "VariadicParam"),
    (0x20000000, "ProvenanceSkipFrame"),
    (0x40000000, "NoRecording"),
]

_FUNC_ATOMIC_FLAGS: typing.List[typing.Tuple[int, str]] = [
    (0x01, "Optimized"),
    (0x02, "Locked"),
    (0x04, "MaybeIntercepted"),
    (0x08, "LockedForPrologueGen"),
    (0x10, "Zombie"),
    (0x20, "LockedForAsyncJit"),
]

_FUNC_ALL_FLAGS: typing.List[typing.Tuple[int, str]] = [
    (0x01, "isPreFunc"),
    (0x02, "hasPrivateAncestor"),
    (0x04, "shouldSampleJit"),
    (0x08, "hasForeignThis"),
    (0x10, "registeredInDataMap"),
    (0x20, "hasNamedParams"),
]


def _decode_flags(val: int, flag_defs: typing.List[typing.Tuple[int, str]]) -> str:
    names = []
    remaining = val
    for bit, name in flag_defs:
        if val & bit:
            names.append(name)
            remaining &= ~bit
    if remaining:
        names.append(f"0x{remaining:x}")
    return "|".join(names) if names else "None"


def _read_atomic(val: lldb.SBValue) -> int:
    try:
        return utils.atomic_get(val).unsigned
    except Exception:
        pass
    m_i = val.GetChildMemberWithName("_M_i")
    if m_i.IsValid() and m_i.value is not None:
        return m_i.unsigned
    if val.num_children > 0:
        return _read_atomic(val.GetChildAtIndex(0))
    if val.value is not None:
        return val.unsigned
    raise FormatError("Cannot read atomic value")


# Keyed by uint16_t RuntimeCoeffects::m_data; at most ~15 distinct values in practice.
_coeffect_cache: typing.Dict[int, str] = {}


def _coeffect_str(raw: int, target: lldb.SBTarget) -> str:
    if raw in _coeffect_cache:
        return _coeffect_cache[raw]
    if raw == 0:
        _coeffect_cache[0] = "pure"
        return "pure"
    try:
        result = target.EvaluateExpression(
            f"HPHP::RuntimeCoeffects::fromValue({raw}).toString()"
        )
        if result.error.Success() and result.summary:
            s = result.summary.strip('"')
            if s:
                _coeffect_cache[raw] = s
                return s
    except Exception:
        pass
    fallback = f"0x{raw:x}"
    _coeffect_cache[raw] = fallback
    return fallback


def _rawptr_hex(val: lldb.SBValue) -> str:
    ptr = utils.rawptr(val)
    if ptr is None or utils.is_nullptr(ptr):
        return "0x0"
    return f"0x{ptr.unsigned:x}"


@format("^HPHP::Func$", regex=True)
def pp_Func(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        name = utils.nameof(val_obj)
    except Exception:
        name = None

    parts: typing.List[str] = []

    def _add(label: str, fn: typing.Callable[[], str]) -> None:
        try:
            parts.append(f"{label}={fn()}")
        except Exception:
            pass

    _add("funcEntry", lambda: _rawptr_hex(utils.get(val_obj, "m_funcEntry")))

    def _fmt_func_id() -> str:
        func_id = utils.get(val_obj, "m_funcId")
        m_id = utils.get(func_id, "m_id")
        m_s = m_id.GetChildMemberWithName("m_s")
        if m_s.IsValid():
            return str(m_s.unsigned)
        return str(m_id.unsigned)

    _add("funcId", _fmt_func_id)

    def _fmt_string_ptr(field: str) -> str:
        ptr = utils.rawptr(utils.get(val_obj, field))
        if ptr is None or utils.is_nullptr(ptr):
            return "null"
        if ptr.unsigned <= 1:
            return "<pending>"
        return f'"{utils.string_data_val(utils.deref(ptr))}"'

    _add("fullName", lambda: _fmt_string_ptr("m_fullName"))
    _add("name", lambda: _fmt_string_ptr("m_name"))

    def _fmt_cls() -> str:
        m_u = utils.get(val_obj, "m_u")
        cls_ptr = utils.rawptr(utils.get(m_u, "m_cls", "m_base"))
        if cls_ptr is None or utils.is_nullptr(cls_ptr):
            return "null"
        cls_name = utils.nameof(utils.deref(cls_ptr))
        return f'"{cls_name}"' if cls_name else f"0x{cls_ptr.unsigned:x}"

    _add("cls", _fmt_cls)

    _add("methodSlot", lambda: str(utils.get(val_obj, "m_methodSlot").unsigned))
    _add(
        "cloned",
        lambda: str(bool(_read_atomic(utils.get(val_obj, "m_cloned")))).lower(),
    )

    _add(
        "atomicFlags",
        lambda: _decode_flags(
            _read_atomic(utils.get(val_obj, "m_atomicFlags", "m_flags")),
            _FUNC_ATOMIC_FLAGS,
        ),
    )
    # Func::m_allFlags is an AllFlags union; AllFlags::m_allFlags is its uint8_t member.
    _add(
        "allFlags",
        lambda: _decode_flags(
            utils.get(val_obj, "m_allFlags", "m_allFlags").unsigned,
            _FUNC_ALL_FLAGS,
        ),
    )

    _add("jitReqCount", lambda: str(_read_atomic(utils.get(val_obj, "m_jitReqCount"))))

    def _fmt_coeffects() -> str:
        coeffects = utils.get(val_obj, "m_requiredCoeffects")
        m_data = coeffects.GetChildMemberWithName("m_data")
        if m_data.IsValid() and m_data.value is not None:
            raw = m_data.unsigned
        else:
            raw = _read_atomic(coeffects)
        return _coeffect_str(raw, val_obj.target)

    _add("coeffects", _fmt_coeffects)

    _add("maxStack", lambda: str(utils.get(val_obj, "m_maxStackCells").signed))
    _add("unit", lambda: f"0x{utils.get(val_obj, 'm_unit').unsigned:x}")
    _add("shared", lambda: _rawptr_hex(utils.get(val_obj, "m_shared")))
    _add("inoutBits", lambda: f"0x{utils.get(val_obj, 'm_inoutBits').unsigned:x}")

    def _fmt_params() -> str:
        raw = utils.get(val_obj, "m_paramCounts").unsigned
        num = raw >> 1
        variadic = not (raw & 1)
        return f"{num} (variadic)" if variadic else str(num)

    _add("params", _fmt_params)

    _add(
        "attrs",
        lambda: _decode_flags(
            _read_atomic(utils.get(val_obj, "m_attrs", "m_attrs")),
            _FUNC_ATTRS,
        ),
    )

    def _fmt_prologue() -> str:
        table = utils.get(val_obj, "m_prologueTable")
        first = table.GetChildAtIndex(0)
        if first.IsValid():
            return _rawptr_hex(first)
        return "?"

    _add("prologue[0]", _fmt_prologue)

    if not parts:
        raise FormatError("no fields could be extracted")

    detail = ", ".join(parts)
    headline = f'"{name}" ' if name else ""
    return f"{headline}{{{detail}}}"


@format("^HPHP::Object$", regex=True)
def pp_Object(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        val = utils.get(val_obj, "m_obj")
    except AssertionError as e:
        raise FormatError(str(e))
    s = utils.nameof(val)
    if s is None:
        raise FormatError("nameof returned None for Object")
    return '"' + s + '"'


# ------------------------------------------------------------------------------
# Extensions


@format("^HPHP::Extension$", regex=True)
def pp_Extension(val_obj: lldb.SBValue, _internal_dict) -> str:
    val: lldb.SBValue = utils.deref(val_obj)

    def cstr(v: lldb.SBValue) -> str:
        return utils.read_cstring(v, 256, val.process)

    try:
        name = cstr(utils.deref(utils.get(val, "m_name")))
        version = cstr(utils.deref(utils.get(val, "m_version")))
        oncall = cstr(utils.deref(utils.get(val, "m_oncall")))
    except Exception as e:
        raise FormatError(str(e))
    return f"{name} (version: {version}, oncall: {oncall})"


# ------------------------------------------------------------------------------
# HHBBC Bytecode


@format("^HPHP::HHBBC::Bytecode$", regex=True)
def pp_HhbbcBytecode(val_obj: lldb.SBValue, _internal_dict) -> str:
    try:
        op = utils.get(val_obj, "op").value
        if op is None:
            raise FormatError("op field has no value")
        val = str(utils.get(val_obj, op))
    except AssertionError as e:
        raise FormatError(str(e))
    val = re.sub(r"\(HPHP::HHBBC::bc::.*\) ", "", val)
    return "bc::%s { %s }" % (op, val)


def __lldb_init_module(
    debugger: lldb.SBDebugger,
    top_module: str = "",
) -> None:
    """Register the pretty printers in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object

    Returns:
        None
    """
    for cmd in Formatters:
        debugger.HandleCommand(cmd(top_module))
