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
Pure-Python runtime TypeSystem model for the thrift-python SchemaRegistry.

It contains:

* ``Primitive`` / ``PresenceQualifier`` -- the only discriminant enums.
* ``TypeRef`` -- a type edge, discriminated by ``isinstance`` / ``match``
  (``PrimitiveTypeRef`` / ``ListTypeRef`` / ``SetTypeRef`` / ``MapTypeRef`` /
  ``StructTypeRef`` / ``UnionTypeRef`` / ``EnumTypeRef`` / ``OpaqueAliasTypeRef``).
* ``DefinitionNode`` -- what URI lookups return (``StructNode`` / ``UnionNode`` /
  ``EnumNode`` / ``OpaqueAliasNode``), plus ``FieldDefinition`` / ``FieldIdentity``
  / ``EnumValue``.
* ``TypeSystem`` -- the (possibly unbounded) collection interface;
  ``EnumerableTypeSystem`` -- the *finitely enumerable* refinement (its
  ``get_known_uris`` is total); and ``IndexedTypeSystem`` -- the bounded,
  self-contained implementation.

Equality is *structural*: nodes by URI, ``TypeRef`` by ``TypeId``. Object
identity remains available via ``is``.
"""

from __future__ import annotations

import enum
from abc import ABC, abstractmethod
from collections.abc import Callable, Iterable, Iterator, Mapping, Sequence
from types import MappingProxyType
from typing import assert_never, Generic, TypeVar

from thrift.lib.python.schema._record import SerializableRecord


class InvalidTypeError(Exception):
    """Raised when a type system is constructed with invalid or unresolvable
    types (duplicate field ids/names, unknown URIs, illegal opaque-alias
    targets, etc.)."""


# ---------------------------------------------------------------------------
# Discriminant enums
# ---------------------------------------------------------------------------


class Primitive(enum.Enum):
    """The 10 primitive types. Values match the primitive ``TypeId`` arms 1-10
    (``thrift/lib/thrift/type_id.thrift``)."""

    BOOL = 1
    BYTE = 2
    I16 = 3
    I32 = 4
    I64 = 5
    FLOAT = 6
    DOUBLE = 7
    STRING = 8
    BINARY = 9
    ANY = 10


class PresenceQualifier(enum.IntEnum):
    """Field presence. Values match ``PresenceQualifier`` in
    ``thrift/lib/thrift/type_system.thrift``.

    Note ``TERSE`` is distinct from ``UNQUALIFIED``."""

    UNQUALIFIED = 1
    OPTIONAL = 2
    TERSE = 3


# ---------------------------------------------------------------------------
# TypeRef hierarchy -- a resolved type edge, discriminated by isinstance/match.
# Equality/hash are structural ("by TypeId"); object identity via ``is``.
# ---------------------------------------------------------------------------


class TypeRefBase:
    """Base of the resolved type-reference hierarchy.

    Equality and hashing are *structural*: two ``TypeRef``s are equal iff they
    denote the same ``TypeId`` -- primitives by kind, containers recursively,
    and user-defined edges by URI. Subclasses only implement ``_key``;
    ``__eq__`` / ``__hash__`` are inherited.
    """

    __slots__: tuple[str, ...] = ()

    def _key(self) -> tuple[object, ...]:
        raise NotImplementedError

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, TypeRefBase):
            return NotImplemented
        return self._key() == other._key()

    def __hash__(self) -> int:
        return hash(self._key())

    def __repr__(self) -> str:
        return f"{type(self).__name__}()"


class PrimitiveTypeRef(TypeRefBase):
    """A primitive type edge (bool, i32, string, ..., any)."""

    __slots__ = ("_primitive",)
    __match_args__ = ("primitive",)
    _primitive: Primitive

    def __init__(self, primitive: Primitive) -> None:
        self._primitive = primitive

    @property
    def primitive(self) -> Primitive:
        return self._primitive

    def _key(self) -> tuple[object, ...]:
        return ("primitive", self._primitive)

    def __repr__(self) -> str:
        return f"PrimitiveTypeRef({self._primitive.name})"


class ListTypeRef(TypeRefBase):
    """A ``list<T>`` type edge."""

    __slots__ = ("_element_type",)
    __match_args__ = ("element_type",)
    _element_type: TypeRefBase

    def __init__(self, element_type: TypeRefBase) -> None:
        self._element_type = element_type

    @property
    def element_type(self) -> TypeRefBase:
        return self._element_type

    def _key(self) -> tuple[object, ...]:
        return ("list", self._element_type._key())

    def __repr__(self) -> str:
        return f"ListTypeRef({self._element_type!r})"


class SetTypeRef(TypeRefBase):
    """A ``set<T>`` type edge."""

    __slots__ = ("_element_type",)
    __match_args__ = ("element_type",)
    _element_type: TypeRefBase

    def __init__(self, element_type: TypeRefBase) -> None:
        self._element_type = element_type

    @property
    def element_type(self) -> TypeRefBase:
        return self._element_type

    def _key(self) -> tuple[object, ...]:
        return ("set", self._element_type._key())

    def __repr__(self) -> str:
        return f"SetTypeRef({self._element_type!r})"


class MapTypeRef(TypeRefBase):
    """A ``map<K, V>`` type edge."""

    __slots__ = ("_key_type", "_value_type")
    __match_args__ = ("key_type", "value_type")
    _key_type: TypeRefBase
    _value_type: TypeRefBase

    def __init__(self, key_type: TypeRefBase, value_type: TypeRefBase) -> None:
        self._key_type = key_type
        self._value_type = value_type

    @property
    def key_type(self) -> TypeRefBase:
        return self._key_type

    @property
    def value_type(self) -> TypeRefBase:
        return self._value_type

    def _key(self) -> tuple[object, ...]:
        return ("map", self._key_type._key(), self._value_type._key())

    def __repr__(self) -> str:
        return f"MapTypeRef({self._key_type!r}, {self._value_type!r})"


# A user-defined type edge is generic over the concrete node it resolves to, so
# each leaf narrows ``node`` to its own ``DefinitionNode`` subtype.
_NodeT = TypeVar("_NodeT", bound="DefinitionNodeBase")


class _UserDefinedTypeRef(TypeRefBase, Generic[_NodeT]):
    """Shared implementation for the four user-defined type edges
    (``StructTypeRef`` / ``UnionTypeRef`` / ``EnumTypeRef`` /
    ``OpaqueAliasTypeRef``): each holds a resolved ``DefinitionNode`` and is keyed
    structurally by that node's URI."""

    __slots__: tuple[str, ...] = ("_node",)
    __match_args__ = ("node",)
    _node: _NodeT

    def __init__(self, node: _NodeT) -> None:
        self._node = node

    @property
    def node(self) -> _NodeT:
        return self._node

    def _key(self) -> tuple[object, ...]:
        return ("user", self._node.uri)

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self._node.uri!r})"


