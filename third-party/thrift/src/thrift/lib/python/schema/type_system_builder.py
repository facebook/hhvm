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
Programmatic, two-phase construction of a bounded ``EnumerableTypeSystem``
(concretely an ``IndexedTypeSystem``).

``TypeSystemBuilder`` lets callers hand-pick arbitrary definitions
(``add_struct`` / ``add_union`` / ``add_enum`` / ``add_opaque_alias``) and then
``build()`` a bounded, self-contained type system -- with no ``.thrift`` source,
no ``SyntaxGraph``, and no ``sourceInfo``.

Field/element types are supplied either as resolved primitive/container
``TypeRef``s or, for user-defined types, by URI. A bare ``str`` or a :func:`ref`
marker stands in for a user-defined reference and may be nested inside containers
(e.g. ``ListTypeRef(ref("uri/Foo"))``). The build runs in two phases so that
cyclic and mutually-recursive types resolve against stable node identities:

1. placeholders: one empty node per URI, registered before any field is
   populated
2. populate: each field/element/target ``TypeRef`` is resolved against the
   phase-1 map, with validators raising ``InvalidTypeError``
"""

from __future__ import annotations

from collections.abc import Iterator
from typing import assert_never, Sequence

from thrift.lib.python.schema.type_system import (
    DefinitionNode,
    EnumerableTypeSystem,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    FieldDefinition,
    FieldIdentity,
    IndexedTypeSystem,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    PresenceQualifier,
    PrimitiveTypeRef,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
    TypeSystem,
    UnionNode,
    UnionTypeRef,
)


# ---------------------------------------------------------------------------
# Builder input: unresolved type specifications.
# ---------------------------------------------------------------------------


class UnresolvedTypeRef(TypeRef):
    """Builder-input marker: an unresolved reference to a user-defined type by
    URI, resolved to a node-holding ``TypeRef`` during ``build()``.

    It is a ``TypeRef`` subclass purely so it can nest inside container input
    (``ListTypeRef`` / ``SetTypeRef`` / ``MapTypeRef``); it is **never** part of
    a built ``TypeSystem`` and is deliberately excluded from ``TypeRefT``."""

    __slots__ = ("_uri",)
    _uri: str

    def __init__(self, uri: str) -> None:
        self._uri = uri

    @property
    def uri(self) -> str:
        return self._uri

    def _key(self) -> tuple[object, ...]:
        return ("unresolved", self._uri)

    def __repr__(self) -> str:
        return f"UnresolvedTypeRef({self._uri!r})"


# A type edge supplied to the builder: a resolved primitive/container ``TypeRef``
# (which may contain ``UnresolvedTypeRef`` markers for nested user types), or a
# ``str`` URI referencing a user-defined type.
TypeInput = TypeRef | str


def ref(uri: str) -> UnresolvedTypeRef:
    """Reference a user-defined type by URI in builder input -- including when
    nested in a container, e.g. ``ListTypeRef(ref("uri/Foo"))``."""
    return UnresolvedTypeRef(uri)


class FieldSpec:
    """Builder input for a struct/union field.

    The ``type`` is a :data:`TypeInput`; it is resolved to a concrete
    ``TypeRef`` during ``build()``."""

    __slots__ = ("_identity", "_presence", "_type")
    _identity: FieldIdentity
    _presence: PresenceQualifier
    _type: TypeInput

    def __init__(
        self,
        identity: FieldIdentity,
        presence: PresenceQualifier,
        type: TypeInput,
    ) -> None:
        self._identity = identity
        self._presence = presence
        self._type = type

    @property
    def identity(self) -> FieldIdentity:
        return self._identity

    @property
    def presence(self) -> PresenceQualifier:
        return self._presence

    @property
    def type(self) -> TypeInput:
        return self._type

    def __repr__(self) -> str:
        return f"FieldSpec({self._identity!r}, {self._presence.name}, {self._type!r})"


# ---------------------------------------------------------------------------
# Internal pending records (one per add_*; resolved at build()).
# ---------------------------------------------------------------------------


class _PendingStructured:
    __slots__ = ("uri", "fields", "is_union", "is_sealed")
    uri: str
    fields: list[FieldSpec]
    is_union: bool
    is_sealed: bool

    def __init__(
        self, uri: str, fields: list[FieldSpec], is_union: bool, is_sealed: bool
    ) -> None:
        self.uri = uri
        self.fields = fields
        self.is_union = is_union
        self.is_sealed = is_sealed


class _PendingEnum:
    __slots__ = ("uri", "values")
    uri: str
    values: list[EnumValue]

    def __init__(self, uri: str, values: list[EnumValue]) -> None:
        self.uri = uri
        self.values = values


class _PendingOpaqueAlias:
    __slots__ = ("uri", "target")
    uri: str
    target: TypeInput

    def __init__(self, uri: str, target: TypeInput) -> None:
        self.uri = uri
        self.target = target


_Pending = _PendingStructured | _PendingEnum | _PendingOpaqueAlias


# ---------------------------------------------------------------------------
# Type resolution (phase 2 helpers)
# ---------------------------------------------------------------------------


def _type_ref_for_node(node: DefinitionNode) -> TypeRef:
    """Wrap a resolved ``DefinitionNode`` in the matching user-defined
    ``TypeRef`` (``StructTypeRef`` / ``UnionTypeRef`` / ``EnumTypeRef`` /
    ``OpaqueAliasTypeRef``)."""
    if isinstance(node, StructNode):
        return StructTypeRef(node)
    if isinstance(node, UnionNode):
        return UnionTypeRef(node)
    if isinstance(node, EnumNode):
        return EnumTypeRef(node)
    if isinstance(node, OpaqueAliasNode):
        return OpaqueAliasTypeRef(node)
    raise InvalidTypeError(f"Unknown definition kind: {node!r}")


def _resolve_uri(uri: str, nodes: dict[str, DefinitionNode]) -> TypeRef:
    """Resolve a user-defined URI against the phase-1 node map to a node-holding
    ``TypeRef``."""
    node = nodes.get(uri)
    if node is None:
        raise InvalidTypeError(f"Unresolved type URI: {uri!r}")
    return _type_ref_for_node(node)


def _resolve(spec: TypeInput, nodes: dict[str, DefinitionNode]) -> TypeRef:
    """Recursively resolve a builder ``TypeInput`` into a concrete ``TypeRef``.

    Primitives pass through, containers recurse, and user references (bare
    ``str`` or :class:`UnresolvedTypeRef`) resolve against ``nodes``. Passing an
    already node-holding ``TypeRef`` is rejected: user types must be referenced
    by URI."""
    if isinstance(spec, str):
        return _resolve_uri(spec, nodes)
    if isinstance(spec, UnresolvedTypeRef):
        return _resolve_uri(spec.uri, nodes)
    if isinstance(spec, PrimitiveTypeRef):
        return spec
    if isinstance(spec, ListTypeRef):
        return ListTypeRef(_resolve(spec.element_type, nodes))
    if isinstance(spec, SetTypeRef):
        return SetTypeRef(_resolve(spec.element_type, nodes))
    if isinstance(spec, MapTypeRef):
        return MapTypeRef(
            _resolve(spec.key_type, nodes), _resolve(spec.value_type, nodes)
        )
    if isinstance(spec, (StructTypeRef, UnionTypeRef, EnumTypeRef, OpaqueAliasTypeRef)):
        raise InvalidTypeError("reference user-defined types by URI, not by node")
    raise InvalidTypeError(f"Unsupported type input: {spec!r}")


def _is_user_defined(type_ref: TypeRef) -> bool:
    return isinstance(
        type_ref,
        (StructTypeRef, UnionTypeRef, EnumTypeRef, OpaqueAliasTypeRef),
    )


# ---------------------------------------------------------------------------
# TypeSystemBuilder
# ---------------------------------------------------------------------------


class TypeSystemBuilder:
    """Programmatic builder for a bounded, self-contained ``EnumerableTypeSystem``
    (concretely an ``IndexedTypeSystem``).

    ``add_*`` methods register definitions (returning ``self`` for chaining);
    ``build()`` runs the two-phase resolution and validators."""

    __slots__ = ("_pending", "_uris")
    _pending: list[_Pending]
    _uris: set[str]

    def __init__(self) -> None:
        self._pending = []
        self._uris = set()

    def _reserve_uri(self, uri: str) -> None:
        if not uri:
            raise InvalidTypeError("user-defined type URI must be non-empty")
        if uri in self._uris:
            raise InvalidTypeError(f"Duplicate definition URI: {uri!r}")
        self._uris.add(uri)

    def add_struct(
        self,
        uri: str,
        fields: Sequence[FieldSpec],
        *,
        is_sealed: bool = False,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(
            _PendingStructured(uri, list(fields), is_union=False, is_sealed=is_sealed)
        )
        return self

    def add_union(
        self,
        uri: str,
        fields: Sequence[FieldSpec],
        *,
        is_sealed: bool = False,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(
            _PendingStructured(uri, list(fields), is_union=True, is_sealed=is_sealed)
        )
        return self

    def add_enum(self, uri: str, values: Sequence[EnumValue]) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(_PendingEnum(uri, list(values)))
        return self

    def add_opaque_alias(self, uri: str, target: TypeInput) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(_PendingOpaqueAlias(uri, target))
        return self

    def build(self) -> EnumerableTypeSystem:
        nodes = self._build_placeholders()
        self._populate(nodes)
        return IndexedTypeSystem(nodes)

    def _build_placeholders(self) -> dict[str, DefinitionNode]:
        """Phase 1: allocate one empty node per URI with stable identity."""
        nodes: dict[str, DefinitionNode] = {}
        for pending in self._pending:
            if isinstance(pending, _PendingStructured):
                nodes[pending.uri] = (
                    UnionNode(uri=pending.uri, is_sealed=pending.is_sealed)
                    if pending.is_union
                    else StructNode(uri=pending.uri, is_sealed=pending.is_sealed)
                )
            elif isinstance(pending, _PendingEnum):
                nodes[pending.uri] = EnumNode(uri=pending.uri)
            elif isinstance(pending, _PendingOpaqueAlias):
                nodes[pending.uri] = OpaqueAliasNode(uri=pending.uri)
            else:
                assert_never(pending)
        return nodes

    def _populate(self, nodes: dict[str, DefinitionNode]) -> None:
        """Phase 2: resolve and validate every node's contents."""
        for pending in self._pending:
            if isinstance(pending, _PendingStructured):
                node = nodes[pending.uri]
                assert isinstance(node, (StructNode, UnionNode))
                node._set_fields(_build_fields(pending, nodes))
            elif isinstance(pending, _PendingEnum):
                enum_node = nodes[pending.uri]
                assert isinstance(enum_node, EnumNode)
                enum_node._set_values(_build_enum_values(pending))
            elif isinstance(pending, _PendingOpaqueAlias):
                alias_node = nodes[pending.uri]
                assert isinstance(alias_node, OpaqueAliasNode)
                target = _resolve(pending.target, nodes)
                if _is_user_defined(target):
                    raise InvalidTypeError(
                        f"opaque alias {pending.uri!r} target must not be "
                        "user-defined (got a struct/union/enum/opaque-alias)"
                    )
                alias_node._set_target_type(target)
            else:
                assert_never(pending)


