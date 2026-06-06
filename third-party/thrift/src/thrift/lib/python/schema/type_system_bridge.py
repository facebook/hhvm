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
SyntaxGraph -> TypeSystem bridge.

``SyntaxGraphBridge`` is the unbounded ``TypeSystem`` view over an existing AST:
it resolves a URI (via a :class:`SyntaxResolver`) to its AST definition and
materializes a memoized ``DefinitionNode`` graph.

Semantic mappings:

* **typedefs are erased** to their true type (``.true_type``) -- there is no
  typedef node;
* **exceptions** bridge to ``StructNode``;
* **unions** force all fields ``OPTIONAL``;
* the IDL ``any`` type (``AnyStruct``, URI ``facebook.com/thrift/type/Any``)
  bridges to ``PrimitiveTypeRef(ANY)`` -- *not* a user-defined struct;
* field presence maps the AST ``qualifier``: ``Optional -> OPTIONAL``,
  ``Terse -> TERSE``, else ``UNQUALIFIED`` (union forces ``OPTIONAL``);
* every bridged definition **must have a URI** (the URI is its TypeSystem
  identity): a URI-less definition is rejected with ``InvalidTypeError`` at
  bridge time.

The build is cycle-safe: each placeholder node is registered in the cache
(keyed by the AST ``definition_key``) before its fields are populated, so
self- and mutually-recursive types resolve against stable identities. It is
also atomic: every node added during a single top-level build is tracked, and
if population fails partway (e.g. a transitively-referenced type is URI-less)
*all* of them are evicted -- a failed build never leaves a partially-populated
node cached, not even one reached through a cycle.
"""

from __future__ import annotations

import functools
from typing import Protocol

from thrift.lib.python.schema import syntax_graph as _ast
from thrift.lib.python.schema.type_system import (
    DefinitionNode,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    FieldDefinition,
    FieldIdentity,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
    TypeSystem,
    UnionNode,
    UnionTypeRef,
)


class SyntaxResolver(Protocol):
    """The AST entry-point the bridge needs: resolve a URI to its AST
    definition (or ``None`` if unknown), triggering any lazy module discovery.
    Satisfied structurally by ``SchemaRegistry``."""

    def definition_by_uri(self, uri: str) -> object | None: ...


# The IDL ``any`` built-in is defined as a struct; the TypeSystem abstracts that
# "structiness" away into the ``ANY`` primitive.
_ANY_STRUCT_URI = "facebook.com/thrift/type/Any"

# AST definitions the bridge can turn into TypeSystem nodes.
_BridgeableDef = _ast.StructNode | _ast.UnionNode | _ast.ExceptionNode | _ast.EnumNode


@functools.cache
def _interned_primitive(primitive: Primitive) -> PrimitiveTypeRef:
    """Interned ``PrimitiveTypeRef`` singletons (no per-edge allocation)."""
    return PrimitiveTypeRef(primitive)


def _ts_primitive(ast_primitive: _ast.Primitive) -> PrimitiveTypeRef:
    """Map an AST ``Primitive`` (BOOL..BINARY) to the interned TS primitive.

    The names line up one-to-one; the TS-only ``ANY`` is handled by the
    AnyStruct special-case, not here."""
    return _interned_primitive(Primitive[ast_primitive.name])


def _ts_presence(field: _ast.FieldNode, is_union: bool) -> PresenceQualifier:
    """AST field qualifier -> TS ``PresenceQualifier``."""
    if is_union or field.qualifier == _ast.FieldQualifier.Optional:
        return PresenceQualifier.OPTIONAL
    if field.qualifier == _ast.FieldQualifier.Terse:
        return PresenceQualifier.TERSE
    return PresenceQualifier.UNQUALIFIED


def _ref_for_node(node: DefinitionNode) -> TypeRef:
    """Wrap a resolved TS node in the matching user-defined ``TypeRef``."""
    if isinstance(node, StructNode):
        return StructTypeRef(node)
    if isinstance(node, UnionNode):
        return UnionTypeRef(node)
    if isinstance(node, EnumNode):
        return EnumTypeRef(node)
    if isinstance(node, OpaqueAliasNode):
        return OpaqueAliasTypeRef(node)
    raise InvalidTypeError(f"Unexpected definition node: {node!r}")


class SyntaxGraphBridge(TypeSystem):
    """Lazy, memoized ``TypeSystem`` view bridged from AST definitions.

    Unbounded: ``get_known_uris()`` returns ``None`` (the source is
    lazy/module-discovery based). Bridged nodes are fully materialized and own
    no AST references, so the source AST can be garbage-collected
    independently."""

    def __init__(self, resolver: SyntaxResolver) -> None:
        self._resolver = resolver
        # Keyed by AST definition_key (always unique; URIs may be empty).
        self._cache: dict[bytes, DefinitionNode] = {}

    def get_user_defined_type(self, uri: str) -> DefinitionNode | None:
        ast_def = self._resolver.definition_by_uri(uri)
        if ast_def is None:
            return None
        # Typedefs/constants/services/interactions are not TypeSystem
        # user-defined types.
        if not isinstance(
            ast_def,
            (_ast.StructNode, _ast.UnionNode, _ast.ExceptionNode, _ast.EnumNode),
        ):
            return None
        added_keys: set[bytes] = set()
        try:
            return self._bridge_definition(ast_def, added_keys)
        except Exception:
            for key in added_keys:
                self._cache.pop(key, None)
            raise

    def get_known_uris(self) -> None:
        return None

    # -- internal -----------------------------------------------------------

    def _bridge_definition(
        self,
        ast_def: _BridgeableDef,
        added_keys: set[bytes],
        referenced_by: str | None = None,
    ) -> DefinitionNode:
        if not ast_def.uri:
            where = f" (referenced by field {referenced_by})" if referenced_by else ""
            raise InvalidTypeError(
                f"Cannot bridge URI-less {type(ast_def).__name__} "
                f"{ast_def.name!r}{where}: a TypeSystem type's identity is its "
                f"URI, so the thrift file defining {ast_def.name!r} may be missing "
                "a `package` declaration (or the type a `@thrift.Uri`)."
            )

        key = ast_def.definition_key
        cached = self._cache.get(key)
        if cached is not None:
            return cached

        if isinstance(ast_def, _ast.EnumNode):
            enum_node = EnumNode(
                uri=ast_def.uri,
                values=[EnumValue(v.name, v.value) for v in ast_def.values],
            )
            self._cache[key] = enum_node
            added_keys.add(key)
            return enum_node

        # struct/exception -> StructNode; union -> UnionNode.
        structured: StructNode | UnionNode
        if isinstance(ast_def, _ast.UnionNode):
            structured = UnionNode(uri=ast_def.uri)
            is_union = True
        else:  # StructNode or ExceptionNode
            structured = StructNode(uri=ast_def.uri)
            is_union = False
        # Register BEFORE populating so cyclic references resolve.
        self._cache[key] = structured
        added_keys.add(key)
        structured._set_fields(
            [
                self._bridge_field(f, ast_def.name, is_union, added_keys)
                for f in ast_def.fields
            ]
        )
        return structured

    def _bridge_field(
        self,
        field: _ast.FieldNode,
        parent_name: str,
        is_union: bool,
        added_keys: set[bytes],
    ) -> FieldDefinition:
        return FieldDefinition(
            identity=FieldIdentity(field.id, field.name),
            presence=_ts_presence(field, is_union),
            type=self._bridge_type(
                field.type, added_keys, f"{parent_name}.{field.name}"
            ),
        )

    def _bridge_type(
        self,
        ast_ref: object,
        added_keys: set[bytes],
        referenced_by: str | None = None,
    ) -> TypeRef:
        assert isinstance(ast_ref, _ast.TypeRef)
        true_ref = ast_ref.true_type  # typedef erasure happens here

        if isinstance(true_ref, _ast.PrimitiveTypeRef):
            return _ts_primitive(true_ref.primitive)
        if isinstance(true_ref, _ast.ListTypeRef):
            return ListTypeRef(
                self._bridge_type(true_ref.element_type, added_keys, referenced_by)
            )
        if isinstance(true_ref, _ast.SetTypeRef):
            return SetTypeRef(
                self._bridge_type(true_ref.element_type, added_keys, referenced_by)
            )
        if isinstance(true_ref, _ast.MapTypeRef):
            return MapTypeRef(
                self._bridge_type(true_ref.key_type, added_keys, referenced_by),
                self._bridge_type(true_ref.value_type, added_keys, referenced_by),
            )
        if isinstance(true_ref, _ast.StructTypeRef):
            node = true_ref.node
            if node.uri == _ANY_STRUCT_URI:
                return _interned_primitive(Primitive.ANY)
            return _ref_for_node(
                self._bridge_definition(node, added_keys, referenced_by)
            )
        if isinstance(true_ref, _ast.ExceptionTypeRef):
            return _ref_for_node(
                self._bridge_definition(true_ref.node, added_keys, referenced_by)
            )
        if isinstance(true_ref, _ast.UnionTypeRef):
            return _ref_for_node(
                self._bridge_definition(true_ref.node, added_keys, referenced_by)
            )
        if isinstance(true_ref, _ast.EnumTypeRef):
            return _ref_for_node(
                self._bridge_definition(true_ref.node, added_keys, referenced_by)
            )
        raise InvalidTypeError(f"Cannot bridge AST type to TypeSystem: {true_ref!r}")