class StructTypeRef(_UserDefinedTypeRef["StructNode"]):
    """A user-defined struct edge; holds the resolved ``StructNode``."""

    __slots__ = ()


class UnionTypeRef(_UserDefinedTypeRef["UnionNode"]):
    """A user-defined union edge; holds the resolved ``UnionNode``."""

    __slots__ = ()


class EnumTypeRef(_UserDefinedTypeRef["EnumNode"]):
    """A user-defined enum edge; holds the resolved ``EnumNode``."""

    __slots__ = ()


class OpaqueAliasTypeRef(_UserDefinedTypeRef["OpaqueAliasNode"]):
    """A user-defined opaque-alias edge; holds the resolved ``OpaqueAliasNode``.

    Opaque aliases exist only in programmatically-built type systems (Thrift IDL
    cannot define one)."""

    __slots__ = ()


TypeRef = (
    PrimitiveTypeRef
    | ListTypeRef
    | SetTypeRef
    | MapTypeRef
    | StructTypeRef
    | UnionTypeRef
    | EnumTypeRef
    | OpaqueAliasTypeRef
)


# ---------------------------------------------------------------------------
# Field model
# ---------------------------------------------------------------------------


class FieldIdentity:
    """The (id, name) identity of a struct/union field. ``id`` is an i16."""

    __slots__ = ("_id", "_name")
    _id: int
    _name: str

    def __init__(self, id: int, name: str) -> None:
        self._id = id
        self._name = name

    @property
    def id(self) -> int:
        return self._id

    @property
    def name(self) -> str:
        return self._name

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, FieldIdentity):
            return NotImplemented
        return self._id == other._id and self._name == other._name

    def __hash__(self) -> int:
        return hash((self._id, self._name))

    def __repr__(self) -> str:
        return f"FieldIdentity({self._id}, {self._name!r})"