def _build_fields(
    pending: _PendingStructured, nodes: dict[str, DefinitionNode]
) -> list[FieldDefinition]:
    """Resolve a structured node's fields, rejecting duplicate ids/names and
    forcing union fields optional."""
    result: list[FieldDefinition] = []
    seen_ids: set[int] = set()
    seen_names: set[str] = set()
    for spec in pending.fields:
        field_id = spec.identity.id
        field_name = spec.identity.name
        if field_id in seen_ids:
            raise InvalidTypeError(f"Duplicate field id {field_id} in {pending.uri!r}")
        if field_name in seen_names:
            raise InvalidTypeError(
                f"Duplicate field name {field_name!r} in {pending.uri!r}"
            )
        seen_ids.add(field_id)
        seen_names.add(field_name)
        presence = PresenceQualifier.OPTIONAL if pending.is_union else spec.presence
        result.append(
            FieldDefinition(
                identity=spec.identity,
                presence=presence,
                type=_resolve(spec.type, nodes),
            )
        )
    return result


def _build_enum_values(pending: _PendingEnum) -> list[EnumValue]:
    """Validate an enum's values, rejecting duplicate names and datums."""
    seen_names: set[str] = set()
    seen_datums: set[int] = set()
    for value in pending.values:
        if value.name in seen_names:
            raise InvalidTypeError(
                f"Duplicate enum value name {value.name!r} in {pending.uri!r}"
            )
        if value.datum in seen_datums:
            raise InvalidTypeError(
                f"Duplicate enum value datum {value.datum} in {pending.uri!r}"
            )
        seen_names.add(value.name)
        seen_datums.add(value.datum)
    return list(pending.values)


