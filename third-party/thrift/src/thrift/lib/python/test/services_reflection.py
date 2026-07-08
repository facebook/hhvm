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

# pyre-strict

import unittest
from typing import Any

from parameterized import parameterized
from test_thrift.thrift_clients import TestingService, TestingServiceChild
from test_thrift.thrift_services import (
    TestingServiceChildInterface,
    TestingServiceInterface,
)
from test_thrift.thrift_types import (
    Color,
    easy,
    HardError,
    I32List,
    SetI32,
    SimpleError,
)
from thrift.python.bidi_service.thrift_services import TestBidiServiceInterface
from thrift.python.bidi_service.thrift_types import (
    MethodException,
    SinkChunk,
    SinkException,
    StreamChunk,
    StreamException,
)
from thrift.python.reflection import inspect, inspectable
from thrift.python.reflection.constants_reflection import ThriftType
from thrift.python.reflection.services_reflection import (
    ArgumentSpec,
    ExceptionSpec,
    FunctionSpec,
    ServiceSpec,
    SinkSpec,
    StreamSpec,
)
from thrift.python.reflection.types_reflection import ListSpec, MapSpec, SetSpec
from thrift.python.reflection_enums import FunctionQualifier
from thrift.python.sink_service.thrift_services import TestSinkServiceInterface
from thrift.python.types import Map as _fbthrift_Map, Set as _fbthrift_Set


def _get_spec() -> ServiceSpec:
    spec = inspect(TestingServiceInterface)
    assert isinstance(spec, ServiceSpec)
    return spec


def _get_func(name: str) -> FunctionSpec:
    func = _get_spec().get_function(name)
    assert func is not None, f"function {name!r} not found"
    return func


class InspectServiceBasicTest(unittest.TestCase):
    def test_service_interface_returns_service_spec(self) -> None:
        self.assertIsInstance(inspect(TestingServiceInterface), ServiceSpec)

    def test_client_returns_service_spec(self) -> None:
        self.assertIsInstance(inspect(TestingService), ServiceSpec)  # pyre-ignore[6]

    def test_service_and_client_same_name(self) -> None:
        service_spec = inspect(TestingServiceInterface)
        client_spec = inspect(TestingService)  # pyre-ignore[6]
        assert isinstance(service_spec, ServiceSpec)
        assert isinstance(client_spec, ServiceSpec)
        self.assertEqual(service_spec.name, client_spec.name)
        self.assertEqual(
            set(service_spec.functions.keys()), set(client_spec.functions.keys())
        )

    def test_service_name(self) -> None:
        self.assertEqual(_get_spec().name, "TestingService")

    def test_service_functions_present(self) -> None:
        spec = _get_spec()
        for name in ("getName", "shutdown", "invert", "complex_action", "takes_a_list"):
            self.assertIsNotNone(spec.get_function(name), f"missing {name}")

    def test_get_function_returns_none_for_unknown(self) -> None:
        self.assertIsNone(_get_spec().get_function("nonexistent"))

    def test_inspectable(self) -> None:
        self.assertTrue(inspectable(TestingServiceInterface))
        self.assertTrue(inspectable(TestingService))


class ReturnTypeTest(unittest.TestCase):
    @parameterized.expand(
        [
            ("string", "getName", str, ThriftType.STRING),
            ("bool", "invert", bool, ThriftType.BOOL),
            ("i32", "getPriority", int, ThriftType.I32),
            ("float", "getRequestTimeout", float, ThriftType.FLOAT),
            ("struct", "echoStruct", easy, ThriftType.STRUCT),
            ("enum", "echoColor", Color, ThriftType.ENUM),
            ("typedef_container", "echoList", I32List, ThriftType.LIST),
        ]
    )
    def test_return_type(
        self,
        _name: str,
        func_name: str,
        expected_type: type[Any],
        expected_thrift_type: ThriftType,
    ) -> None:
        func = _get_func(func_name)
        self.assertEqual(func.return_type, expected_type)
        self.assertEqual(func.return_thrift_type, expected_thrift_type)

    def test_non_typedef_container_return(self) -> None:
        func = _get_func("echoMap")
        assert func.return_type is not None
        self.assertTrue(issubclass(func.return_type, _fbthrift_Map))
        self.assertEqual(func.return_thrift_type, ThriftType.MAP)

    @parameterized.expand(
        [
            ("void", "pick_a_color"),
            ("oneway", "shutdown"),
        ]
    )
    def test_void_return(self, _name: str, func_name: str) -> None:
        func = _get_func(func_name)
        self.assertIsNone(func.return_type)
        self.assertEqual(func.return_thrift_type, ThriftType.VOID)