class FieldDefinition:
    """A field in a struct or union.

    ``custom_default`` is the field's IDL default value (``None`` if it has the
    intrinsic default), and ``annotations`` maps each retained structured
    annotation's URI to its (field-id-keyed) ``SerializableRecord`` value."""

    __slots__ = ("_identity", "_presence", "_type", "_custom_default", "_annotations")
    _identity: FieldIdentity
    _presence: PresenceQualifier
    _type: TypeRef
    _custom_default: SerializableRecord | None
    _annotations: dict[str, SerializableRecord]

    def __init__(
        self,
        *,
        identity: FieldIdentity,
        presence: PresenceQualifier,
        type: TypeRef,
        custom_default: SerializableRecord | None = None,
        annotations: Mapping[str, SerializableRecord] | None = None,
    ) -> None:
        self._identity = identity
        self._presence = presence
        self._type = type
        self._custom_default = custom_default
        self._annotations = dict(annotations) if annotations else {}

    @property
    def identity(self) -> FieldIdentity:
        return self._identity

    @property
    def presence(self) -> PresenceQualifier:
        return self._presence

    @property
    def type(self) -> TypeRef:
        return self._type

    @property
    def custom_default(self) -> SerializableRecord | None:
        return self._custom_default

    @property
    def annotations(self) -> Mapping[str, SerializableRecord]:
        return MappingProxyType(self._annotations)

    def __repr__(self) -> str:
        return (
            f"FieldDefinition({self._identity!r}, "
            f"{self._presence.name}, {self._type!r})"
        )


class EnumValue:
    """A named value in an enum (``datum`` is an i32).

    ``annotations`` maps each retained structured annotation's URI to its
    (field-id-keyed) ``SerializableRecord`` value. Equality and hashing are *by
    name + datum only* (annotations are excluded)."""

    __slots__ = ("_name", "_datum", "_annotations")
    _name: str
    _datum: int
    _annotations: dict[str, SerializableRecord]

    def __init__(
        self,
        name: str,
        datum: int,
        annotations: Mapping[str, SerializableRecord] | None = None,
    ) -> None:
        self._name = name
        self._datum = datum
        self._annotations = dict(annotations) if annotations else {}

    @property
    def name(self) -> str:
        return self._name

    @property
    def datum(self) -> int:
        return self._datum

    @property
    def annotations(self) -> Mapping[str, SerializableRecord]:
        return MappingProxyType(self._annotations)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, EnumValue):
            return NotImplemented
        return self._name == other._name and self._datum == other._datum

    def __hash__(self) -> int:
        return hash((self._name, self._datum))

    def __repr__(self) -> str:
        return f"EnumValue({self._name!r}, {self._datum})"


# ---------------------------------------------------------------------------
# Source provenance
# ---------------------------------------------------------------------------


