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

from __future__ import absolute_import, division, print_function, unicode_literals

from collections import namedtuple, OrderedDict

from thrift.Thrift import TType


__all__ = ["Serializer", "struct_to_dict", "parse_struct_spec"]
StructField = namedtuple("StructField", "id type name type_args default req_type")


def parse_struct_spec(struct):
    """
    Given a thrift struct return a generator of parsed field information

    StructField fields:
        id - the field number
        type - a Thrift.TType
        name - the field name
        type_args - type arguments (ex: Key type Value type for maps)
        default - the default value
        req_type - the field required setting
            (0: Required, 1: Optional, 2: Optional IN, Required OUT)

    :param struct: a thrift struct
    :return: a generator of StructField tuples
    """
    for field in struct.thrift_spec:
        if not field:
            continue
        yield StructField._make(field)


def struct_to_dict(struct, defaults: bool = False):
    """
    Given a Thrift Struct convert it into a dict
    :param struct: a thrift struct
    :param defaults: return default values
    :return: OrderedDict
    """
    adict = OrderedDict()
    union = struct.isUnion()
    if union and struct.field == 0:
        # if struct.field is 0 then it is unset escape
        return adict
    for field in parse_struct_spec(struct):
        if union:
            if field.id == struct.field:
                value = struct.value
            else:
                continue
        else:
            value = getattr(struct, field.name, field.default)
        if value != field.default or defaults:
            if field.type == TType.STRUCT:
                if value is not None:
                    sub_dict = struct_to_dict(value, defaults=defaults)
                    if sub_dict or defaults:  # Do not include empty sub structs
                        adict[field.name] = sub_dict
            elif field.type == TType.LIST:
                sub_list = __list_to_dict(value, field.type_args, defaults=defaults)
                if sub_list or defaults:
                    adict[field.name] = sub_list
            elif field.type == TType.SET:
                sub_set = __set_to_dict(value, field.type_args, defaults=defaults)
                if sub_set or defaults:
                    adict[field.name] = sub_set
            elif field.type == TType.MAP:
                sub_map = __map_to_dict(value, field.type_args, defaults=defaults)
                if sub_map or defaults:
                    adict[field.name] = sub_map
            else:
                adict[field.name] = value
        if union:  # If we got this far then we have the union value
            break
    return adict


def __list_to_dict(alist, type_args, defaults: bool = False):
    """
    Given a python list-like collection, potentially containing Thrift Structs,
    convert it into a dict
    :param alist: a list or set
    :param defaults: return default values
    :return: List
    """
    if not alist:
        return alist

    element_type = type_args[0]
    if element_type == TType.STRUCT:
        return [struct_to_dict(element, defaults=defaults) for element in alist]
    if element_type == TType.LIST:
        return [
            __list_to_dict(element, type_args[1], defaults=defaults)
            for element in alist
        ]
    if element_type == TType.SET:
        return [
            __set_to_dict(element, type_args[1], defaults=defaults) for element in alist
        ]
    else:
        return alist


def __set_to_dict(aset, type_args, defaults: bool = False):
    """
    Given a python set-like collection, potentially containing Thrift Structs
    and recursively parsing the elements
    :param aset: a set
    :param defaults: return default values
    :return: Set
    """
    if not aset:
        return aset

    element_type = type_args[0]
    if element_type == TType.STRUCT:
        return {struct_to_dict(element, defaults=defaults) for element in aset}
    if element_type == TType.LIST:
        return {
            __list_to_dict(element, type_args[1], defaults=defaults) for element in aset
        }
    if element_type == TType.SET:
        return {
            __set_to_dict(element, type_args[1], defaults=defaults) for element in aset
        }
    else:
        return aset


def __map_to_dict(amap, type_args, defaults: bool = False):
    """
    Given a python dictionary, potentially containing Thrift Structs, convert it
    into a dict
    :param amap: a map
    :param defaults: return default values
    :return: Dict
    """
    if not amap:
        return amap

    keys, values = zip(*amap.items())

    keys = __list_to_dict(keys, type_args[:2], defaults=defaults)
    values = __list_to_dict(values, type_args[2:4], defaults=defaults)

    return dict(zip(keys, values))
