#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

import inspect
import sys
from types import ModuleType
from typing import Any, cast, Dict, List, Optional, Tuple, Type

from thrift.Thrift import TType

ExtraTypeSpec = Any
FieldSpec = Tuple[
    int,  # field id
    TType,
    str,  # field_name
    ExtraTypeSpec,
    Any,  # default value
    int,  # 0=required, 1=optional, 2=default
]

_args_suffix = "_args"
_result_suffix = "_result"


class NoSuchFunctionError(ValueError):
    pass


class Function:
    """A class describing the argument types and return type of a thrift function."""

    def __init__(
        self,
        name: str,
        module: ModuleType,
        arg_specs: List[FieldSpec],
        result_spec: Optional[Tuple[FieldSpec]],
    ) -> None:
        self.name = name
        self.module = module
        self.arg_specs = arg_specs
        self.result_spec = result_spec

    @property
    def service_name(self) -> str:
        return self.module.__name__

    def __str__(self) -> str:
        arg_info = []
        for info in self.arg_specs:
            if info is None:
                # The thrift_spec format is pretty dumb, and attempts to make the tuple
                # indices match up with the field ID numbers in most cases (when the
                # field IDs are positive).  It fills gaps in the field IDs with None
                # elements.
                continue
            (
                _field_id,
                thrift_type,  # enum describing thrift base type
                arg_name,
                extra_type_spec,
                _default_value,
                _required,
            ) = info
            type_name = get_type_name(thrift_type, extra_type_spec)
            arg_info.append(f"{arg_name}: {type_name}")

        args_str = ", ".join(arg_info)
        if self.result_spec is None:
            # This is a oneway function.  The server sends no response at all (and
            # therefore cannot even indicate errors).  There is no confirmation that the
            # request was even received or processed.
            return f"{self.name}({args_str}) [oneway]"
        else:
            result_str = _get_result_str(self.result_spec)
            return f"{self.name}({args_str}) -> {result_str}"


def get_service_module_hierarchy(service_module: ModuleType) -> List[ModuleType]:
    """Given a thrift service module, return a list containing it plus all
    of the modules for its parent services.
    """
    iface_class = getattr(service_module, "Iface", None)
    if iface_class is None:
        raise TypeError(
            f"{service_module!r} does not look like a thrift service module"
        )

    result = []
    for cls in iface_class.__mro__:
        module = sys.modules[cls.__module__]
        # Check to make sure this actually looks like a thrift service module.
        # The MRO hierarchy will include the base "object" class which is not from a
        # thrift module.
        if hasattr(module, "Iface"):
            result.append(module)

    return result


def list_all_functions(service_module: ModuleType) -> Dict[str, Function]:
    """Given a thrift service module, return information about the functions exposed by
    this service, including all of its inherited functions.
    """
    modules = get_service_module_hierarchy(service_module)
    functions: Dict[str, Function] = {}
    for module in reversed(modules):
        functions.update(list_service_functions(module))

    return functions


def list_service_functions(service_module: ModuleType) -> Dict[str, Function]:
    """Given a thrift service module, return information about the functions it exposes.
    This returns only the functions directly defined by this service, excluding
    functions inherited from parent service interfaces.  Use list_all_functions() if you
    also want inherited functions to be returned.
    """
    functions = {}
    for name in dir(service_module):
        if not name.endswith(_args_suffix):
            continue
        fn_name = name[: -len(_args_suffix)]
        functions[fn_name] = get_function_info(service_module, fn_name)

    return functions


def get_function_info(
    service_module: ModuleType, fn_name: str, search_parent_interfaces: bool = True
) -> Function:
    """Return a Function object describing the interface of the specified function
    in the supplied thrift service module.
    """
    if search_parent_interfaces:
        modules = get_service_module_hierarchy(service_module)
    else:
        modules = [service_module]

    for module in modules:
        fn_info = _get_function_info(module, fn_name)
        if fn_info is not None:
            return fn_info

    raise NoSuchFunctionError(f"no function named {fn_name}")


def _get_function_info(service_module: ModuleType, fn_name: str) -> Optional[Function]:
    args_type = getattr(service_module, fn_name + _args_suffix, None)
    if args_type is None:
        # This function is not defined by this module
        return None

    ordered_arg_specs = _get_function_arg_specs(service_module, fn_name)
    result_type = getattr(service_module, fn_name + _result_suffix, None)

    result_spec = (
        None if result_type is None else cast(Tuple[FieldSpec], result_type.thrift_spec)
    )
    return Function(fn_name, service_module, ordered_arg_specs, result_spec)