class SourceInfo:
    """The source provenance of a user-defined type: a ``locator`` (a URI for
    the resource -- typically a .thrift file -- holding the source IDL, e.g.
    ``"file://thrift/lib/thrift/standard.thrift"``) plus the definition ``name``
    within that resource.

    It is an immutable value, equal and hashed *by* ``(locator, name)``. A
    node's ``source_info`` is deliberately **excluded** from the node's
    ``__eq__``/``__hash__`` (nodes remain equal-by-URI) and from the
    ``TypeSystemDigest``."""

    __slots__ = ("_locator", "_name")
    _locator: str
    _name: str

    def __init__(self, locator: str, name: str) -> None:
        self._locator = locator
        self._name = name

    @property
    def locator(self) -> str:
        return self._locator

    @property
    def name(self) -> str:
        return self._name

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SourceInfo):
            return NotImplemented
        return self._locator == other._locator and self._name == other._name

    def __hash__(self) -> int:
        return hash((self._locator, self._name))

    def __repr__(self) -> str:
        return f"SourceInfo({self._locator!r}, {self._name!r})"


# ---------------------------------------------------------------------------
# DefinitionNode hierarchy -- what URI lookups return. Equality/hash by URI.
# ---------------------------------------------------------------------------


class DefinitionNodeBase:
    """Base of the four user-defined node kinds (struct / union / enum /
    opaque-alias). Equality and hashing are *by URI* -- two distinct node
    objects with the same URI compare equal (``is`` still distinguishes them)."""

    __slots__: tuple[str, ...] = ("_uri", "_annotations", "_source_info")
    _uri: str
    _annotations: dict[str, SerializableRecord]
    _source_info: SourceInfo | None

    def __init__(
        self,
        *,
        uri: str,
        annotations: Mapping[str, SerializableRecord] | None = None,
        source_info: SourceInfo | None = None,
    ) -> None:
        self._uri = uri
        self._annotations = dict(annotations) if annotations else {}
        self._source_info = source_info

    def _set_annotations(self, annotations: Mapping[str, SerializableRecord]) -> None:
        """Populate this node's annotations (used by the builder/bridge during
        the two-phase build, after the placeholder node is created)."""
        self._annotations = dict(annotations)

    def _set_source_info(self, source_info: SourceInfo | None) -> None:
        """Populate this node's source provenance (used by the builder/bridge
        and the wire importer during the two-phase build, after the placeholder
        node is created)."""
        self._source_info = source_info

    @property
    def uri(self) -> str:
        return self._uri

    @property
    def annotations(self) -> Mapping[str, SerializableRecord]:
        return MappingProxyType(self._annotations)

    @property
    def source_info(self) -> SourceInfo | None:
        return self._source_info

    def __eq__(self, other: object) -> bool:
        # A Thrift URI names exactly one definition, so a struct and an enum can
        # never share a URI in a valid type system. Also, IndexedTypeSystem is
        # URI-keyed, so it can hold only one node per URI anyway.
        if not isinstance(other, DefinitionNodeBase):
            return NotImplemented
        return self._uri == other._uri

    def __hash__(self) -> int:
        return hash(self._uri)

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self._uri!r})"


class _StructuredNode(DefinitionNodeBase):
    """Shared implementation for ``StructNode`` and ``UnionNode`` -- a list of
    fields plus precomputed ``by_id`` / ``by_name`` indexes."""

    __slots__: tuple[str, ...] = ("_fields", "_by_id", "_by_name", "_is_sealed")
    _fields: tuple[FieldDefinition, ...]
    _by_id: dict[int, FieldDefinition]
    _by_name: dict[str, FieldDefinition]
    _is_sealed: bool

    def __init__(
        self,
        *,
        uri: str,
        fields: Sequence[FieldDefinition] | None = None,
        is_sealed: bool = False,
        annotations: Mapping[str, SerializableRecord] | None = None,
        source_info: SourceInfo | None = None,
    ) -> None:
        super().__init__(uri=uri, annotations=annotations, source_info=source_info)
        self._is_sealed = is_sealed
        self._fields = ()
        self._by_id = {}
        self._by_name = {}
        if fields is not None:
            self._set_fields(fields)

    def _set_fields(self, fields: Sequence[FieldDefinition]) -> None:
        """Populate fields and (re)build the precomputed indexes. Used by the
        builder/bridge during the two-phase build."""
        self._fields = tuple(fields)
        self._by_id = {f.identity.id: f for f in self._fields}
        self._by_name = {f.identity.name: f for f in self._fields}

    @property
    def fields(self) -> tuple[FieldDefinition, ...]:
        return self._fields

    @property
    def is_sealed(self) -> bool:
        return self._is_sealed

    def field_by_id(self, field_id: int) -> FieldDefinition | None:
        return self._by_id.get(field_id)

    def field_by_name(self, name: str) -> FieldDefinition | None:
        return self._by_name.get(name)


