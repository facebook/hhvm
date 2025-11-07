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

from collections.abc import Iterable, Mapping, Set as abcSet, Sequence
from enum import IntEnum
from math import isclose

from thrift.py3.types import Struct as py3_Struct
from thrift.py3.exceptions import GeneratedError as py3_GeneratedError
from thrift.python.exceptions import GeneratedError
from thrift.python.types import Enum, Struct, Union, typeinfo_float, Float32TypeInfo
from thrift.python.types cimport List, Set, Map
from thrift.python.mutable_types import MutableStruct, MutableUnion
from thrift.python.mutable_exceptions cimport MutableGeneratedError

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

cdef inline _is_float32_enforced():
    return isinstance(typeinfo_float, Float32TypeInfo)

def round_thrift_float32_if_rollout(val, convert_int=False):
    if _is_float32_enforced():
        return round_thrift_to_float32(val, convert_int)

    return val


cdef inline assert_equal_type(unittest, result, expected, str field_context):
    unittest.assertEqual(type(result), type(expected), field_context + ".__name__")

cdef inline format_type_context(result, str field_context):
    cdef str prefix = field_context + "->" if field_context else ""
    return f"{prefix}{type(result).__name__}"

def assert_thrift_almost_equal(unittest, result, expected, str field_context=None, **almost_equal_kwargs):
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
                NOTE: if `rel_tol` kwarg passed, will use `math.isclose` with `rel_tol` kwarg

        Return: None

        Limitations:
            - py-deprecated currently not supported
            - float keys for sets and maps--strongly discouraged--require exact equality
            - raises AssertionError on the first inequality rather than reporting all

    """
    if isinstance(result, float) or isinstance(expected, float):
        rel_tol = almost_equal_kwargs.get('rel_tol', None)
        if rel_tol:
            unittest.assertTrue(
                isclose(
                    result,
                    expected,
                    rel_tol=rel_tol
                ),
                msg=field_context + "; result {result} is not close to expected {expected} with rel_tol {rel_tol}"
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

    if isinstance(result, (Struct, GeneratedError, MutableStruct, MutableGeneratedError)):
        type_context = format_type_context(result, field_context) 
        assert_equal_type(unittest, result, expected, type_context)
        for fld_name, fld_val in result:
            expected_val = getattr(expected, fld_name)
            assert_thrift_almost_equal(
                unittest,
                fld_val,
                expected_val,
                field_context=f"{type_context}.{fld_name}",
                **almost_equal_kwargs
            )

    elif isinstance(result, (Union, MutableUnion)):
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
            **almost_equal_kwargs
        )

    elif isinstance(result, Mapping):
        if not isinstance(expected, Mapping):
            unittest.fail(
                f"result {type(result)} is a Mapping, but {type(expected)} is not: {field_context}",
            )
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
                **almost_equal_kwargs
            )

    elif isinstance(result, abcSet):
        if not isinstance(expected, abcSet):
            unittest.fail(
                f"result {type(result)} is a Set, but {type(expected)} is not: {field_context}",
            )
        type_context = format_type_context(result, field_context)
        missing_keys = result ^ expected
        if missing_keys:
            unittest.fail(
                "result and expected have non-empty symmetric difference: "
                f"{missing_keys}; {type_context}"
            )

    elif isinstance(result, Sequence):
        if not isinstance(expected, Sequence):
            unittest.fail(
                f"result {type(result)} is a Sequence, but {type(expected)} is not: {field_context}",
            )
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
    else:
        # all non-float scalar types
        unittest.assertEqual(result, expected, msg=field_context)
    
