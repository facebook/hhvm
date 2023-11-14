# Copyright 2022-present Facebook. All Rights Reserved.

""" Pretty printers for HPHP types """

import lldb
import re
import sizeof
import sys
import traceback
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import idx
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.idx as idx
    import hhvm_lldb.utils as utils


Formatters = []


def format(datatype: str, regex: bool = False, skip_pointers = False, skip_references = False, synthetic_children: bool = False):
    """ Wrapper for pretty printer functions.

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
    def inner(func_or_class):
        extra_options = []
        if regex:
            extra_options.append("-x")
        if skip_pointers:
            extra_options.append("-p")
        if skip_references:
            extra_options.append("-r")
        extra_options = " ".join(extra_options)

        if synthetic_children:
            assert isinstance(func_or_class, type), "Can only use synthetic_children=True in @format decorator on classes"
            Formatters.append(lambda top_module:
                f'type synthetic add {extra_options} '
                f'--python-class {top_module + "." if top_module else ""}pretty.{func_or_class.__name__} "{datatype}"'
            )
            # Modify the top-level summary of this type
            if hasattr(func_or_class, "summary"):
                Formatters.append(lambda top_module:
                    f'type summary add --expand {extra_options} '
                    f'--summary-string "{func_or_class.summary()}" "{datatype}"'
                )
            return func_or_class
        else:
            Formatters.append(lambda top_module:
                f'type summary add {extra_options} '
                f'--python-function {top_module + "." if top_module else ""}pretty.{func_or_class.__name__} "{datatype}"'
            )
            def wrapper(val_obj, internal_dict):
                # When given a nullptr, just print the address, rather than try and probably fail
                # to get its contents in whatever pretty printers would normally be called.
                if utils.is_nullptr(val_obj):
                    return '0x0'

                # When the pretty printer for this value fails for some reason,
                # just show the unformatted version.
                try:
                    return func_or_class(val_obj, internal_dict)
                except Exception:
                    utils.debug_print(f"Failed to pretty print '{val_obj.name}' in {func_or_class.__name__}()")
                    if utils._Debug:
                        traceback.print_exc()
                    return val_obj.value
            return wrapper
    return inner


#------------------------------------------------------------------------------
# NOTE: the functions prefixed with "pp_" all have the following signature:
#
#   def pp_<TypeName>(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
#      """ Pretty print HPHP::<TypeName>
#          Arguments:
#              val_obj: an SBValue wrapping an HPHP::<TypeName>
#              internal_dict: an LLDB support object not to be used
#
#          Returns:
#              A string representing the <TypeName>, or None if there was an error.
#      """

#------------------------------------------------------------------------------
# TypedValues and its subtypes

