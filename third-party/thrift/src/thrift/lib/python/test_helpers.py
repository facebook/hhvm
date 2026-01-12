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

# pyre-strict

from collections.abc import Iterable, Mapping, Sequence, Set as abcSet
from dataclasses import asdict as dataclass_asdict, is_dataclass
from enum import IntEnum
from math import isclose
from typing import cast as typing_cast, Protocol, TypeVar

from thrift.py3.exceptions import GeneratedError as py3_GeneratedError
from thrift.py3.types import Struct as py3_Struct
from thrift.python.exceptions import GeneratedError
from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.mutable_types import MutableStruct, MutableUnion

# pyre-ignore[21]: no pyre annotations for internal types
from thrift.python.types import (
    _fbthrift_internal_DO_NOT_USE_get_List_typeinfo,
    _fbthrift_internal_DO_NOT_USE_get_Map_typeinfo,
    _fbthrift_internal_DO_NOT_USE_get_Set_typeinfo,
)

# putting this here to prevent formatter from combining thrift.python.types imports
T = TypeVar("T")

# types with pyre typing
from thrift.python.types import (
    Enum,
    Float32TypeInfo,
    List,
    Map,
    round_float32,
    Set,
    Struct,
    typeinfo_float,
    Union,
)

###
#    This module contains helper functions suitable for migrating customer tests
###


def round_thrift_to_float32(val: T, convert_int: bool = False) -> T:
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
    if isinstance(val, float):
        return round_float32(val)
    # must check str, bytes before Iterable because they are iterable
    if isinstance(val, (str, bytes)):
        return val
    if convert_int and isinstance(val, int):
        return val if isinstance(val, (Enum, IntEnum, bool)) else round_float32(val)

    if isinstance(val, List):
        val_info = _fbthrift_internal_DO_NOT_USE_get_List_typeinfo(val)
        return List(val_info, (round_thrift_to_float32(x) for x in val))
    if isinstance(val, Set):
        val_info = _fbthrift_internal_DO_NOT_USE_get_Set_typeinfo(val)
        return Set(val_info, (round_thrift_to_float32(x) for x in val))
    if isinstance(val, Map):
        key_info, val_info = _fbthrift_internal_DO_NOT_USE_get_Map_typeinfo(val)
        return Map(
            key_info,
            val_info,
            {
                round_thrift_to_float32(k): round_thrift_to_float32(v)
                for k, v in val.items()
            },
        )

    if isinstance(val, Mapping):
        return type(val)(
            {
                round_thrift_to_float32(k): round_thrift_to_float32(v)
                for k, v in val.items()
            }
        )
    if isinstance(val, (Struct, MutableStruct)):
        return type(val)(
            **{fld_name: round_thrift_to_float32(fld_val) for fld_name, fld_val in val}
        )
    if isinstance(val, Iterable):
        return type(val)((round_thrift_to_float32(x) for x in val))

    if isinstance(val, (Union, MutableUnion)):
        if val.fbthrift_current_value is None:
            return val
        else:
            fld_name = val.fbthrift_current_field.name
            fld_val = val.fbthrift_current_value
            return type(val)(**{fld_name: round_thrift_to_float32(fld_val)})

    return val


def _internal_is_float32_enforced() -> bool:
    return isinstance(typeinfo_float, Float32TypeInfo)


def round_thrift_float32_if_rollout(val: T, convert_int: bool = False) -> T:
    if _internal_is_float32_enforced():
        return round_thrift_to_float32(val, convert_int)

    return val


class Unittest(Protocol):
    def assertEqual(self, first: T, second: T, msg: str | None = None) -> None: ...
    def assertTrue(self, first: object, msg: str | None = None) -> None: ...
    def assertAlmostEqual(
        self, first: T, second: T, msg: str | None = None, **kwargs: object
    ) -> None: ...
    def fail(self, msg: str | None = None) -> None: ...


def assert_equal_type(
    unittest: Unittest, result: T, expected: T, field_context: str
) -> None:
    unittest.assertEqual(type(result), type(expected), field_context + ".__name__")


def format_type_context(result: T, field_context: str | None) -> str:
    prefix = field_context + "->" if field_context else ""
    return f"{prefix}{type(result).__name__}"


def _assert_struct_almost_equal(
    unittest: Unittest,
    result: Struct | GeneratedError | MutableStruct | MutableGeneratedError,
    expected: Struct | GeneratedError | MutableStruct | MutableGeneratedError,
    field_context: str,
    **almost_equal_kwargs: object,
) -> None:
    type_context = format_type_context(result, field_context)
    assert_equal_type(unittest, result, expected, type_context)
    for fld_name, fld_val in result:
        expected_val = getattr(expected, fld_name)
        assert_thrift_almost_equal(
            unittest,
            fld_val,
            expected_val,
            field_context=f"{type_context}.{fld_name}",
            **almost_equal_kwargs,
        )


def _assert_union_almost_equal(
    unittest: Unittest,
    result: Union | MutableUnion,
    expected: Union | MutableUnion,
    field_context: str,
    **almost_equal_kwargs: object,
) -> None:
    type_context = format_type_context(result, field_context)
    assert_equal_type(unittest, result, expected, type_context)
    unittest.assertEqual(
        result.fbthrift_current_field,
        expected.fbthrift_current_field,
        msg=type_context + ".fbthrift_current_field",
    )
    assert_thrift_almost_equal(
        unittest,
        result.fbthrift_current_value,
        expected.fbthrift_current_value,
        field_context=f"{type_context}.{result.fbthrift_current_field.name}",
        **almost_equal_kwargs,
    )