class StructNode(_StructuredNode):
    """A struct."""

    __slots__ = ()


class UnionNode(_StructuredNode):
    """A union -- same shape as ``StructNode``, but every field must be
    ``OPTIONAL``; a non-optional field raises ``InvalidTypeError``."""

    __slots__ = ()

    def _set_fields(self, fields: Sequence[FieldDefinition]) -> None:
        for field in fields:
            if field.presence != PresenceQualifier.OPTIONAL:
                raise InvalidTypeError(
                    f"Union {self.uri!r} field {field.identity.name!r} must be "
                    f"OPTIONAL, not {field.presence.name}"
                )
        super()._set_fields(fields)


class EnumNode(DefinitionNodeBase):
    """An enum: an ordered list of ``EnumValue``."""

    __slots__ = ("_values",)
    _values: tuple[EnumValue, ...]

    def __init__(
        self,
        *,
        uri: str,
        values: Sequence[EnumValue] | None = None,
        annotations: Mapping[str, SerializableRecord] | None = None,
        source_info: SourceInfo | None = None,
    ) -> None:
        super().__init__(uri=uri, annotations=annotations, source_info=source_info)
        self._values = ()
        if values is not None:
            self._set_values(values)

    def _set_values(self, values: Sequence[EnumValue]) -> None:
        self._values = tuple(values)

    @property
    def values(self) -> tuple[EnumValue, ...]:
        return self._values


class OpaqueAliasNode(DefinitionNodeBase):
    """A programmatic opaque alias to a non-user-defined target type
    (primitive or container)."""

    __slots__ = ("_target_type",)
    _target_type: TypeRef | None

    def __init__(
        self,
        *,
        uri: str,
        target_type: TypeRef | None = None,
        annotations: Mapping[str, SerializableRecord] | None = None,
        source_info: SourceInfo | None = None,
    ) -> None:
        super().__init__(uri=uri, annotations=annotations, source_info=source_info)
        self._target_type = None
        if target_type is not None:
            self._set_target_type(target_type)

    def _set_target_type(self, target_type: TypeRef) -> None:
        self._target_type = target_type

    @property
    def target_type(self) -> TypeRef:
        assert self._target_type is not None, "opaque alias target not populated"
        return self._target_type


# ---------------------------------------------------------------------------
# Source-identifier index (locator -> {name -> node}).
# ---------------------------------------------------------------------------


def _index_by_source(
    nodes: Iterable[DefinitionNode],
) -> dict[str, dict[str, DefinitionNode]]:
    """Index user-defined nodes by their source provenance as
    ``{locator: {name: node}}``. Nodes without ``source_info`` are skipped.

    Raises ``InvalidTypeError`` if two nodes share one ``(locator, name)``
    source identifier."""
    index: dict[str, dict[str, DefinitionNode]] = {}
    for node in nodes:
        info = node.source_info
        if info is None:
            continue
        by_name = index.setdefault(info.locator, {})
        if info.name in by_name:
            raise InvalidTypeError(
                f"Duplicate source identifier name {info.name!r} at location "
                f"{info.locator!r}"
            )
        by_name[info.name] = node
    return index