class QualifierTest(unittest.TestCase):
    @parameterized.expand(
        [
            ("readonly", "getName", FunctionQualifier.READ_ONLY),
            ("idempotent", "invert", FunctionQualifier.IDEMPOTENT),
            ("oneway", "shutdown", FunctionQualifier.ONE_WAY),
            ("unspecified", "complex_action", FunctionQualifier.UNSPECIFIED),
        ]
    )
    def test_qualifier(
        self, _name: str, func_name: str, expected: FunctionQualifier
    ) -> None:
        self.assertEqual(_get_func(func_name).qualifier, expected)

    def test_is_oneway(self) -> None:
        self.assertTrue(_get_func("shutdown").is_oneway)
        self.assertFalse(_get_func("getName").is_oneway)


class ArgumentsTest(unittest.TestCase):
    def test_no_arguments(self) -> None:
        self.assertEqual(len(_get_func("getName").arguments), 0)

    def test_argument_count(self) -> None:
        self.assertEqual(len(_get_func("complex_action").arguments), 4)

    @parameterized.expand(
        [
            ("string", "complex_action", "first", str, ThriftType.STRING),
            ("i64", "complex_action", "third", int, ThriftType.I64),
            ("struct", "echoStruct", "what", easy, ThriftType.STRUCT),
            ("enum", "echoColor", "color", Color, ThriftType.ENUM),
            ("typedef_container", "echoList", "ints", I32List, ThriftType.LIST),
        ]
    )
    def test_argument_type(
        self,
        _name: str,
        func_name: str,
        arg_name: str,
        expected_type: type[Any],
        expected_thrift_type: ThriftType,
    ) -> None:
        arg = _get_func(func_name).get_argument(arg_name)
        assert arg is not None
        self.assertEqual(arg.type, expected_type)
        self.assertEqual(arg.thrift_type, expected_thrift_type)

    def test_non_typedef_container_argument(self) -> None:
        arg = _get_func("echoMap").get_argument("m")
        assert arg is not None
        self.assertTrue(issubclass(arg.type, _fbthrift_Map))
        self.assertEqual(arg.thrift_type, ThriftType.MAP)

    def test_arguments_by_name(self) -> None:
        by_name = _get_func("complex_action").arguments_by_name
        self.assertIn("first", by_name)
        self.assertEqual(by_name["first"].type, str)

    def test_get_argument(self) -> None:
        func = _get_func("complex_action")
        first = func.get_argument("first")
        assert first is not None
        self.assertEqual(first.type, str)
        self.assertIsNone(func.get_argument("nonexistent"))

    def test_arguments_are_argument_spec(self) -> None:
        for arg in _get_func("complex_action").arguments:
            self.assertIsInstance(arg, ArgumentSpec)


class ExceptionsTest(unittest.TestCase):
    def test_no_exceptions(self) -> None:
        self.assertEqual(len(_get_func("getName").exceptions), 0)

    @parameterized.expand(
        [
            ("simple_error", "takes_a_list", SimpleError),
            ("hard_error", "hard_error", HardError),
        ]
    )
    def test_declared_exception(
        self, _name: str, func_name: str, expected_type: type[Any]
    ) -> None:
        func = _get_func(func_name)
        self.assertEqual(len(func.exceptions), 1)
        self.assertIs(func.exceptions[0].type, expected_type)
        self.assertEqual(func.exceptions[0].thrift_type, ThriftType.STRUCT)

    def test_exceptions_are_exception_spec(self) -> None:
        for exc in _get_func("takes_a_list").exceptions:
            self.assertIsInstance(exc, ExceptionSpec)

    def test_exceptions_by_name(self) -> None:
        by_name = _get_func("takes_a_list").exceptions_by_name
        self.assertIn("e", by_name)
        self.assertIs(by_name["e"].type, SimpleError)

    def test_get_exception(self) -> None:
        func = _get_func("takes_a_list")
        exc = func.get_exception("e")
        assert exc is not None
        self.assertIs(exc.type, SimpleError)
        self.assertIsNone(func.get_exception("nonexistent"))