def _assert_mapping_almost_equal(
    unittest: Unittest,
    result: Mapping,
    expected: object,
    field_context: str,
    **almost_equal_kwargs: object,
) -> None:
    if not isinstance(expected, Mapping):
        unittest.fail(
            f"result {type(result)} is a Mapping, but {type(expected)} is not: {field_context}",
        )
    expected = typing_cast(Mapping, expected)
    type_context = format_type_context(result, field_context)
    missing_keys = set(result.keys()) ^ set(expected.keys())
    if missing_keys:
        unittest.fail(
            "result and expected have non-empty symmetric difference between "
            f"key sets: {missing_keys}; {type_context}"
        )
    for key, value in result.items():
        assert_thrift_almost_equal(
            unittest,
            value,
            expected[key],
            field_context=f"{type_context}[{key}]",
            **almost_equal_kwargs,
        )


def _assert_set_almost_equal(
    unittest: Unittest,
    result: abcSet,
    expected: object,
    field_context: str,
    **almost_equal_kwargs: object,
) -> None:
    if not isinstance(expected, abcSet):
        unittest.fail(
            f"result {type(result)} is a Set, but {type(expected)} is not: {field_context}",
        )
    type_context = format_type_context(result, field_context)
    missing_keys = result ^ typing_cast(abcSet, expected)
    if missing_keys:
        unittest.fail(
            "result and expected have non-empty symmetric difference: "
            f"{missing_keys}; {type_context}"
        )


def _assert_sequence_almost_equal(
    unittest: Unittest,
    result: Sequence,
    expected: object,
    field_context: str,
    **almost_equal_kwargs: object,
) -> None:
    if not isinstance(expected, Sequence):
        unittest.fail(
            f"result {type(result)} is a Sequence, but {type(expected)} is not: {field_context}",
        )
    expected = typing_cast(Sequence, expected)
    type_context = format_type_context(result, field_context)
    unittest.assertEqual(len(result), len(expected), msg=type_context + ".__len__")
    for i, value in enumerate(result):
        assert_thrift_almost_equal(
            unittest,
            value,
            expected[i],
            field_context=f"{type_context}[{i}]",
            **almost_equal_kwargs,
        )


def _assert_dataclass_almost_equal(
    unittest: Unittest,
    result: object,
    expected: object,
    field_context: str | None,
    **almost_equal_kwargs: object,
) -> None:
    if not isinstance(expected, type(result)):
        unittest.fail(
            f"result is a dataclass {type(result)}, but {type(expected)} is not: {field_context}",
        )
    type_context = format_type_context(result, field_context)
    assert_thrift_almost_equal(
        unittest,
        dataclass_asdict(result),
        dataclass_asdict(expected),
        field_context=f"{type_context}.asdict",
        **almost_equal_kwargs,
    )


def assert_thrift_almost_equal(
    unittest: object,
    result: T,
    expected: T,
    field_context: str | None = None,
    **almost_equal_kwargs: object,
) -> None:
    """
    A testing helper that asserts that two thrift types (structs, unions, immutable thrift containers),
    or containers containing thrift types, are equal.

    Relative to standard unittest, this function has two advantages:
        - includes context path to first inequality
        - uses unittest.assertAlmostEqual for `float` and `double` fields


    Params:
        - unittest: implements assertEqual, assertAlmostEqual, and fail methods
        - result, expected: objects to compare
        - field_context: a context string to indicate where the discrepancy occurred
        - **almost_equal_kwargs: kwargs passed to unittest.assertAlmostEqual
            - `places`, `delta`: absolute tolerance kwargs passed to assertAlmostEqual
            - `rel_tol` kwarg passed, will use `math.isclose` with `rel_tol` kwarg

    Return: None

    Limitations:
        - py-deprecated currently not supported
        - float keys for sets and maps--strongly discouraged--require exact equality
        - raises AssertionError on the first inequality rather than reporting all

    """
    # don't force end-user to inherit from Unittest Protocol
    unittest = typing_cast(Unittest, unittest)
    if isinstance(result, float) or isinstance(expected, float):
        rel_tol = typing_cast(float | None, almost_equal_kwargs.get("rel_tol", None))
        if rel_tol:
            unittest.assertTrue(
                isclose(
                    typing_cast(float, result),
                    typing_cast(float, expected),
                    rel_tol=rel_tol,
                ),
                msg=(field_context or "float")
                + "; result {result} is not close to expected {expected} with rel_tol {rel_tol}",
            )
        else:
            unittest.assertAlmostEqual(
                result,
                expected,
                msg=field_context,
                **almost_equal_kwargs,
            )
        return
    # fast path for common scalar types
    if isinstance(result, (Enum, int, str, bytes)):
        unittest.assertEqual(result, expected, msg=field_context)
        return

    # convert py3 types to thrift-python. Remember, in thrift-py3, Struct is base class of Union
    if isinstance(result, (py3_Struct, py3_GeneratedError)):
        result = result._to_python()
    if isinstance(expected, (py3_Struct, py3_GeneratedError)):
        expected = expected._to_python()

    if isinstance(
        result, (Struct, GeneratedError, MutableStruct, MutableGeneratedError)
    ):
        _assert_struct_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    elif isinstance(result, (Union, MutableUnion)):
        _assert_union_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    elif isinstance(result, Mapping):
        _assert_mapping_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    elif isinstance(result, abcSet):
        _assert_set_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    elif isinstance(result, Sequence):
        _assert_sequence_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    elif is_dataclass(result):
        _assert_dataclass_almost_equal(
            unittest, result, expected, field_context, **almost_equal_kwargs
        )

    else:
        # all non-float scalar types
        unittest.assertEqual(
            result,
            expected,
            msg=(field_context or "")
            + f": {type(result).__module__}.{type(result).__name__}",
        )
