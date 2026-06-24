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

from collections.abc import Iterator, Mapping
from types import MappingProxyType
from typing import assert_never, Sequence

from thrift.lib.python.schema.type_system import (
    _collect_closure,
    _index_by_source,
    _type_ref_for_node,
    _type_ref_uris,
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
    PruneOptions,
    SetTypeRef,
    SourceInfo,
    StructNode,
    StructTypeRef,
    TypeRef,
    TypeRefBase,
    TypeSystem,
    UnionNode,
    UnionTypeRef,
)


# ---------------------------------------------------------------------------
# Builder input: unresolved type specifications.
# ---------------------------------------------------------------------------


class UnresolvedTypeRef(TypeRefBase):
    """Builder-input marker: an unresolved reference to a user-defined type by
    URI, resolved to a node-holding ``TypeRef`` during ``build()``.

    It is a ``TypeRef`` subclass purely so it can nest inside container input
    (``ListTypeRef`` / ``SetTypeRef`` / ``MapTypeRef``); it is **never** part of
    a built ``TypeSystem`` (which holds only resolved edges)."""

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
TypeInput = TypeRefBase | str


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
    __slots__ = ("uri", "fields", "is_union", "is_sealed", "source_info")
    uri: str
    fields: list[FieldSpec]
    is_union: bool
    is_sealed: bool
    source_info: SourceInfo | None

    def __init__(
        self,
        uri: str,
        fields: list[FieldSpec],
        is_union: bool,
        is_sealed: bool,
        source_info: SourceInfo | None = None,
    ) -> None:
        self.uri = uri
        self.fields = fields
        self.is_union = is_union
        self.is_sealed = is_sealed
        self.source_info = source_info


class _PendingEnum:
    __slots__ = ("uri", "values", "source_info")
    uri: str
    values: list[EnumValue]
    source_info: SourceInfo | None

    def __init__(
        self,
        uri: str,
        values: list[EnumValue],
        source_info: SourceInfo | None = None,
    ) -> None:
        self.uri = uri
        self.values = values
        self.source_info = source_info


class _PendingOpaqueAlias:
    __slots__ = ("uri", "target", "source_info")
    uri: str
    target: TypeInput
    source_info: SourceInfo | None

    def __init__(
        self,
        uri: str,
        target: TypeInput,
        source_info: SourceInfo | None = None,
    ) -> None:
        self.uri = uri
        self.target = target
        self.source_info = source_info


_Pending = _PendingStructured | _PendingEnum | _PendingOpaqueAlias


# ---------------------------------------------------------------------------
# Type resolution (phase 2 helpers)
# ---------------------------------------------------------------------------


def _resolve_uri(
    uri: str,
    nodes: dict[str, DefinitionNode],
    base: TypeSystem | None = None,
) -> TypeRef:
    """Resolve a user-defined URI against the phase-1 node map (falling back to
    ``base`` for an overlay build) to a node-holding ``TypeRef``."""
    node = nodes.get(uri)
    if node is None and base is not None:
        node = base.get_user_defined_type(uri)
    if node is None:
        raise InvalidTypeError(f"Unresolved type URI: {uri!r}")
    return _type_ref_for_node(node)