# ---------------------------------------------------------------------------
# Pruning: extract a bounded, self-contained TypeSystem from a larger source.
# ---------------------------------------------------------------------------


class PruneOptions:
    """Options for :func:`build_pruned`.

    ``include_source_info`` is a forward-looking toggle: the current model
    carries no ``sourceInfo``, so it currently has no effect on the pruned type
    structure. It is accepted now so the signature is stable across milestones.
    """

    __slots__ = ("_include_source_info",)
    _include_source_info: bool

    def __init__(self, include_source_info: bool = True) -> None:
        self._include_source_info = include_source_info

    @property
    def include_source_info(self) -> bool:
        return self._include_source_info

    def __repr__(self) -> str:
        return f"PruneOptions(include_source_info={self._include_source_info})"


def build_pruned(
    source: TypeSystem,
    root_uris: Sequence[str],
    options: PruneOptions | None = None,
    *,
    share: bool = False,
) -> EnumerableTypeSystem:
    """Extract a bounded, self-contained ``EnumerableTypeSystem`` holding exactly
    ``root_uris`` plus their transitive type-structure closure.

    ``source`` may be any ``TypeSystem`` -- including the lazy ``SchemaRegistry``
    view or another ``IndexedTypeSystem``.

    By default the result is **independent**: every node is deep-copied and the
    graph is rewired to the copies, so dropping or mutating ``source`` leaves the
    pruned type system intact. Pass ``share=True`` for a fast path that reuses
    ``source``'s memoized nodes (the result's lifetime is then tied to
    ``source``).

    Raises ``InvalidTypeError`` if a root URI is absent from ``source``, or if
    ``source`` is not self-contained (a referenced URI fails to resolve).
    """
    closure = _collect_closure(source, root_uris)
    if share:
        return IndexedTypeSystem(dict(closure))
    return IndexedTypeSystem(_copy_closure(closure))


