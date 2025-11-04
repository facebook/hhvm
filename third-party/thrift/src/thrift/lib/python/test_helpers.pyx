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

from collections.abc import Iterable, Mapping
from enum import IntEnum

from thrift.python.types import Enum, Struct, Union
from thrift.python.types cimport List, Set, Map
from thrift.python.mutable_types import MutableStruct, MutableUnion

###
#    This module contains helper functions suitable for migrating customer tests
###

def round_thrift_to_float32(val, convert_int=False):
    """
        A testing helper that rounds scalars, containers,
        and thrift types (structs, unions, immutable thrift containers)
        to float32. Returns non-numeric scalars unchanged. Mutable thrift
        containers are not yet implemented.

        By default will not convert `int` subclass unless `convert_int` is
        set to True. Does not support thrift Structs / Unions with container
        fields if `convert_int` is set to True.
    """
#
# This is meant as a convenience function for testing and is not perf optimized
    cdef type val_type = type(val)
    if issubclass(val_type, float):
        return <float> val
    # must check str, bytes before Iterable because they are iterable
    if issubclass(val_type, (str, bytes)):
        return val
    if convert_int and issubclass(val_type, int):
        return val if issubclass(val_type, (Enum, IntEnum, bool)) else <float>val

    if val_type is List:
        val_info = (<List>val)._fbthrift_val_info
        return List(val_info, (round_thrift_to_float32(x) for x in val))
    if val_type is Set:
        val_info = (<Set>val)._fbthrift_val_info
        return Set(val_info, (round_thrift_to_float32(x) for x in val))
    if val_type is Map:
        key_info = (<Map>val)._fbthrift_key_info
        val_info = (<Map>val)._fbthrift_val_info
        return Map(
            key_info,
            val_info, 
            {round_thrift_to_float32(k): round_thrift_to_float32(v) for k, v in val.items()}
        )

    if issubclass(val_type, Mapping):
        return val_type(
            {round_thrift_to_float32(k): round_thrift_to_float32(v) for k, v in val.items()}
        )
    if issubclass(val_type, (Struct, MutableStruct)):
        return val_type(
            **{fld_name: round_thrift_to_float32(fld_val) for fld_name, fld_val in val}
        )
    if issubclass(val_type, Iterable):
        return val_type((round_thrift_to_float32(x) for x in val))


    if issubclass(val_type, (Union, MutableUnion)):
        if val.fbthrift_current_value is None:
            return val
        else:
            fld_name = val.fbthrift_current_field.name
            fld_val = val.fbthrift_current_value
            return val_type(**{fld_name: round_thrift_to_float32(fld_val)})



    return val
    
