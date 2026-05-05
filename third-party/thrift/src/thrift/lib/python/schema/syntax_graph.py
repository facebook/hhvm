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

"""
Pure Python SyntaxGraph -- AST-level graph of .thrift files.

Reads serialized type::Schema data and builds an in-memory graph
of Python objects.
"""

from __future__ import annotations

from typing import Any, Generic, TypeVar

from apache.thrift.syntax_graph.syntax_graph.thrift_types import (  # noqa: F401 -- re-export
    FieldPresenceQualifier,
    Primitive,
)
from apache.thrift.type.schema import thrift_types as _schema_types
from thrift.python.serializer import deserialize, Protocol


# ---------------------------------------------------------------------------
# Internal: Lazy resolution for cyclic references
# ---------------------------------------------------------------------------

T = TypeVar("T")


class _Lazy(Generic[T]):
    """Deferred reference resolved on first access. Handles cyclic types."""

    __slots__ = ("_resolver", "_definition_key", "_resolved")
    _resolver: _Resolver
    _definition_key: bytes
    _resolved: Any | None

    def __init__(self, resolver: _Resolver, definition_key: bytes) -> None:
        self._resolver = resolver
        self._definition_key = definition_key
        self._resolved = None

    def resolve(self) -> Any:
        if self._resolved is None:
            self._resolved = self._resolver.resolve(self._definition_key)
        return self._resolved

    @property
    def definition_key(self) -> bytes:
        return self._definition_key


class _Resolver:
    """Maps DefinitionKey -> Definition node. Shared across all lazy refs."""

    def __init__(self) -> None:
        self._definitions: dict[bytes, Definition] = {}

    def register(self, key: bytes, node: Definition) -> None:
        if key in self._definitions:
            raise RuntimeError(f"Duplicate definition key: {key!r}")
        self._definitions[key] = node

    def resolve(self, key: bytes) -> Definition:
        node = self._definitions.get(key)
        if node is None:
            raise RuntimeError(f"Unresolvable definition key: {key!r}")
        return node

    def __contains__(self, key: bytes) -> bool:
        return key in self._definitions


# ---------------------------------------------------------------------------
# TypeRef hierarchy
# ---------------------------------------------------------------------------


class TypeRef:
    """Abstract base for type references."""

    __slots__: tuple[str, ...] = ()

    @property
    def true_type(self) -> TypeRef:
        return self

    def __eq__(self, other: object) -> bool:
        return NotImplemented

    def __hash__(self) -> int:
        return id(self)

    def __repr__(self) -> str:
        return f"{type(self).__name__}()"


class PrimitiveTypeRef(TypeRef):
    """Reference to a primitive type (bool, i32, string, etc.)."""

    __slots__ = ("_primitive",)
    _primitive: Primitive

    def __init__(self, primitive: Primitive) -> None:
        self._primitive = primitive

    @property
    def primitive(self) -> Primitive:
        return self._primitive

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, PrimitiveTypeRef):
            return NotImplemented
        return self._primitive == other._primitive

    def __hash__(self) -> int:
        return hash(("primitive", self._primitive))

    def __repr__(self) -> str:
        return f"PrimitiveTypeRef({self._primitive.name})"


class StructTypeRef(TypeRef):
    """Reference to a struct type."""

    __slots__ = ("_lazy",)
    _lazy: _Lazy[StructNode]

    def __init__(self, lazy: _Lazy[StructNode]) -> None:
        self._lazy = lazy

    @property
    def node(self) -> StructNode:
        defn = self._lazy.resolve()
        assert isinstance(defn, StructNode)
        return defn

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, StructTypeRef):
            return NotImplemented
        return self._lazy.definition_key == other._lazy.definition_key

    def __hash__(self) -> int:
        return hash(("struct", self._lazy.definition_key))

    def __repr__(self) -> str:
        return f"StructTypeRef({self.node.name!r})"


class UnionTypeRef(TypeRef):
    """Reference to a union type."""

    __slots__ = ("_lazy",)
    _lazy: _Lazy[UnionNode]

    def __init__(self, lazy: _Lazy[UnionNode]) -> None:
        self._lazy = lazy

    @property
    def node(self) -> UnionNode:
        defn = self._lazy.resolve()
        assert isinstance(defn, UnionNode)
        return defn

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, UnionTypeRef):
            return NotImplemented
        return self._lazy.definition_key == other._lazy.definition_key

    def __hash__(self) -> int:
        return hash(("union", self._lazy.definition_key))

    def __repr__(self) -> str:
        return f"UnionTypeRef({self.node.name!r})"


