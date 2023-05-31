# Copyright 2022-present Facebook. All Rights Reserved.

""" Pretty printers for HPHP types """

import lldb
import sys
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

    Arguments:
        datatype: the name of the data type being formatted
        regex: whether the datatype in string is a regex

    Returns:
        The original function.
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
        else:
            Formatters.append(lambda top_module:
                f'type summary add {extra_options} '
                f'--python-function {top_module + "." if top_module else ""}pretty.{func_or_class.__name__} "{datatype}"'
            )
        return func_or_class
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
    ptr = utils.rawptr(val)
    if utils.is_nullptr(ptr):
        return None

    inner = utils.deref(ptr)
    inner_type = utils.rawtype(inner.type)

    if inner_type.name == "HPHP::StringData":
        return utils.string_data_val(inner)
    return utils.nameof(inner)


@format("^HPHP::req::ptr<.*>$", regex=True)
def pp_ReqPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


@format("^HPHP::(LowPtr<.*>|detail::LowPtrImpl<.*>)$", regex=True)
def pp_LowPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


#------------------------------------------------------------------------------
# Resource

@format("^HPHP::Resource$", regex=True)
def pp_Resource(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.rawptr(utils.get(val_obj, "m_res"))
    return utils.pretty_resource_header(val)


#------------------------------------------------------------------------------
# Strings

@format("^HPHP::StringData$", regex=True)
def pp_StringData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    # Note: string_data_val() will dereference a pointer value, if given
    return utils.string_data_val(val_obj)


@format("^HPHP::(Static)?String$", regex=True)
def pp_String(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    # Note: SBValue.GetChildMemberWithName(), used by utils.get(),
    # will get the members of both pointers and the pointed-to values themselves
    val = utils.rawptr(utils.get(val_obj, "m_str"))
    return utils.string_data_val(val)


@format("^HPHP::StrNR$", regex=True)
def pp_StrNR(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.get(val_obj, "m_px")
    return utils.string_data_val(val)

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
        self.val_obj = val_obj.GetNonSyntheticValue()
        self.update()

    def num_children(self) -> int:
        if self._is('Vec'):
            m_size = utils.get(self.val_obj, "m_size")
            return m_size.unsigned
        elif self._is('Dict') or self._is('Keyset'):
            m_used = utils.get(self.val_obj.children[1].children[0], "m_used")
            return m_used.unsigned
        else:
            print("Invalid array type!", file=sys.stderr)
            return 0

    def get_child_index(self, name: str) -> int:
        try:
            return int(name.lstrip('[').rstrip(']'))
        except ValueError:
            return -1

    def get_child_at_index(self, index: int) -> lldb.SBValue:
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        char_ptr_type = utils.Type("char", self.val_obj.target).GetPointerType()
        # Note: use CreateChildAtOffset was working when pretty printing the dereferenced m_arr
        # from an Array struct, but not when pretty printing an ArrayData itself.
        # base = self.val_obj.CreateChildAtOffset("tmp", self.val_obj.type.size, char_ptr_type)
        base = self.val_obj.CreateValueFromAddress("tmp", self.val_obj.load_addr + self.val_obj.type.size, char_ptr_type)
        assert base.IsValid(), "Couldn't get base address of array"
        if self._is('Vec'):
            return idx.vec_at(base, index)
        elif self._is('Dict'):
            return idx.dict_at(base, index)
        elif self._is('Keyset'):
            return idx.keyset_at(base, index)
        else:
            print("Invalid array type!", file=sys.stderr)
            return None

    def update(self):
        # Doing all of this logic in here, rather than __init__(), because the API
        # says we should be re-updating internal state as much as possible (since the
        # state of variables can change since the last invocation).
        heap_obj = self.val_obj.children[0].children[0]  # HPHP::HeapObject
        self.m_kind = utils.get(heap_obj, "m_kind")
        self.m_count = utils.get(heap_obj, "m_count").unsigned
        if self._is('Vec'):
            pass
        elif self._is('Dict'):
            self.val_obj = self.val_obj.Cast(utils.Type("HPHP::VanillaDict", self.val_obj.target))
        elif self._is('Keyset'):
            self.val_obj = self.val_obj.Cast(utils.Type("HPHP::VanillaKeyset", self.val_obj.target))
        elif self._is('BespokeVec') or self._is('BespokeDict') or self._is('BespokeKeyset'):
            print(f"Unsupported bespoke array type ('{self.m_kind}')! Run `expression -R -- {self.val_obj.path}` to see its raw form", file=sys.stderr)
        else:
            print(f"Invalid array type ('{self.m_kind}')! Run `expression -R -- {self.val_obj.path}` to see its raw form", file=sys.stderr)

        # Return false to make sure we always update this object every time we
        # stop. If we return True, then the value will never update again.
        return False

    def _is(self, member: str) -> bool:
        kind = utils.Enum("HPHP::ArrayData::ArrayKind", "k" + member + "Kind", self.val_obj.target).unsigned
        return self.m_kind.unsigned == kind


@format("^HPHP::Array$", regex=True, synthetic_children=True)
class pp_Array(pp_ArrayData):
    def __init__(self, val_obj, _internal_dict):
        val = utils.deref(utils.get(val_obj, "m_arr"))
        super().__init__(val, _internal_dict)


#------------------------------------------------------------------------------
# Objects

@format("^HPHP::Object$", regex=True)
def pp_Object(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.get(val_obj, "m_obj")
    return utils.nameof(val)


#------------------------------------------------------------------------------
# Extensions

@format("^HPHP::Extension$", regex=True)
def pp_Extension(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.deref(val_obj)
    def cstr(v):
        return utils.read_cstring(v, 256, val.process)
    name = cstr(utils.deref(utils.get(val, "m_name")))
    version = cstr(utils.deref(utils.get(val, "m_version")))
    oncall = cstr(utils.deref(utils.get(val, "m_oncall")))
    return f"{name} (version: {version}, oncall: {oncall})"


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
