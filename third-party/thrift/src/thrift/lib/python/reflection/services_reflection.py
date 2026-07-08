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

from __future__ import annotations

import builtins
from types import MappingProxyType
from typing import Any, Sequence

from thrift.python.reflection.constants_reflection import (
    _ImmutableMeta,
    ConstantSpec,
    ThriftType,
)
from thrift.python.reflection.types_reflection import (
    _structured_annotations_to_dict,
    FieldSpec,
)
from thrift.python.reflection_enums import FunctionQualifier


class ArgumentSpec(FieldSpec):
    def __init__(self, **kwargs: Any) -> None:
        super().__init__(**kwargs)

    def __eq__(self, other: object) -> bool:
        if type(other) is not ArgumentSpec:
            return NotImplemented
        return super().__eq__(other)

    def __repr__(self) -> str:
        return (
            f"ArgumentSpec(id={self.id}, name={self.name!r}, py_name={self.py_name!r}, "
            f"type={self.type!r}, thrift_type={self.thrift_type!r}, "
            f"qualifier={self.qualifier!r}, default={self.default!r})"
        )


class ExceptionSpec(FieldSpec):
    def __init__(self, **kwargs: Any) -> None:
        super().__init__(**kwargs)

    def __eq__(self, other: object) -> bool:
        if type(other) is not ExceptionSpec:
            return NotImplemented
        return super().__eq__(other)

    def __repr__(self) -> str:
        return (
            f"ExceptionSpec(id={self.id}, name={self.name!r}, py_name={self.py_name!r}, "
            f"type={self.type!r}, thrift_type={self.thrift_type!r}, "
            f"qualifier={self.qualifier!r}, default={self.default!r})"
        )


class StreamSpec(metaclass=_ImmutableMeta):
    __slots__ = ("elem_type", "elem_thrift_type", "exceptions")
    elem_type: builtins.type[Any]
    elem_thrift_type: ThriftType
    exceptions: tuple[ExceptionSpec, ...]

    def __init__(
        self,
        *,
        elem_type: builtins.type[Any],
        elem_thrift_type: ThriftType,
        exceptions: Sequence[ExceptionSpec] | None = None,
    ) -> None:
        self.elem_type = elem_type
        self.elem_thrift_type = elem_thrift_type
        self.exceptions = tuple(exceptions) if exceptions else ()

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, StreamSpec):
            return NotImplemented
        return (
            self.elem_type == other.elem_type
            and self.elem_thrift_type == other.elem_thrift_type
            and self.exceptions == other.exceptions
        )

    def __repr__(self) -> str:
        return (
            f"StreamSpec(elem_type={self.elem_type!r}, "
            f"elem_thrift_type={self.elem_thrift_type!r})"
        )


class SinkSpec(metaclass=_ImmutableMeta):
    __slots__ = (
        "payload_type",
        "payload_thrift_type",
        "payload_exceptions",
        "final_response_type",
        "final_response_thrift_type",
        "final_response_exceptions",
    )
    payload_type: builtins.type[Any]
    payload_thrift_type: ThriftType
    payload_exceptions: tuple[ExceptionSpec, ...]
    final_response_type: builtins.type[Any] | None
    final_response_thrift_type: ThriftType
    final_response_exceptions: tuple[ExceptionSpec, ...]

    def __init__(
        self,
        *,
        payload_type: builtins.type[Any],
        payload_thrift_type: ThriftType,
        payload_exceptions: Sequence[ExceptionSpec] | None = None,
        final_response_type: builtins.type[Any] | None = None,
        final_response_thrift_type: ThriftType = ThriftType.VOID,
        final_response_exceptions: Sequence[ExceptionSpec] | None = None,
    ) -> None:
        self.payload_type = payload_type
        self.payload_thrift_type = payload_thrift_type
        self.payload_exceptions = (
            tuple(payload_exceptions) if payload_exceptions else ()
        )
        self.final_response_type = final_response_type
        self.final_response_thrift_type = final_response_thrift_type
        self.final_response_exceptions = (
            tuple(final_response_exceptions) if final_response_exceptions else ()
        )

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SinkSpec):
            return NotImplemented
        return (
            self.payload_type == other.payload_type
            and self.payload_thrift_type == other.payload_thrift_type
            and self.payload_exceptions == other.payload_exceptions
            and self.final_response_type == other.final_response_type
            and self.final_response_thrift_type == other.final_response_thrift_type
            and self.final_response_exceptions == other.final_response_exceptions
        )

    def __repr__(self) -> str:
        return (
            f"SinkSpec(payload_type={self.payload_type!r}, "
            f"final_response_type={self.final_response_type!r})"
        )