def _collect_closure(
    source: TypeSystem, root_uris: Sequence[str]
) -> dict[str, DefinitionNode]:
    """DFS the dependency closure of ``root_uris`` over ``source``, returning a
    ``{uri: source node}`` map."""
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
        worklist.extend(_referenced_uris(node))
    return closure


def _referenced_uris(node: DefinitionNode) -> Iterator[str]:
    """The user-defined URIs ``node`` references through its type structure
    (enum nodes reference nothing)."""
    if isinstance(node, (StructNode, UnionNode)):
        for field in node.fields:
            yield from _type_ref_uris(field.type)
    elif isinstance(node, OpaqueAliasNode):
        yield from _type_ref_uris(node.target_type)


def _type_ref_uris(type_ref: TypeRef) -> Iterator[str]:
    """The user-defined URIs reachable from a ``TypeRef``, recursing into
    containers (primitives reference nothing)."""
    if isinstance(type_ref, (ListTypeRef, SetTypeRef)):
        yield from _type_ref_uris(type_ref.element_type)
    elif isinstance(type_ref, MapTypeRef):
        yield from _type_ref_uris(type_ref.key_type)
        yield from _type_ref_uris(type_ref.value_type)
    elif isinstance(
        type_ref, (StructTypeRef, UnionTypeRef, EnumTypeRef, OpaqueAliasTypeRef)
    ):
        yield type_ref.node.uri