class ExceptionTypeRef(TypeRef):
    """Reference to an exception type."""

    __slots__ = ("_lazy",)
    _lazy: _Lazy[ExceptionNode]

    def __init__(self, lazy: _Lazy[ExceptionNode]) -> None:
        self._lazy = lazy

    @property
    def node(self) -> ExceptionNode:
        defn = self._lazy.resolve()
        assert isinstance(defn, ExceptionNode)
        return defn

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ExceptionTypeRef):
            return NotImplemented
        return self._lazy.definition_key == other._lazy.definition_key

    def __hash__(self) -> int:
        return hash(("exception", self._lazy.definition_key))

    def __repr__(self) -> str:
        return f"ExceptionTypeRef({self.node.name!r})"


class EnumTypeRef(TypeRef):
    """Reference to an enum type."""

    __slots__ = ("_lazy",)
    _lazy: _Lazy[EnumNode]

    def __init__(self, lazy: _Lazy[EnumNode]) -> None:
        self._lazy = lazy

    @property
    def node(self) -> EnumNode:
        defn = self._lazy.resolve()
        assert isinstance(defn, EnumNode)
        return defn

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, EnumTypeRef):
            return NotImplemented
        return self._lazy.definition_key == other._lazy.definition_key

    def __hash__(self) -> int:
        return hash(("enum", self._lazy.definition_key))

    def __repr__(self) -> str:
        return f"EnumTypeRef({self.node.name!r})"


class TypedefTypeRef(TypeRef):
    """Reference to a typedef type."""

    __slots__ = ("_lazy",)
    _lazy: _Lazy[TypedefNode]

    def __init__(self, lazy: _Lazy[TypedefNode]) -> None:
        self._lazy = lazy

    @property
    def node(self) -> TypedefNode:
        defn = self._lazy.resolve()
        assert isinstance(defn, TypedefNode)
        return defn

    @property
    def true_type(self) -> TypeRef:
        return self.node.target_type.true_type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, TypedefTypeRef):
            return NotImplemented
        return self._lazy.definition_key == other._lazy.definition_key

    def __hash__(self) -> int:
        return hash(("typedef", self._lazy.definition_key))

    def __repr__(self) -> str:
        return f"TypedefTypeRef({self.node.name!r})"


class ListTypeRef(TypeRef):
    """Reference to a list<T> type."""

    __slots__ = ("_element_type",)
    _element_type: TypeRef

    def __init__(self, element_type: TypeRef) -> None:
        self._element_type = element_type

    @property
    def element_type(self) -> TypeRef:
        return self._element_type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ListTypeRef):
            return NotImplemented
        return self._element_type == other._element_type

    def __hash__(self) -> int:
        return hash(("list", self._element_type))

    def __repr__(self) -> str:
        return f"ListTypeRef({self._element_type!r})"


class SetTypeRef(TypeRef):
    """Reference to a set<T> type."""

    __slots__ = ("_element_type",)
    _element_type: TypeRef

    def __init__(self, element_type: TypeRef) -> None:
        self._element_type = element_type

    @property
    def element_type(self) -> TypeRef:
        return self._element_type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SetTypeRef):
            return NotImplemented
        return self._element_type == other._element_type

    def __hash__(self) -> int:
        return hash(("set", self._element_type))

    def __repr__(self) -> str:
        return f"SetTypeRef({self._element_type!r})"


class MapTypeRef(TypeRef):
    """Reference to a map<K,V> type."""

    __slots__ = ("_key_type", "_value_type")
    _key_type: TypeRef
    _value_type: TypeRef

    def __init__(self, key_type: TypeRef, value_type: TypeRef) -> None:
        self._key_type = key_type
        self._value_type = value_type

    @property
    def key_type(self) -> TypeRef:
        return self._key_type

    @property
    def value_type(self) -> TypeRef:
        return self._value_type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, MapTypeRef):
            return NotImplemented
        return (
            self._key_type == other._key_type and self._value_type == other._value_type
        )

    def __hash__(self) -> int:
        return hash(("map", self._key_type, self._value_type))

    def __repr__(self) -> str:
        return f"MapTypeRef({self._key_type!r}, {self._value_type!r})"