class FunctionSpec(metaclass=_ImmutableMeta):
    # For streaming functions (stream, sink, bidi), ``return_type`` and
    # ``return_thrift_type`` represent the *initial response* only.  They are
    # ``None`` / ``ThriftType.VOID`` when the function has no initial response
    # (e.g. ``stream<i32> foo()``).  The stream/sink element types and their
    # exceptions are available via the ``stream`` and ``sink`` fields.  A
    # function is bidirectional when both ``stream`` and ``sink`` are set.
    __slots__ = (
        "name",
        "arguments",
        "_arguments_by_name",
        "return_type",
        "return_thrift_type",
        "qualifier",
        "exceptions",
        "_exceptions_by_name",
        "stream",
        "sink",
        "structured_annotations",
    )
    name: str
    arguments: tuple[ArgumentSpec, ...]
    _arguments_by_name: MappingProxyType[str, ArgumentSpec] | None
    return_type: builtins.type[Any] | None
    return_thrift_type: ThriftType
    qualifier: FunctionQualifier
    exceptions: tuple[ExceptionSpec, ...]
    _exceptions_by_name: MappingProxyType[str, ExceptionSpec] | None
    stream: StreamSpec | None
    sink: SinkSpec | None
    structured_annotations: MappingProxyType[str, ConstantSpec]

    def __init__(
        self,
        *,
        name: str,
        arguments: Sequence[ArgumentSpec] | None = None,
        return_type: builtins.type[Any] | None = None,
        return_thrift_type: ThriftType = ThriftType.VOID,
        qualifier: FunctionQualifier = FunctionQualifier.UNSPECIFIED,
        exceptions: Sequence[ExceptionSpec] | None = None,
        stream: StreamSpec | None = None,
        sink: SinkSpec | None = None,
        structured_annotations: dict[str, ConstantSpec] | None = None,
    ) -> None:
        self.name = name
        self.arguments = tuple(arguments) if arguments else ()
        self._arguments_by_name = None
        self.return_type = return_type
        self.return_thrift_type = return_thrift_type
        self.qualifier = qualifier
        self.exceptions = tuple(exceptions) if exceptions else ()
        self._exceptions_by_name = None
        self.stream = stream
        self.sink = sink
        self.structured_annotations = MappingProxyType(structured_annotations or {})

    @property
    def is_oneway(self) -> bool:
        return self.qualifier == FunctionQualifier.ONE_WAY

    @property
    def is_streaming(self) -> bool:
        return self.stream is not None or self.sink is not None

    @property
    def is_bidi(self) -> bool:
        return self.stream is not None and self.sink is not None

    @property
    def arguments_by_name(self) -> MappingProxyType[str, ArgumentSpec]:
        result = self._arguments_by_name
        if result is None:
            result = MappingProxyType({f.py_name: f for f in self.arguments})
            object.__setattr__(self, "_arguments_by_name", result)
        return result

    def get_argument(self, name: str) -> ArgumentSpec | None:
        if (field := self.arguments_by_name.get(name)) is not None:
            return field
        return next((f for f in self.arguments if f.name == name), None)

    @property
    def exceptions_by_name(self) -> MappingProxyType[str, ExceptionSpec]:
        result = self._exceptions_by_name
        if result is None:
            result = MappingProxyType({f.py_name: f for f in self.exceptions})
            object.__setattr__(self, "_exceptions_by_name", result)
        return result

    def get_exception(self, name: str) -> ExceptionSpec | None:
        if (field := self.exceptions_by_name.get(name)) is not None:
            return field
        return next((f for f in self.exceptions if f.name == name), None)

    @property
    def annotations(self) -> dict[str, str | dict[str, object]]:
        return _structured_annotations_to_dict(self.structured_annotations)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, FunctionSpec):
            return NotImplemented
        return (
            self.name == other.name
            and self.arguments == other.arguments
            and self.return_type == other.return_type
            and self.return_thrift_type == other.return_thrift_type
            and self.qualifier == other.qualifier
            and self.exceptions == other.exceptions
            and self.stream == other.stream
            and self.sink == other.sink
            and self.structured_annotations == other.structured_annotations
        )

    def __repr__(self) -> str:
        return (
            f"FunctionSpec(name={self.name!r}, "
            f"return_type={self.return_type!r}, "
            f"return_thrift_type={self.return_thrift_type!r})"
        )


class ServiceSpec(metaclass=_ImmutableMeta):
    __slots__ = ("name", "functions", "parent", "structured_annotations")
    name: str
    functions: MappingProxyType[str, FunctionSpec]
    parent: ServiceSpec | None
    structured_annotations: MappingProxyType[str, ConstantSpec]

    def __init__(
        self,
        *,
        name: str,
        functions: dict[str, FunctionSpec] | None = None,
        parent: ServiceSpec | None = None,
        structured_annotations: dict[str, ConstantSpec] | None = None,
    ) -> None:
        self.name = name
        self.functions = MappingProxyType(functions or {})
        self.parent = parent
        self.structured_annotations = MappingProxyType(structured_annotations or {})

    def get_function(self, name: str) -> FunctionSpec | None:
        return self.functions.get(name)

    @property
    def annotations(self) -> dict[str, str | dict[str, object]]:
        return _structured_annotations_to_dict(self.structured_annotations)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ServiceSpec):
            return NotImplemented
        return (
            self.name == other.name
            and self.functions == other.functions
            and self.parent == other.parent
            and self.structured_annotations == other.structured_annotations
        )

    def __repr__(self) -> str:
        return (
            f"ServiceSpec(name={self.name!r}, "
            f"functions={list(self.functions.keys())!r})"
        )