def _get_function_arg_specs(
    service_module: ModuleType, fn_name: str
) -> List[FieldSpec]:
    args_type = getattr(service_module, fn_name + _args_suffix)

    # Re-order the argument types to actually match the python function argument
    # ordering.
    #
    # The entries in the thrift spec will be ordered by thrift field ID numbers (and
    # it actually tries to match up the tuple indices to the thrift field IDs, which
    # results in huge tuples if you happen to use a large ID value).
    #
    # The field ID numbers do not necessarily match the source code order of the
    # arguments.
    iface_class = getattr(service_module, "Iface")
    py_fn = getattr(iface_class, fn_name)
    sig = inspect.signature(py_fn)

    # First build a dictionary of the thrift specs by name
    arg_specs = {}
    for spec in args_type.thrift_spec:
        if spec is None:
            continue
        arg_name = spec[2]
        arg_specs[arg_name] = spec

    # Now put them in the correct order
    ordered_arg_specs = []
    for param in list(sig.parameters)[1:]:  # skip the initial "self" parameter
        arg_spec = arg_specs.pop(param)
        ordered_arg_specs.append(arg_spec)

    assert not arg_specs, (
        f"did not consume all thrift arguments for {fn_name}: {arg_specs}"
    )

    return ordered_arg_specs


def get_type_name(thrift_type: TType, extra_type_spec: Any) -> str:
    """Convert information about a thrift type into a human-readable string."""
    if thrift_type == TType.STRING:
        is_binary = not cast(bool, extra_type_spec)
        if is_binary:
            return "binary"
        else:
            return "string"
    elif thrift_type == TType.BOOL:
        return "bool"
    elif thrift_type == TType.BYTE:
        return "byte"
    elif thrift_type == TType.DOUBLE:
        return "double"
    elif thrift_type == TType.I16:
        return "i16"
    elif thrift_type == TType.I32:
        if extra_type_spec is None:
            return "i32"
        else:
            # This is an enum, and the extra_type_spec contains the enum type
            python_type = cast(Type, extra_type_spec)
            return f"{python_type.__module__}.{python_type.__qualname__}"
    elif thrift_type == TType.I64:
        return "i64"
    elif thrift_type == TType.DOUBLE:
        return "double"
    elif thrift_type == TType.FLOAT:
        return "float"
    elif thrift_type == TType.STRUCT:
        python_type, type_spec, is_union, *_ = extra_type_spec
        return f"{python_type.__module__}.{python_type.__qualname__}"
    elif thrift_type == TType.MAP:
        (
            key_thrift_type,
            key_extra_spec,
            value_thrift_type,
            value_extra_spec,
        ) = extra_type_spec
        key_type_name = get_type_name(key_thrift_type, key_extra_spec)
        value_type_name = get_type_name(value_thrift_type, value_extra_spec)
        return f"map<{key_type_name}, {value_type_name}>"
    elif thrift_type == TType.SET:
        (elem_thrift_type, elem_extra_spec) = extra_type_spec
        elem_type_name = get_type_name(elem_thrift_type, elem_extra_spec)
        return f"set<{elem_type_name}>"
    elif thrift_type == TType.LIST:
        (elem_thrift_type, elem_extra_spec) = extra_type_spec
        elem_type_name = get_type_name(elem_thrift_type, elem_extra_spec)
        return f"list<{elem_type_name}>"
    else:
        # The thrift compiler currently does not emit any other
        # types for python.  (C++ also has a "STREAM" type but this is not
        # currently supported for python.)
        return "unsupported thrift type"


def _get_result_str(result_spec: Tuple[FieldSpec]) -> str:
    # The entry with field ID 0 is the success result.  If the function returns void
    # this will not be present.
    # All other entries are possible exception types that may be thrown.
    for info in result_spec:
        if info is None:
            continue
        (
            field_id,
            thrift_type,  # enum describing thrift base type
            _arg_name,
            extra_type_spec,
            _default_value,
            _required,
        ) = info
        if field_id == 0:
            return get_type_name(thrift_type, extra_type_spec)
    return "void"