# ---------------------------------------------------------------------------
# Annotation
# ---------------------------------------------------------------------------


class Annotation:
    """A structured annotation applied to a definition, field, or enum value."""

    __slots__ = ("_type", "_value")
    _type: TypeRef
    _value: dict[str, Any]

    def __init__(self, type_ref: TypeRef, value: dict[str, Any]) -> None:
        self._type = type_ref
        self._value = value

    @property
    def type(self) -> TypeRef:
        return self._type

    @property
    def value(self) -> dict[str, Any]:
        return self._value

    def __repr__(self) -> str:
        return f"Annotation({self._type!r})"


# ---------------------------------------------------------------------------
# EnumValue
# ---------------------------------------------------------------------------


class EnumValue:
    """A value in an enum definition."""

    __slots__ = ("_name", "_value", "_annotations")
    _name: str
    _value: int
    _annotations: list[Annotation]

    def __init__(self, name: str, value: int, annotations: list[Annotation]) -> None:
        self._name = name
        self._value = value
        self._annotations = annotations

    @property
    def name(self) -> str:
        return self._name

    @property
    def value(self) -> int:
        return self._value

    @property
    def annotations(self) -> list[Annotation]:
        return self._annotations

    def __repr__(self) -> str:
        return f"EnumValue({self._name!r}, {self._value})"


# ---------------------------------------------------------------------------
# FieldNode
# ---------------------------------------------------------------------------


class FieldNode:
    """A field in a struct, union, or exception."""

    __slots__ = (
        "_id",
        "_name",
        "_type",
        "_presence",
        "_doc_block",
        "_annotations",
        "_default_value",
        "_parent",
    )
    _id: int
    _name: str
    _type: TypeRef
    _presence: FieldPresenceQualifier
    _doc_block: str | None
    _annotations: list[Annotation]
    _default_value: Any | None
    _parent: StructuredDefinition | None

    def __init__(
        self,
        *,
        id: int,
        name: str,
        type: TypeRef,
        presence: FieldPresenceQualifier,
        doc_block: str | None,
        annotations: list[Annotation],
        default_value: Any | None = None,
    ) -> None:
        self._id = id
        self._name = name
        self._type = type
        self._presence = presence
        self._doc_block = doc_block
        self._annotations = annotations
        self._default_value = default_value
        self._parent = None

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str:
        return self._name

    @property
    def type(self) -> TypeRef:
        return self._type

    @property
    def presence(self) -> FieldPresenceQualifier:
        return self._presence

    @property
    def doc_block(self) -> str | None:
        return self._doc_block

    @property
    def annotations(self) -> list[Annotation]:
        return self._annotations

    @property
    def default_value(self) -> Any | None:
        return self._default_value

    @property
    def parent(self) -> StructuredDefinition:
        assert self._parent is not None
        return self._parent

    def __repr__(self) -> str:
        return f"FieldNode({self._id}, {self._name!r})"


# ---------------------------------------------------------------------------
# Definition hierarchy
# ---------------------------------------------------------------------------


class Definition:
    """Abstract base for all top-level Thrift definitions."""

    __slots__: tuple[str, ...] = (
        "_name",
        "_definition_key",
        "_resolver",
        "_doc_block",
        "_annotations",
        "_program",
    )
    _name: str
    _definition_key: bytes
    _resolver: _Resolver
    _doc_block: str | None
    _annotations: list[Annotation]
    _program: ProgramNode | None

    def __init__(
        self,
        *,
        name: str,
        definition_key: bytes,
        resolver: _Resolver,
        doc_block: str | None,
        annotations: list[Annotation],
    ) -> None:
        self._name = name
        self._definition_key = definition_key
        self._resolver = resolver
        self._doc_block = doc_block
        self._annotations = annotations
        self._program = None

    @property
    def name(self) -> str:
        return self._name

    @property
    def program(self) -> ProgramNode:
        assert self._program is not None
        return self._program

    @property
    def doc_block(self) -> str | None:
        return self._doc_block

    @property
    def annotations(self) -> list[Annotation]:
        return self._annotations

    def as_type(self) -> TypeRef:
        raise TypeError(f"{type(self).__name__} cannot be used as a type reference")

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self._name!r})"


