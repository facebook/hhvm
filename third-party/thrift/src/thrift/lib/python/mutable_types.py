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

import dataclasses
import typing


class MutableStructOrUnion:
    pass


class MutableStruct(MutableStructOrUnion):
    pass


def _gen_subclass_forbidden(thrift_type_name: str):
    """
    Returns a callable suitable for usage as a '__init_subclass__' classmethod,
    which always raises a `TypeError` (effectively disabling inheritance).

    Args:
        thrift_type_name: the (human-readable) name of the Thrift type (struct,
            union, etc.) for which the returned lambda will prevent
            inheritance. Used for error messaging.
    """

    def _subclass_forbidden(cls):
        raise TypeError(
            "Inheriting from thrift-python data types is forbidden: "
            f"'{cls.__name__}' cannot inherit from '{thrift_type_name}'."
        )

    return _subclass_forbidden


class MutableStructMeta(type):
    """Metaclass for all generated (mutable) thrift-python Struct types."""

    def __new__(metacls, cls_name: str, bases: tuple, dct: dict):
        """
        Returns a new mutable Thrift Struct class with the given name and
        members.

        Args:
            cls_name: Name of the Thrift Struct, as specified in the Thrift IDL.

            bases: unused, MUST be empty.

            dct: class members, including the SPEC for this class under
                the key '_fbthrift_SPEC'.
        """
        if bases:
            raise ValueError(
                f"New mutable thrift-python struct class ('{cls_name}') must "
                "not have base types, but had: {bases}"
            )

        # Set[Tuple (field spec)]. See `StructInfo` class docstring for the
        # contents of the field spec tuples.
        fields = dct.pop("_fbthrift_SPEC")

        dataclass_fields = []
        for field_spec in fields:
            (
                id,
                qualifier,
                name,
                type_info,
                default_value,
                adapter_info,
                is_primitive,
            ) = field_spec

            if adapter_info is not None:
                raise NotImplementedError(
                    "MutableStructMeta: adapted fields are not yet supported "
                    f"for mutable thrift-python struct (struct={cls_name}, "
                    f"field_name={name})."
                )

            field_type = typing.Any
            dataclass_field = dataclasses.field(
                default=None, metadata={"__fbthrift_field_spec": field_spec}
            )
            dataclass_fields.append((name, field_type, dataclass_field))

        dct["__init_subclass__"] = classmethod(_gen_subclass_forbidden(cls_name))
        return dataclasses.make_dataclass(
            cls_name,
            dataclass_fields,
            bases=(MutableStruct,),
            namespace=dct,
            match_args=False,
            kw_only=True,
            slots=True,
        )