def _resolve(
    spec: TypeInput,
    nodes: dict[str, DefinitionNode],
    base: TypeSystem | None = None,
) -> TypeRef:
    """Recursively resolve a builder ``TypeInput`` into a concrete ``TypeRef``.

    Primitives pass through, containers recurse, and user references (bare
    ``str`` or :class:`UnresolvedTypeRef`) resolve against ``nodes`` (then
    ``base``, for an overlay build). Passing an already node-holding ``TypeRef``
    is rejected: user types must be referenced by URI."""
    if isinstance(spec, str):
        return _resolve_uri(spec, nodes, base)
    if isinstance(spec, UnresolvedTypeRef):
        return _resolve_uri(spec.uri, nodes, base)
    if isinstance(spec, PrimitiveTypeRef):
        return spec
    if isinstance(spec, ListTypeRef):
        return ListTypeRef(_resolve(spec.element_type, nodes, base))
    if isinstance(spec, SetTypeRef):
        return SetTypeRef(_resolve(spec.element_type, nodes, base))
    if isinstance(spec, MapTypeRef):
        return MapTypeRef(
            _resolve(spec.key_type, nodes, base),
            _resolve(spec.value_type, nodes, base),
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
        source_info: SourceInfo | None = None,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(
            _PendingStructured(
                uri,
                list(fields),
                is_union=False,
                is_sealed=is_sealed,
                source_info=source_info,
            )
        )
        return self

    def add_union(
        self,
        uri: str,
        fields: Sequence[FieldSpec],
        *,
        is_sealed: bool = False,
        source_info: SourceInfo | None = None,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(
            _PendingStructured(
                uri,
                list(fields),
                is_union=True,
                is_sealed=is_sealed,
                source_info=source_info,
            )
        )
        return self

    def add_enum(
        self,
        uri: str,
        values: Sequence[EnumValue],
        *,
        source_info: SourceInfo | None = None,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(_PendingEnum(uri, list(values), source_info=source_info))
        return self

    def add_opaque_alias(
        self,
        uri: str,
        target: TypeInput,
        *,
        source_info: SourceInfo | None = None,
    ) -> TypeSystemBuilder:
        self._reserve_uri(uri)
        self._pending.append(_PendingOpaqueAlias(uri, target, source_info=source_info))
        return self

    def build(self) -> EnumerableTypeSystem:
        nodes = self._build_placeholders()
        self._populate(nodes)
        return IndexedTypeSystem(nodes)

    def _build_placeholders(self) -> dict[str, DefinitionNode]:
        """Phase 1: allocate one empty node per URI with stable identity. Any
        caller-supplied ``source_info`` is attached up front (it is identity-
        neutral, so it does not affect resolution)."""
        nodes: dict[str, DefinitionNode] = {}
        for pending in self._pending:
            if isinstance(pending, _PendingStructured):
                nodes[pending.uri] = (
                    UnionNode(
                        uri=pending.uri,
                        is_sealed=pending.is_sealed,
                        source_info=pending.source_info,
                    )
                    if pending.is_union
                    else StructNode(
                        uri=pending.uri,
                        is_sealed=pending.is_sealed,
                        source_info=pending.source_info,
                    )
                )
            elif isinstance(pending, _PendingEnum):
                nodes[pending.uri] = EnumNode(
                    uri=pending.uri, source_info=pending.source_info
                )
            elif isinstance(pending, _PendingOpaqueAlias):
                nodes[pending.uri] = OpaqueAliasNode(
                    uri=pending.uri, source_info=pending.source_info
                )
            else:
                assert_never(pending)
        return nodes

    def _populate(
        self, nodes: dict[str, DefinitionNode], base: TypeSystem | None = None
    ) -> None:
        """Phase 2: resolve and validate every node's contents. ``base``, when
        set (an overlay build), is the fallback for unresolved URI references."""
        for pending in self._pending:
            if isinstance(pending, _PendingStructured):
                node = nodes[pending.uri]
                assert isinstance(node, (StructNode, UnionNode))
                node._set_fields(_build_fields(pending, nodes, base))
            elif isinstance(pending, _PendingEnum):
                enum_node = nodes[pending.uri]
                assert isinstance(enum_node, EnumNode)
                enum_node._set_values(_build_enum_values(pending))
            elif isinstance(pending, _PendingOpaqueAlias):
                alias_node = nodes[pending.uri]
                assert isinstance(alias_node, OpaqueAliasNode)
                target = _resolve(pending.target, nodes, base)
                if _is_user_defined(target):
                    raise InvalidTypeError(
                        f"opaque alias {pending.uri!r} target must not be "
                        "user-defined (got a struct/union/enum/opaque-alias)"
                    )
                alias_node._set_target_type(target)
            else:
                assert_never(pending)


def _build_fields(
    pending: _PendingStructured,
    nodes: dict[str, DefinitionNode],
    base: TypeSystem | None = None,
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
                type=_resolve(spec.type, nodes, base),
            )
        )
    return result


def _validate_enum_values(uri: str, values: Sequence[EnumValue]) -> None:
    """Reject duplicate enum value names and datums (shared by the programmatic
    builder and :func:`from_serializable`)."""
    seen_names: set[str] = set()
    seen_datums: set[int] = set()
    for value in values:
        if value.name in seen_names:
            raise InvalidTypeError(
                f"Duplicate enum value name {value.name!r} in {uri!r}"
            )
        if value.datum in seen_datums:
            raise InvalidTypeError(
                f"Duplicate enum value datum {value.datum} in {uri!r}"
            )
        seen_names.add(value.name)
        seen_datums.add(value.datum)


def _build_enum_values(pending: _PendingEnum) -> list[EnumValue]:
    """Validate an enum's values, rejecting duplicate names and datums."""
    _validate_enum_values(pending.uri, pending.values)
    return list(pending.values)


# ---------------------------------------------------------------------------
# Pruning: extract a bounded, self-contained TypeSystem from a larger source.
# ---------------------------------------------------------------------------


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

    ``options.include_source_info`` (default ``True``) controls whether each
    copied node carries its ``source_info`` provenance; it is identity-neutral
    (excluded from the digest). With ``share=True`` the source nodes are reused
    as-is, so their ``source_info`` is preserved regardless of this flag.

    Raises ``InvalidTypeError`` if a root URI is absent from ``source``, or if
    ``source`` is not self-contained (a referenced URI fails to resolve).
    """
    opts = options if options is not None else PruneOptions()
    closure = _collect_closure(source, root_uris, _referenced_uris)
    if share:
        return IndexedTypeSystem(dict(closure))
    return IndexedTypeSystem(_copy_closure(closure, opts.include_source_info))


def _referenced_uris(node: DefinitionNode) -> Iterator[str]:
    """The user-defined URIs ``node`` references through its type structure
    (enum nodes reference nothing)."""
    match node:
        case StructNode() | UnionNode():
            for field in node.fields:
                yield from _type_ref_uris(field.type)
        case EnumNode():
            pass  # enums reference no type-structure URIs
        case OpaqueAliasNode():
            yield from _type_ref_uris(node.target_type)
        case _:
            assert_never(node)


def _copy_closure(
    closure: dict[str, DefinitionNode], include_source_info: bool = True
) -> dict[str, DefinitionNode]:
    """Deep-copy a resolved closure into fresh, independent nodes, rewiring every
    edge to the copies. Two-phase (placeholder then populate) so cyclic and
    mutually-recursive types resolve against stable node identities.

    ``include_source_info`` propagates each node's ``source_info`` into the copy
    (default ``True``); when ``False`` the copies carry no provenance."""
    new_nodes: dict[str, DefinitionNode] = {
        uri: _empty_copy(node) for uri, node in closure.items()
    }
    for uri, node in closure.items():
        _populate_copy(node, new_nodes[uri], new_nodes, include_source_info)
    return new_nodes


def _empty_copy(node: DefinitionNode) -> DefinitionNode:
    """Phase 1: an empty same-kind node carrying ``node``'s URI identity."""
    match node:
        case UnionNode():
            return UnionNode(uri=node.uri, is_sealed=node.is_sealed)
        case StructNode():
            return StructNode(uri=node.uri, is_sealed=node.is_sealed)
        case EnumNode():
            return EnumNode(uri=node.uri)
        case OpaqueAliasNode():
            return OpaqueAliasNode(uri=node.uri)
        case _:
            assert_never(node)


def _populate_copy(
    src: DefinitionNode,
    dst: DefinitionNode,
    new_nodes: dict[str, DefinitionNode],
    include_source_info: bool = True,
) -> None:
    """Phase 2: copy ``src``'s contents into ``dst``, rewiring edges to the
    copied nodes in ``new_nodes``."""
    match src:
        case StructNode() | UnionNode():
            assert isinstance(dst, (StructNode, UnionNode))
            dst._set_fields([_copy_field(f, new_nodes) for f in src.fields])
        case EnumNode():
            assert isinstance(dst, EnumNode)
            dst._set_values(
                [EnumValue(v.name, v.datum, v.annotations) for v in src.values]
            )
        case OpaqueAliasNode():
            assert isinstance(dst, OpaqueAliasNode)
            dst._set_target_type(_copy_type_ref(src.target_type, new_nodes))
        case _:
            assert_never(src)
    # Annotation records are immutable value objects with no node edges, so
    # sharing them across the copy keeps the result independent of source nodes.
    dst._set_annotations(src.annotations)
    # ``SourceInfo`` is likewise an immutable value with no node edges: safe to
    # share into the copy. Skipped (left ``None``) when source info is excluded.
    if include_source_info:
        dst._set_source_info(src.source_info)


def _copy_field(
    field: FieldDefinition, new_nodes: dict[str, DefinitionNode]
) -> FieldDefinition:
    return FieldDefinition(
        identity=FieldIdentity(field.identity.id, field.identity.name),
        presence=field.presence,
        type=_copy_type_ref(field.type, new_nodes),
        # Defaults and annotations are immutable value records: safe to share.
        custom_default=field.custom_default,
        annotations=field.annotations,
    )


def _copy_type_ref(
    type_ref: TypeRefBase, new_nodes: dict[str, DefinitionNode]
) -> TypeRef:
    """Deep-copy a ``TypeRef``, rewiring user-defined edges to ``new_nodes``."""
    if not isinstance(type_ref, TypeRef):
        raise InvalidTypeError(
            f"{type(type_ref).__name__} is not a resolved type edge (TypeRef)."
        )
    match type_ref:
        case PrimitiveTypeRef():
            return PrimitiveTypeRef(type_ref.primitive)
        case ListTypeRef():
            return ListTypeRef(_copy_type_ref(type_ref.element_type, new_nodes))
        case SetTypeRef():
            return SetTypeRef(_copy_type_ref(type_ref.element_type, new_nodes))
        case MapTypeRef():
            return MapTypeRef(
                _copy_type_ref(type_ref.key_type, new_nodes),
                _copy_type_ref(type_ref.value_type, new_nodes),
            )
        case StructTypeRef() | UnionTypeRef() | EnumTypeRef() | OpaqueAliasTypeRef():
            return _type_ref_for_node(new_nodes[type_ref.node.uri])
        case _:
            assert_never(type_ref)


# ---------------------------------------------------------------------------
# build_derived_from: additive overlay composition (overlay union base).
# ---------------------------------------------------------------------------


class _DerivedTypeSystem(TypeSystem):
    """An additive overlay ``result = overlay ∪ base``: holds the overlay's own
    nodes and **delegates** to ``base`` on a miss (base nodes are never copied)."""

    __slots__ = ("_overlay", "_base", "_source_index")
    _overlay: dict[str, DefinitionNode]
    _base: TypeSystem
    _source_index: dict[str, dict[str, DefinitionNode]]

    def __init__(
        self,
        overlay: dict[str, DefinitionNode],
        base: TypeSystem,
        source_index: dict[str, dict[str, DefinitionNode]],
    ) -> None:
        self._overlay = overlay
        self._base = base
        self._source_index = source_index

    def get_user_defined_type(self, uri: str) -> DefinitionNode | None:
        node = self._overlay.get(uri)
        if node is not None:
            return node
        return self._base.get_user_defined_type(uri)

    def get_known_uris(self) -> frozenset[str] | None:
        base_uris = self._base.get_known_uris()
        if base_uris is None:
            return None
        return base_uris | frozenset(self._overlay)

    def get_user_defined_type_by_source_identifier(
        self, locator: str, name: str
    ) -> DefinitionNode | None:
        node = self._source_index.get(locator, {}).get(name)
        if node is not None:
            return node
        return self._base.get_user_defined_type_by_source_identifier(locator, name)

    def get_user_defined_types_at_location(
        self, locator: str
    ) -> Mapping[str, DefinitionNode]:
        result = dict(self._base.get_user_defined_types_at_location(locator))
        result.update(self._source_index.get(locator, {}))
        return MappingProxyType(result)

    def __repr__(self) -> str:
        return f"_DerivedTypeSystem({len(self._overlay)} overlay types over {self._base!r})"


def build_derived_from(overlay: TypeSystemBuilder, base: TypeSystem) -> TypeSystem:
    """Compose ``overlay`` (a builder of pending definitions) on top of ``base``,
    producing an additive ``result = overlay ∪ base``.

    The overlay may reference base types by URI (resolved through ``base`` during
    the build). The result is **not** independent: it holds ``base`` and delegates
    to it -- base nodes are never copied.

    A URI already defined in ``base`` is rejected with ``InvalidTypeError`` (the
    overlay cannot shadow or change a base type by URI). An overlay source
    identifier ``(locator, name)`` already present in ``base`` is likewise
    rejected (even when the URI differs) when ``base`` supports source enumeration."""
    nodes = overlay._build_placeholders()
    for uri in nodes:
        if base.get_user_defined_type(uri) is not None:
            raise InvalidTypeError(
                f"Type {uri!r} conflicts with an existing definition in the base "
                "TypeSystem"
            )
    # Build the overlay's own source index eagerly so overlay-internal duplicate
    # source identifiers are rejected up front (mirroring the eager URI check
    # above) instead of being deferred to the first lazy source lookup.
    overlay_source_index = _index_by_source(nodes.values())
    # Reject an overlay source identifier that already exists in ``base`` (its URI
    # may differ).
    for locator, by_name in overlay_source_index.items():
        for name in by_name:
            if (
                base.get_user_defined_type_by_source_identifier(locator, name)
                is not None
            ):
                raise InvalidTypeError(
                    f"Source identifier {name!r} at location {locator!r} conflicts "
                    "with an existing definition in the base TypeSystem"
                )
    overlay._populate(nodes, base)
    return _DerivedTypeSystem(nodes, base, source_index=overlay_source_index)