class StructuredDefinition(Definition):
    """Base for struct, union, and exception definitions."""

    __slots__: tuple[str, ...] = ("_uri", "_fields")
    _uri: str
    _fields: list[FieldNode]

    def __init__(
        self,
        *,
        uri: str,
        fields: list[FieldNode],
        **kwargs: Any,
    ) -> None:
        super().__init__(**kwargs)
        self._uri = uri
        self._fields = fields
        for field in fields:
            field._parent = self

    @property
    def uri(self) -> str:
        return self._uri

    @property
    def fields(self) -> list[FieldNode]:
        return self._fields


class StructNode(StructuredDefinition):
    """A Thrift struct definition."""

    __slots__ = ()

    def as_type(self) -> StructTypeRef:
        return StructTypeRef(_Lazy(self._resolver, self._definition_key))


class UnionNode(StructuredDefinition):
    """A Thrift union definition."""

    __slots__ = ()

    def as_type(self) -> UnionTypeRef:
        return UnionTypeRef(_Lazy(self._resolver, self._definition_key))


class ExceptionNode(StructuredDefinition):
    """A Thrift exception definition."""

    __slots__ = ()

    def as_type(self) -> ExceptionTypeRef:
        return ExceptionTypeRef(_Lazy(self._resolver, self._definition_key))


class EnumNode(Definition):
    """A Thrift enum definition."""

    __slots__ = ("_uri", "_values")
    _uri: str
    _values: list[EnumValue]

    def __init__(self, *, uri: str, values: list[EnumValue], **kwargs: Any) -> None:
        super().__init__(**kwargs)
        self._uri = uri
        self._values = values

    @property
    def uri(self) -> str:
        return self._uri

    @property
    def values(self) -> list[EnumValue]:
        return self._values

    def as_type(self) -> EnumTypeRef:
        return EnumTypeRef(_Lazy(self._resolver, self._definition_key))


class TypedefNode(Definition):
    """A Thrift typedef definition."""

    __slots__ = ("_target_type",)
    _target_type: TypeRef

    def __init__(self, *, target_type: TypeRef, **kwargs: Any) -> None:
        super().__init__(**kwargs)
        self._target_type = target_type

    @property
    def target_type(self) -> TypeRef:
        return self._target_type

    def as_type(self) -> TypedefTypeRef:
        return TypedefTypeRef(_Lazy(self._resolver, self._definition_key))


class ConstantNode(Definition):
    """A Thrift constant definition."""

    __slots__ = ("_type", "_value")
    _type: TypeRef
    _value: Any

    def __init__(self, *, type: TypeRef, value: Any = None, **kwargs: Any) -> None:
        super().__init__(**kwargs)
        self._type = type
        self._value = value

    @property
    def type(self) -> TypeRef:
        return self._type

    @property
    def value(self) -> Any:
        return self._value


# ---------------------------------------------------------------------------
# RPC types
# ---------------------------------------------------------------------------


class FunctionException:
    """A declared exception in a function or stream/sink."""

    __slots__ = ("_id", "_name", "_type")
    _id: int
    _name: str
    _type: TypeRef

    def __init__(self, *, id: int, name: str, type: TypeRef) -> None:
        self._id = id
        self._name = name
        self._type = type

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str:
        return self._name

    @property
    def type(self) -> TypeRef:
        return self._type

    def __repr__(self) -> str:
        return f"FunctionException({self._id}, {self._name!r})"


class FunctionStream:
    """A stream return type specification."""

    __slots__ = ("_payload_type", "_exceptions")
    _payload_type: TypeRef
    _exceptions: list[FunctionException]

    def __init__(
        self, payload_type: TypeRef, exceptions: list[FunctionException]
    ) -> None:
        self._payload_type = payload_type
        self._exceptions = exceptions

    @property
    def payload_type(self) -> TypeRef:
        return self._payload_type

    @property
    def exceptions(self) -> list[FunctionException]:
        return self._exceptions


class FunctionSink:
    """A sink return type specification."""

    __slots__ = (
        "_payload_type",
        "_final_response_type",
        "_client_exceptions",
        "_server_exceptions",
    )
    _payload_type: TypeRef
    _final_response_type: TypeRef
    _client_exceptions: list[FunctionException]
    _server_exceptions: list[FunctionException]

    def __init__(
        self,
        payload_type: TypeRef,
        final_response_type: TypeRef,
        client_exceptions: list[FunctionException],
        server_exceptions: list[FunctionException],
    ) -> None:
        self._payload_type = payload_type
        self._final_response_type = final_response_type
        self._client_exceptions = client_exceptions
        self._server_exceptions = server_exceptions

    @property
    def payload_type(self) -> TypeRef:
        return self._payload_type

    @property
    def final_response_type(self) -> TypeRef:
        return self._final_response_type

    @property
    def client_exceptions(self) -> list[FunctionException]:
        return self._client_exceptions

    @property
    def server_exceptions(self) -> list[FunctionException]:
        return self._server_exceptions