@format("^HPHP::((Unaligned)?TypedValue|Variant|VarNR)$", regex=True)
def pp_TypedValue(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    m_type = utils.get(val_obj, "m_type")
    m_data = utils.get(val_obj, "m_data")
    return utils.pretty_tv(m_type, m_data)

#------------------------------------------------------------------------------
# Pointers

def pretty_ptr(val: lldb.SBValue) -> typing.Optional[str]:
    utils.debug_print(f"pretty_ptr(val=0x{val.unsigned:x})")

    ptr = utils.rawptr(val)

    if utils.is_nullptr(ptr):
        return None

    inner = utils.deref(ptr)
    inner_type = utils.rawtype(inner.type)

    if inner_type.name == "HPHP::StringData":
        s = utils.string_data_val(inner)
    else:
        s = utils.nameof(inner)
    return '"' + s + '"'


@format("^HPHP::req::ptr<.*>$", regex=True)
def pp_ReqPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


@format("^HPHP::(LowPtr<.*>|detail::LowPtrImpl<.*>)$", regex=True)
def pp_LowPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


#------------------------------------------------------------------------------
# Resource

@format("^HPHP::OptResource$", regex=True)
def pp_Resource(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.rawptr(utils.get(val_obj, "m_res"))
    return utils.pretty_resource_header(val)


#------------------------------------------------------------------------------
# Strings

@format("^HPHP::StringData$", regex=True)
def pp_StringData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    # Note: string_data_val() will dereference a pointer value, if given
    return '"' + utils.string_data_val(val_obj) + '"'


@format("^HPHP::(Static)?String$", regex=True)
def pp_String(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    # Note: SBValue.GetChildMemberWithName(), used by utils.get(),
    # will get the members of both pointers and the pointed-to values themselves
    val = utils.rawptr(utils.get(val_obj, "m_str"))
    return '"' + utils.string_data_val(val) + '"'


@format("^HPHP::StrNR$", regex=True)
def pp_StrNR(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.get(val_obj, "m_px")
    return '"' + utils.string_data_val(val) + '"'

#------------------------------------------------------------------------------
# Optional

@format("^HPHP(::req)?::Optional<.*>$", regex=True)
def pp_Optional(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    """ See:
          * hphp/runtime/base/req-optional.h
          * hphp/util/optional.h
    """
    val = utils.get(val_obj, "m_opt")
    val = val.children[0] if val.children else None
    return str(val)


#------------------------------------------------------------------------------
# Arrays


@format("^HPHP::ArrayData$", regex=True, synthetic_children=True)
class pp_ArrayData:
    """ This conforms to the SyntheticChildrenProvider interface """

    @staticmethod
    def summary():
        # Ideally we'd print the refcount and kind (e.g. Vec/Dict/Keyset),
        # but there's no easy way to do that with the synthetic children API
        # (we have access to a limited set of formatting summary elements),
        # and it doesn't look like we can supply a summary function when
        # supplying a synthetic children provider.
        # return f"ArrayData[{self.m_kind.value}]: {self.m_size} element(s) refcount={self.m_count}"
        return "${svar%#} element(s)"

    def __init__(self, val_obj, _internal_dict):
        # We use this class for both the synthetic children and for the summary.
        # For the summary, we will be given the synthetic lldb.SBValue so we
        # must make sure to get the non-synthetic lldb.SBValue.
        utils.debug_print(f"pp_ArrayData::__init__ with val_obj (load_addr: 0x{val_obj.load_addr:x}, type: {val_obj.type.name})")
        self.val_obj = val_obj.GetNonSyntheticValue()
        self.size = None
        self.func = None
        self.update()

    def num_children(self) -> int:
        if self.size is None:
            utils.debug_print("Unable to determine number of children of ArrayData object, returning 0")
            return 0
        return self.size.unsigned

    def get_child_index(self, name: str) -> int:
        try:
            return int(name.lstrip('[').rstrip(']'))
        except ValueError:
            return -1

    def get_child_at_index(self, index: int) -> lldb.SBValue:
        utils.debug_print(f"pp_ArrayData::get_child_at_index with index {index}")
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        if self.at_func is None:
            print("Invalid array type!", file=sys.stderr)
            return None
        return self.at_func(index)

    def update(self):
        try:
            return self._update()
        except Exception:
            utils.debug_print("Exception while pretty printing ArrayData")
            if utils._Debug:
                traceback.print_exc()
            return False

    def _update(self):
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
        base = specialized_obj.CreateValueFromAddress("tmp", specialized_obj.load_addr + specialized_obj.type.size, char_ptr_type)
        assert base.GetError().Success(), "Couldn't get base address of array"

        if utils.has_array_kind(self.val_obj, 'Vec'):
            self.at_func = lambda ix: idx.vec_at(base, ix)
        elif utils.has_array_kind(self.val_obj, 'Dict'):
            self.at_func = lambda ix: idx.dict_at(base, ix)
        elif utils.has_array_kind(self.val_obj, 'Keyset'):
            self.at_func = lambda ix: idx.keyset_at(base, ix)

        # Return false to make sure we always update this object every time we
        # stop. If we return True, then the value will never update again.
        return False


@format("^HPHP::Array$", regex=True, synthetic_children=True)
class pp_Array(pp_ArrayData):
    def __init__(self, val_obj, _internal_dict):
        if val_obj.GetError().Fail():
            utils.debug_print(f"Invalid array. Error: {val_obj.GetError().GetCString()}")
            return

        val = utils.deref(utils.get(val_obj, "m_arr"))
        super().__init__(val, _internal_dict)


#------------------------------------------------------------------------------
# Classes, Functions, and Objects

@format("^HPHP::(Class|LazyClassData|Func|ObjectData)$", regex=True)
def pp_NamedValue(val_obj: lldb.SBValue, _internal_dict) -> str:
    return '"' + utils.nameof(val_obj) + '"'


@format("^HPHP::Object$", regex=True)
def pp_Object(val_obj: lldb.SBValue, _internal_dict) -> str:
    val = utils.get(val_obj, "m_obj")
    return '"' + utils.nameof(val) + '"'


#------------------------------------------------------------------------------
# Extensions

@format("^HPHP::Extension$", regex=True)
def pp_Extension(val_obj: lldb.SBValue, _internal_dict) -> str:
    val = utils.deref(val_obj)
    def cstr(v):
        return utils.read_cstring(v, 256, val.process)
    name = cstr(utils.deref(utils.get(val, "m_name")))
    version = cstr(utils.deref(utils.get(val, "m_version")))
    oncall = cstr(utils.deref(utils.get(val, "m_oncall")))
    return f"{name} (version: {version}, oncall: {oncall})"


#------------------------------------------------------------------------------
# HHBBC Bytecode

@format("^HPHP::HHBBC::Bytecode$", regex=True)
def pp_HhbbcBytecode(val_obj: lldb.SBValue, _internal_dict) -> str:
    op = utils.get(val_obj, "op").value
    val = str(utils.get(val_obj, op))
    val = re.sub(r"\(HPHP::HHBBC::bc::.*\) ", "", val)
    return 'bc::%s { %s }' % (op, val)


def __lldb_init_module(debugger: lldb.SBDebugger, _internal_dict, top_module=""):
    """ Register the pretty printers in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    for cmd in Formatters:
        debugger.HandleCommand(cmd(top_module))