# ---------------------------------------------------------------------------
# TypeSystem -- a (possibly bounded) collection of user-defined types.
# ---------------------------------------------------------------------------


class TypeSystem(ABC):
    """Interface for a (possibly unbounded) collection of user-defined types,
    keyed by URI. The lazy registry is an *unbounded* implementation."""

    __slots__: tuple[str, ...] = ()

    @abstractmethod
    def get_user_defined_type(self, uri: str) -> DefinitionNode | None:
        """Resolve a URI to its ``DefinitionNode``, or ``None`` if unknown."""
        raise NotImplementedError

    def get_user_defined_type_or_throw(self, uri: str) -> DefinitionNode:
        """Like ``get_user_defined_type`` but raises ``InvalidTypeError`` on a
        miss."""
        node = self.get_user_defined_type(uri)
        if node is None:
            raise InvalidTypeError(f"Unknown user-defined type URI: {uri!r}")
        return node

    @abstractmethod
    def get_known_uris(self) -> frozenset[str] | None:
        """The exact set of known URIs, or ``None`` when the type system is not
        finitely enumerable (e.g. the lazy registry)."""
        raise NotImplementedError

    @abstractmethod
    def get_user_defined_type_by_source_identifier(
        self, locator: str, name: str
    ) -> DefinitionNode | None:
        """Resolve a source identifier ``(locator, name)`` to its
        ``DefinitionNode``, or ``None`` on a miss.

        Unlike URIs, source identifiers are not globally unique, but they
        are unique *within* a single type system. An implementation that
        cannot enumerate its types (e.g. the lazy registry/bridge) returns
        ``None``."""
        raise NotImplementedError

    @abstractmethod
    def get_user_defined_types_at_location(
        self, locator: str
    ) -> Mapping[str, DefinitionNode]:
        """All user-defined types whose source ``locator`` matches, as a
        ``{name: node}`` mapping (empty for an unknown locator).

        For every name in the result,
        ``get_user_defined_type_by_source_identifier`` with that
        ``(locator, name)`` must also succeed. An implementation that cannot
        enumerate its types (e.g. the lazy registry/bridge) returns an empty
        mapping."""
        raise NotImplementedError


class EnumerableTypeSystem(TypeSystem):
    """A ``TypeSystem`` whose types are *finitely enumerable*: ``get_known_uris``
    is total and never returns ``None``."""

    __slots__: tuple[str, ...] = ()

    @abstractmethod
    def get_known_uris(self) -> frozenset[str]:
        """The exact, finite set of known URIs (never ``None``)."""
        raise NotImplementedError


class IndexedTypeSystem(EnumerableTypeSystem):
    """A concrete, bounded, self-contained ``{uri: DefinitionNode}`` collection
    and the canonical ``EnumerableTypeSystem`` implementation;
    ``get_known_uris`` returns the exact set."""

    __slots__ = ("_by_uri", "_source_index")
    _by_uri: dict[str, DefinitionNode]
    # Lazily built source index: ``{locator: {name: node}}``. ``None`` until the
    # first source-identifier lookup (most type systems are never queried this
    # way, so the index is not paid for at construction).
    _source_index: dict[str, dict[str, DefinitionNode]] | None

    def __init__(self, definitions: dict[str, DefinitionNode]) -> None:
        for uri, node in definitions.items():
            if node.uri != uri:
                raise InvalidTypeError(
                    f"IndexedTypeSystem key {uri!r} does not match node uri "
                    f"{node.uri!r}"
                )
        self._by_uri = dict(definitions)
        self._source_index = None

    def get_user_defined_type(self, uri: str) -> DefinitionNode | None:
        return self._by_uri.get(uri)

    def get_known_uris(self) -> frozenset[str]:
        return frozenset(self._by_uri)

    def _ensure_source_index(self) -> dict[str, dict[str, DefinitionNode]]:
        index = self._source_index
        if index is None:
            index = _index_by_source(self._by_uri.values())
            self._source_index = index
        return index

    def get_user_defined_type_by_source_identifier(
        self, locator: str, name: str
    ) -> DefinitionNode | None:
        return self._ensure_source_index().get(locator, {}).get(name)

    def get_user_defined_types_at_location(
        self, locator: str
    ) -> Mapping[str, DefinitionNode]:
        return MappingProxyType(self._ensure_source_index().get(locator, {}))

    def __repr__(self) -> str:
        return f"IndexedTypeSystem({len(self._by_uri)} types)"