class InheritanceTest(unittest.TestCase):
    def test_no_parent(self) -> None:
        self.assertIsNone(_get_spec().parent)

    def test_child_service_has_parent(self) -> None:
        spec = inspect(TestingServiceChildInterface)
        assert isinstance(spec, ServiceSpec)
        self.assertEqual(spec.name, "TestingServiceChild")
        parent = spec.parent
        assert parent is not None
        self.assertEqual(parent.name, "TestingService")
        self.assertIn("getName", parent.functions)

    def test_child_client_has_parent(self) -> None:
        spec = inspect(TestingServiceChild)  # pyre-ignore[6]
        assert isinstance(spec, ServiceSpec)
        self.assertIsNotNone(spec.parent)


class ImmutabilityTest(unittest.TestCase):
    def test_service_spec_immutable(self) -> None:
        with self.assertRaises(AttributeError):
            _get_spec().name = "X"  # type: ignore[misc]

    def test_function_spec_immutable(self) -> None:
        with self.assertRaises(AttributeError):
            _get_func("getName").name = "X"  # type: ignore[misc]


class ContainerTypeInspectabilityTest(unittest.TestCase):
    def test_typedef_list_arg_inspectable(self) -> None:
        arg = _get_func("echoList").get_argument("ints")
        assert arg is not None
        self.assertEqual(arg.type, I32List)
        self.assertTrue(inspectable(arg.type))
        spec = inspect(arg.type)  # pyre-ignore[6]
        self.assertIsInstance(spec, ListSpec)
        assert isinstance(spec, ListSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_typedef_list_return_inspectable(self) -> None:
        func = _get_func("echoList")
        assert func.return_type is not None
        self.assertEqual(func.return_type, I32List)
        spec = inspect(func.return_type)  # pyre-ignore[6]
        self.assertIsInstance(spec, ListSpec)
        assert isinstance(spec, ListSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_non_typedef_map_arg_inspectable(self) -> None:
        arg = _get_func("echoMap").get_argument("m")
        assert arg is not None
        self.assertTrue(inspectable(arg.type))
        spec = inspect(arg.type)  # pyre-ignore[6]
        self.assertIsInstance(spec, MapSpec)
        assert isinstance(spec, MapSpec)
        self.assertEqual(spec.key, Color)
        self.assertEqual(spec.key_thrift_type, ThriftType.ENUM)
        self.assertEqual(spec.value, easy)
        self.assertEqual(spec.value_thrift_type, ThriftType.STRUCT)

    def test_non_typedef_map_return_inspectable(self) -> None:
        func = _get_func("echoMap")
        assert func.return_type is not None
        self.assertTrue(inspectable(func.return_type))
        spec = inspect(func.return_type)  # pyre-ignore[6]
        self.assertIsInstance(spec, MapSpec)
        assert isinstance(spec, MapSpec)
        self.assertEqual(spec.key, Color)
        self.assertEqual(spec.value, easy)

    def test_typedef_set_arg_inspectable(self) -> None:
        arg = _get_func("echoSet").get_argument("s")
        assert arg is not None
        self.assertEqual(arg.type, SetI32)
        self.assertTrue(inspectable(arg.type))
        spec = inspect(arg.type)  # pyre-ignore[6]
        self.assertIsInstance(spec, SetSpec)
        assert isinstance(spec, SetSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_typedef_set_return_inspectable(self) -> None:
        func = _get_func("echoSet")
        assert func.return_type is not None
        self.assertEqual(func.return_type, SetI32)
        spec = inspect(func.return_type)  # pyre-ignore[6]
        self.assertIsInstance(spec, SetSpec)
        assert isinstance(spec, SetSpec)
        self.assertEqual(spec.value, int)

    def test_non_typedef_set_arg_inspectable(self) -> None:
        arg = _get_func("echoSetNoTypedef").get_argument("s")
        assert arg is not None
        self.assertTrue(issubclass(arg.type, _fbthrift_Set))
        self.assertTrue(inspectable(arg.type))
        spec = inspect(arg.type)  # pyre-ignore[6]
        self.assertIsInstance(spec, SetSpec)
        assert isinstance(spec, SetSpec)
        self.assertEqual(spec.value, str)
        self.assertEqual(spec.thrift_type, ThriftType.STRING)

    def test_non_typedef_set_return_inspectable(self) -> None:
        func = _get_func("echoSetNoTypedef")
        assert func.return_type is not None
        self.assertTrue(issubclass(func.return_type, _fbthrift_Set))
        spec = inspect(func.return_type)  # pyre-ignore[6]
        self.assertIsInstance(spec, SetSpec)
        assert isinstance(spec, SetSpec)
        self.assertEqual(spec.value, str)


class StreamReflectionTest(unittest.TestCase):
    def _get_child_spec(self) -> ServiceSpec:
        spec = inspect(TestingServiceChildInterface)
        assert isinstance(spec, ServiceSpec)
        return spec

    def _get_child_func(self, name: str) -> FunctionSpec:
        func = self._get_child_spec().get_function(name)
        assert func is not None, f"function {name!r} not found"
        return func

    def test_stream_return_type_is_void(self) -> None:
        func = self._get_child_func("stream_func")
        self.assertIsNone(func.return_type)
        self.assertEqual(func.return_thrift_type, ThriftType.VOID)

    def test_stream_func_has_stream_spec(self) -> None:
        func = self._get_child_func("stream_func")
        self.assertIsNotNone(func.stream)
        self.assertIsNone(func.sink)

    def test_stream_elem_type(self) -> None:
        func = self._get_child_func("stream_func")
        stream = func.stream
        assert stream is not None
        self.assertIsInstance(stream, StreamSpec)
        self.assertEqual(stream.elem_type, int)
        self.assertEqual(stream.elem_thrift_type, ThriftType.I32)

    def test_stream_no_exceptions(self) -> None:
        func = self._get_child_func("stream_func")
        stream = func.stream
        assert stream is not None
        self.assertEqual(len(stream.exceptions), 0)

    def test_stream_is_streaming(self) -> None:
        func = self._get_child_func("stream_func")
        self.assertTrue(func.is_streaming)
        self.assertFalse(func.is_bidi)

    def test_non_streaming_func(self) -> None:
        func = _get_func("getName")
        self.assertIsNone(func.stream)
        self.assertIsNone(func.sink)
        self.assertFalse(func.is_streaming)
        self.assertFalse(func.is_bidi)


class SinkReflectionTest(unittest.TestCase):
    def _get_sink_spec(self) -> ServiceSpec:
        spec = inspect(TestSinkServiceInterface)
        assert isinstance(spec, ServiceSpec)
        return spec

    def _get_sink_func(self, name: str) -> FunctionSpec:
        func = self._get_sink_spec().get_function(name)
        assert func is not None, f"function {name!r} not found"
        return func

    def test_sink_return_type_is_void(self) -> None:
        func = self._get_sink_func("range_")
        self.assertIsNone(func.return_type)
        self.assertEqual(func.return_thrift_type, ThriftType.VOID)

    def test_basic_sink(self) -> None:
        func = self._get_sink_func("range_")
        self.assertIsNotNone(func.sink)
        self.assertIsNone(func.stream)
        self.assertTrue(func.is_streaming)
        self.assertFalse(func.is_bidi)

    def test_sink_payload_type(self) -> None:
        func = self._get_sink_func("range_")
        sink = func.sink
        assert sink is not None
        self.assertIsInstance(sink, SinkSpec)
        self.assertEqual(sink.payload_type, int)
        self.assertEqual(sink.payload_thrift_type, ThriftType.I32)

    def test_sink_final_response_type(self) -> None:
        func = self._get_sink_func("range_")
        sink = func.sink
        assert sink is not None
        self.assertEqual(sink.final_response_type, int)
        self.assertEqual(sink.final_response_thrift_type, ThriftType.I32)

    def test_sink_with_initial_response(self) -> None:
        func = self._get_sink_func("initialThrow")
        self.assertEqual(func.return_type, bool)
        self.assertEqual(func.return_thrift_type, ThriftType.BOOL)
        sink = func.sink
        assert sink is not None
        self.assertEqual(sink.payload_type, int)
        self.assertEqual(sink.final_response_type, bool)

    def test_sink_payload_exceptions(self) -> None:
        func = self._get_sink_func("sinkThrow")
        sink = func.sink
        assert sink is not None
        self.assertEqual(len(sink.payload_exceptions), 1)
        self.assertIsInstance(sink.payload_exceptions[0], ExceptionSpec)

    def test_sink_final_response_exceptions(self) -> None:
        func = self._get_sink_func("sinkFinalThrow")
        sink = func.sink
        assert sink is not None
        self.assertEqual(len(sink.final_response_exceptions), 1)
        self.assertIsInstance(sink.final_response_exceptions[0], ExceptionSpec)

    def test_sink_no_exceptions(self) -> None:
        func = self._get_sink_func("range_")
        sink = func.sink
        assert sink is not None
        self.assertEqual(len(sink.payload_exceptions), 0)
        self.assertEqual(len(sink.final_response_exceptions), 0)


class BidiReflectionTest(unittest.TestCase):
    def _get_bidi_spec(self) -> ServiceSpec:
        spec = inspect(TestBidiServiceInterface)
        assert isinstance(spec, ServiceSpec)
        return spec

    def _get_bidi_func(self, name: str) -> FunctionSpec:
        func = self._get_bidi_spec().get_function(name)
        assert func is not None, f"function {name!r} not found"
        return func

    def test_bidi_return_type_is_void(self) -> None:
        func = self._get_bidi_func("echo")
        self.assertIsNone(func.return_type)
        self.assertEqual(func.return_thrift_type, ThriftType.VOID)

    def test_bidi_has_both_stream_and_sink(self) -> None:
        func = self._get_bidi_func("echo")
        self.assertIsNotNone(func.stream)
        self.assertIsNotNone(func.sink)
        self.assertTrue(func.is_streaming)
        self.assertTrue(func.is_bidi)

    def test_bidi_stream_elem_type(self) -> None:
        func = self._get_bidi_func("echo")
        stream = func.stream
        assert stream is not None
        self.assertEqual(stream.elem_type, str)
        self.assertEqual(stream.elem_thrift_type, ThriftType.STRING)

    def test_bidi_sink_payload_type(self) -> None:
        func = self._get_bidi_func("echo")
        sink = func.sink
        assert sink is not None
        self.assertEqual(sink.payload_type, str)
        self.assertEqual(sink.payload_thrift_type, ThriftType.STRING)

    def test_bidi_sink_no_final_response(self) -> None:
        func = self._get_bidi_func("echo")
        sink = func.sink
        assert sink is not None
        self.assertIsNone(sink.final_response_type)
        self.assertEqual(sink.final_response_thrift_type, ThriftType.VOID)

    def test_bidi_with_initial_response(self) -> None:
        func = self._get_bidi_func("echoWithResponse")
        self.assertEqual(func.return_type, str)
        self.assertEqual(func.return_thrift_type, ThriftType.STRING)
        self.assertIsNotNone(func.stream)
        self.assertIsNotNone(func.sink)

    def test_bidi_different_types(self) -> None:
        func = self._get_bidi_func("intStream")
        stream = func.stream
        sink = func.sink
        assert stream is not None
        assert sink is not None
        self.assertEqual(stream.elem_type, int)
        self.assertEqual(stream.elem_thrift_type, ThriftType.I32)
        self.assertEqual(sink.payload_type, str)
        self.assertEqual(sink.payload_thrift_type, ThriftType.STRING)

    def test_bidi_struct_types(self) -> None:
        func = self._get_bidi_func("structBidi")
        sink = func.sink
        stream = func.stream
        assert sink is not None
        assert stream is not None
        self.assertEqual(sink.payload_type, SinkChunk)
        self.assertEqual(sink.payload_thrift_type, ThriftType.STRUCT)
        self.assertEqual(stream.elem_type, StreamChunk)
        self.assertEqual(stream.elem_thrift_type, ThriftType.STRUCT)

    def test_bidi_with_exceptions(self) -> None:
        func = self._get_bidi_func("canThrow")
        stream = func.stream
        sink = func.sink
        assert stream is not None
        assert sink is not None
        self.assertEqual(stream.elem_type, int)
        self.assertEqual(stream.elem_thrift_type, ThriftType.I64)
        self.assertEqual(sink.payload_type, int)
        self.assertEqual(sink.payload_thrift_type, ThriftType.I64)
        self.assertEqual(len(func.exceptions), 1)
        self.assertIs(func.exceptions[0].type, MethodException)
        self.assertEqual(len(sink.payload_exceptions), 1)
        self.assertIs(sink.payload_exceptions[0].type, SinkException)
        self.assertEqual(len(stream.exceptions), 1)
        self.assertIs(stream.exceptions[0].type, StreamException)


class AnnotationsTest(unittest.TestCase):
    def test_service_structured_annotations(self) -> None:
        self.assertIn(
            "test_thrift.StructuredAnnotation", _get_spec().structured_annotations
        )