class FunctionResponse:
    """The response specification for a function."""

    __slots__ = ("_type", "_interaction_lazy", "_sink", "_stream")
    _type: TypeRef | None
    _interaction_lazy: _Lazy[InteractionNode] | None
    _sink: FunctionSink | None
    _stream: FunctionStream | None

    def __init__(
        self,
        *,
        type: TypeRef | None,
        interaction_lazy: _Lazy[InteractionNode] | None,
        sink: FunctionSink | None,
        stream: FunctionStream | None,
    ) -> None:
        self._type = type
        self._interaction_lazy = interaction_lazy
        self._sink = sink
        self._stream = stream

    @property
    def type(self) -> TypeRef | None:
        return self._type

    @property
    def interaction(self) -> InteractionNode | None:
        if self._interaction_lazy is None:
            return None
        defn = self._interaction_lazy.resolve()
        assert isinstance(defn, InteractionNode)
        return defn

    @property
    def sink(self) -> FunctionSink | None:
        return self._sink

    @property
    def stream(self) -> FunctionStream | None:
        return self._stream


class FunctionParam:
    """A parameter in a function's parameter list."""

    __slots__ = ("_id", "_name", "_type")
    _id: int
    _name: str
    _type: TypeRef

    def __init__(self, *, id: int, name: str, type: TypeRef) -> None:
        self._id = id
        self._name = name
        self._type = type

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str:
        return self._name

    @property
    def type(self) -> TypeRef:
        return self._type

    def __repr__(self) -> str:
        return f"FunctionParam({self._id}, {self._name!r})"


class FunctionNode:
    """An RPC function in a service or interaction."""

    __slots__ = (
        "_name",
        "_doc_block",
        "_parent",
        "_response",
        "_params",
        "_exceptions",
    )
    _name: str
    _doc_block: str | None
    _parent: ServiceNode | InteractionNode | None
    _response: FunctionResponse
    _params: list[FunctionParam]
    _exceptions: list[FunctionException]

    def __init__(
        self,
        *,
        name: str,
        doc_block: str | None,
        response: FunctionResponse,
        params: list[FunctionParam],
        exceptions: list[FunctionException],
    ) -> None:
        self._name = name
        self._doc_block = doc_block
        self._response = response
        self._params = params
        self._exceptions = exceptions
        self._parent = None

    @property
    def name(self) -> str:
        return self._name

    @property
    def doc_block(self) -> str | None:
        return self._doc_block

    @property
    def parent(self) -> ServiceNode | InteractionNode:
        assert self._parent is not None
        return self._parent

    @property
    def response(self) -> FunctionResponse:
        return self._response

    @property
    def params(self) -> list[FunctionParam]:
        return self._params

    @property
    def exceptions(self) -> list[FunctionException]:
        return self._exceptions

    def __repr__(self) -> str:
        return f"FunctionNode({self._name!r})"


class ServiceNode(Definition):
    """A Thrift service definition."""

    __slots__ = ("_uri", "_base_service_lazy", "_functions")
    _uri: str
    _base_service_lazy: _Lazy[ServiceNode] | None
    _functions: list[FunctionNode]

    def __init__(
        self,
        *,
        uri: str,
        base_service_lazy: _Lazy[ServiceNode] | None,
        functions: list[FunctionNode],
        **kwargs: Any,
    ) -> None:
        super().__init__(**kwargs)
        self._uri = uri
        self._base_service_lazy = base_service_lazy
        self._functions = functions
        for fn in functions:
            fn._parent = self

    @property
    def uri(self) -> str:
        return self._uri

    @property
    def base_service(self) -> ServiceNode | None:
        if self._base_service_lazy is None:
            return None
        defn = self._base_service_lazy.resolve()
        assert isinstance(defn, ServiceNode)
        return defn

    @property
    def functions(self) -> list[FunctionNode]:
        return self._functions