def _copy_closure(closure: dict[str, DefinitionNode]) -> dict[str, DefinitionNode]:
    """Deep-copy a resolved closure into fresh, independent nodes, rewiring every
    edge to the copies. Two-phase (placeholder then populate) so cyclic and
    mutually-recursive types resolve against stable node identities."""
    new_nodes: dict[str, DefinitionNode] = {
        uri: _empty_copy(node) for uri, node in closure.items()
    }
    for uri, node in closure.items():
        _populate_copy(node, new_nodes[uri], new_nodes)
    return new_nodes


def _empty_copy(node: DefinitionNode) -> DefinitionNode:
    """Phase 1: an empty same-kind node carrying ``node``'s URI identity."""
    if isinstance(node, UnionNode):
        return UnionNode(uri=node.uri, is_sealed=node.is_sealed)
    if isinstance(node, StructNode):
        return StructNode(uri=node.uri, is_sealed=node.is_sealed)
    if isinstance(node, EnumNode):
        return EnumNode(uri=node.uri)
    if isinstance(node, OpaqueAliasNode):
        return OpaqueAliasNode(uri=node.uri)
    raise InvalidTypeError(f"Cannot copy definition node: {node!r}")


def _populate_copy(
    src: DefinitionNode,
    dst: DefinitionNode,
    new_nodes: dict[str, DefinitionNode],
) -> None:
    """Phase 2: copy ``src``'s contents into ``dst``, rewiring edges to the
    copied nodes in ``new_nodes``."""
    if isinstance(src, (StructNode, UnionNode)):
        assert isinstance(dst, (StructNode, UnionNode))
        dst._set_fields([_copy_field(f, new_nodes) for f in src.fields])
    elif isinstance(src, EnumNode):
        assert isinstance(dst, EnumNode)
        dst._set_values([EnumValue(v.name, v.datum) for v in src.values])
    elif isinstance(src, OpaqueAliasNode):
        assert isinstance(dst, OpaqueAliasNode)
        dst._set_target_type(_copy_type_ref(src.target_type, new_nodes))


def _copy_field(
    field: FieldDefinition, new_nodes: dict[str, DefinitionNode]
) -> FieldDefinition:
    return FieldDefinition(
        identity=FieldIdentity(field.identity.id, field.identity.name),
        presence=field.presence,
        type=_copy_type_ref(field.type, new_nodes),
    )


def _copy_type_ref(type_ref: TypeRef, new_nodes: dict[str, DefinitionNode]) -> TypeRef:
    """Deep-copy a ``TypeRef``, rewiring user-defined edges to ``new_nodes``."""
    if isinstance(type_ref, PrimitiveTypeRef):
        return PrimitiveTypeRef(type_ref.primitive)
    if isinstance(type_ref, ListTypeRef):
        return ListTypeRef(_copy_type_ref(type_ref.element_type, new_nodes))
    if isinstance(type_ref, SetTypeRef):
        return SetTypeRef(_copy_type_ref(type_ref.element_type, new_nodes))
    if isinstance(type_ref, MapTypeRef):
        return MapTypeRef(
            _copy_type_ref(type_ref.key_type, new_nodes),
            _copy_type_ref(type_ref.value_type, new_nodes),
        )
    if isinstance(
        type_ref, (StructTypeRef, UnionTypeRef, EnumTypeRef, OpaqueAliasTypeRef)
    ):
        return _type_ref_for_node(new_nodes[type_ref.node.uri])
    raise InvalidTypeError(f"Cannot copy TypeRef: {type_ref!r}")