DefinitionNode = StructNode | UnionNode | EnumNode | OpaqueAliasNode


# ---------------------------------------------------------------------------
# Shared model traversals (used by both the builder and the wire-export layer).
# These operate purely on the model above and introduce no wire-type imports.
# ---------------------------------------------------------------------------


def _type_ref_for_node(node: DefinitionNode) -> TypeRef:
    """Wrap a resolved ``DefinitionNode`` in the matching user-defined ``TypeRef``
    (``StructTypeRef`` / ``UnionTypeRef`` / ``EnumTypeRef`` /
    ``OpaqueAliasTypeRef``)."""
    match node:
        case StructNode():
            return StructTypeRef(node)
        case UnionNode():
            return UnionTypeRef(node)
        case EnumNode():
            return EnumTypeRef(node)
        case OpaqueAliasNode():
            return OpaqueAliasTypeRef(node)
        case _:
            assert_never(node)


def _type_ref_uris(type_ref: TypeRefBase) -> Iterator[str]:
    """The user-defined URIs reachable from a ``TypeRef``, recursing into
    containers (primitives reference nothing)."""
    if not isinstance(type_ref, TypeRef):
        raise InvalidTypeError(
            f"{type(type_ref).__name__} is not a resolved type edge (TypeRef)."
        )
    match type_ref:
        case ListTypeRef() | SetTypeRef():
            yield from _type_ref_uris(type_ref.element_type)
        case MapTypeRef():
            yield from _type_ref_uris(type_ref.key_type)
            yield from _type_ref_uris(type_ref.value_type)
        case StructTypeRef() | UnionTypeRef() | EnumTypeRef() | OpaqueAliasTypeRef():
            yield type_ref.node.uri
        case PrimitiveTypeRef():
            pass  # primitives reference nothing
        case _:
            assert_never(type_ref)


def _collect_closure(
    source: TypeSystem,
    root_uris: Sequence[str],
    edges: Callable[[DefinitionNode], Iterator[str]],
) -> dict[str, DefinitionNode]:
    """DFS the closure of ``root_uris`` over ``source``, following ``edges(node)``
    out of each resolved node. Returns a ``{uri: source node}`` map.

    Raises ``InvalidTypeError`` if a root URI is absent, or if a referenced
    (non-root) URI fails to resolve (i.e. the closure is not self-contained)."""
    roots = set(root_uris)
    closure: dict[str, DefinitionNode] = {}
    worklist: list[str] = list(root_uris)
    while worklist:
        uri = worklist.pop()
        if uri in closure:
            continue
        node = source.get_user_defined_type(uri)
        if node is None:
            if uri in roots:
                raise InvalidTypeError(f"Root URI not found in source: {uri!r}")
            raise InvalidTypeError(
                f"Source type system is not self-contained: referenced URI "
                f"{uri!r} is not resolvable"
            )
        closure[uri] = node
        worklist.extend(edges(node))
    return closure


class PruneOptions:
    """Options for ``build_pruned`` (``type_system_builder``) and
    ``build_serializable_type_system`` (``_serializable``)."""

    __slots__ = ("_include_source_info",)
    _include_source_info: bool

    def __init__(self, include_source_info: bool = True) -> None:
        self._include_source_info = include_source_info

    @property
    def include_source_info(self) -> bool:
        return self._include_source_info

    def __repr__(self) -> str:
        return f"PruneOptions(include_source_info={self._include_source_info})"