class InteractionNode(Definition):
    """A Thrift interaction definition."""

    __slots__ = ("_uri", "_functions")
    _uri: str
    _functions: list[FunctionNode]

    def __init__(
        self, *, uri: str, functions: list[FunctionNode], **kwargs: Any
    ) -> None:
        super().__init__(**kwargs)
        self._uri = uri
        self._functions = functions
        for fn in functions:
            fn._parent = self

    @property
    def uri(self) -> str:
        return self._uri

    @property
    def functions(self) -> list[FunctionNode]:
        return self._functions


# ---------------------------------------------------------------------------
# ProgramNode
# ---------------------------------------------------------------------------


class ProgramNode:
    """Represents a .thrift file."""

    __slots__ = (
        "_name",
        "_package",
        "_path",
        "_includes",
        "_definitions",
        "_definitions_by_name",
        "_namespaces",
    )
    _name: str
    _package: str
    _path: str
    _includes: list[ProgramNode]
    _definitions: list[Definition]
    _definitions_by_name: dict[str, Definition]
    _namespaces: dict[str, str]

    def __init__(
        self,
        *,
        name: str,
        package: str,
        path: str,
        namespaces: dict[str, str],
    ) -> None:
        self._name = name
        self._package = package
        self._path = path
        self._namespaces = namespaces
        self._includes = []
        self._definitions = []
        self._definitions_by_name = {}

    @property
    def name(self) -> str:
        return self._name

    @property
    def package(self) -> str:
        return self._package

    @property
    def path(self) -> str:
        return self._path

    @property
    def includes(self) -> list[ProgramNode]:
        return self._includes

    @property
    def definitions(self) -> list[Definition]:
        return self._definitions

    @property
    def namespaces(self) -> dict[str, str]:
        return self._namespaces

    def __getitem__(self, name: str) -> Definition:
        try:
            return self._definitions_by_name[name]
        except KeyError:
            raise KeyError(
                f"No definition {name!r} in program {self._name!r}"
            ) from None

    def __contains__(self, name: str) -> bool:
        return name in self._definitions_by_name

    def __repr__(self) -> str:
        return f"ProgramNode({self._name!r})"


# ---------------------------------------------------------------------------
# SyntaxGraph
# ---------------------------------------------------------------------------


class SyntaxGraph:
    """Root object for the syntax graph. Owns all nodes."""

    __slots__ = ("_programs", "_programs_by_name", "_programs_by_path")
    _programs: list[ProgramNode]
    _programs_by_name: dict[str, ProgramNode]
    _programs_by_path: dict[str, ProgramNode]

    def __init__(
        self,
        programs: list[ProgramNode],
        programs_by_name: dict[str, ProgramNode],
        programs_by_path: dict[str, ProgramNode],
    ) -> None:
        self._programs = programs
        self._programs_by_name = programs_by_name
        self._programs_by_path = programs_by_path

    @classmethod
    def _create_empty(cls) -> SyntaxGraph:
        """Create an empty graph for incremental building."""
        return cls([], {}, {})

    def _add_program(self, program: ProgramNode) -> None:
        """Append a program to this graph (for incremental building)."""
        if program.name in self._programs_by_name:
            raise RuntimeError(f"Duplicate program name: {program.name!r}")
        if program.path and program.path in self._programs_by_path:
            raise RuntimeError(f"Duplicate program path: {program.path!r}")
        self._programs.append(program)
        self._programs_by_name[program.name] = program
        if program.path:
            self._programs_by_path[program.path] = program

    @staticmethod
    def from_thrift_schema(schema: _schema_types.Schema) -> SyntaxGraph:
        raise NotImplementedError("TODO")

    @staticmethod
    def from_serialized_schema(
        data: bytes, protocol: Protocol = Protocol.COMPACT
    ) -> SyntaxGraph:
        """Build from serialized schema bytes."""
        schema = deserialize(_schema_types.Schema, data, protocol=protocol)
        return SyntaxGraph.from_thrift_schema(schema)

    @property
    def programs(self) -> list[ProgramNode]:
        return self._programs

    def get_program_by_name(self, name: str) -> ProgramNode:
        try:
            return self._programs_by_name[name]
        except KeyError:
            raise KeyError(f"No program named {name!r}") from None

    def get_program_by_path(self, path: str) -> ProgramNode:
        try:
            return self._programs_by_path[path]
        except KeyError:
            raise KeyError(f"No program with path {path!r}") from None

    def __repr__(self) -> str:
        return f"SyntaxGraph({len(self._programs)} programs)"
