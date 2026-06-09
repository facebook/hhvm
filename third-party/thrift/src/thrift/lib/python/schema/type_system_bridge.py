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
* field/node structured **annotations** become a URI -> ``SerializableRecord``
  map (compiler-consumed ``Uri`` / ``scope/*`` annotations dropped), and field
  custom **defaults** become a ``SerializableRecord``;
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
from collections.abc import Mapping, Sequence
from typing import Protocol

from apache.thrift.protocol.detail.protocol_detail.thrift_types import Value
from thrift.lib.python.schema import syntax_graph as _ast
from thrift.lib.python.schema._record import (
    BoolRecord,
    ByteArrayRecord,
    FieldSetRecord,
    Float32Record,
    Float64Record,
    Int16Record,
    Int32Record,
    Int64Record,
    Int8Record,
    ListRecord,
    MapRecord,
    SerializableRecord,
    SetRecord,
    TextRecord,
)
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


# ---------------------------------------------------------------------------
# Annotation + custom-default conversion (AST -> SerializableRecord).
#
# Two distinct walkers:
#  * field custom defaults come from a protocol ``Value`` (self-describing, so we
#    convert value-directed -- the active arm picks the record kind);
#  * structured-annotation values come from the AST's *decoded* name-keyed
#    ``dict`` (lossy: an ``int`` could be any width), so we convert type-directed
#    using the annotation struct's own field types and re-resolve names -> ids.
# ---------------------------------------------------------------------------

# Compiler-consumed annotations dropped from the TypeSystem view: exactly the
# ``Uri`` annotation and anything in the ``scope/`` sub-namespace.
_THRIFT_URI_ANNOTATION = "facebook.com/thrift/annotation/Uri"
_SCOPE_ANNOTATION_PREFIX = "facebook.com/thrift/annotation/scope/"


def _is_compiler_consumed_annotation(uri: str) -> bool:
    return uri == _THRIFT_URI_ANNOTATION or uri.startswith(_SCOPE_ANNOTATION_PREFIX)


def _annotation_struct_uri(annotation: _ast.Annotation) -> str:
    """The URI of the annotation's (typedef-erased) struct type. Annotations are
    always struct-shaped; anything else is a malformed schema."""
    true_ref = annotation.type.true_type
    if isinstance(
        true_ref, (_ast.StructTypeRef, _ast.UnionTypeRef, _ast.ExceptionTypeRef)
    ):
        return true_ref.node.uri
    raise InvalidTypeError(
        f"annotation type is not a struct/union/exception: {true_ref!r}"
    )


def _as_bytes(raw: object) -> bytes:
    """Coerce a thrift binary payload to ``bytes``. The ``stringValue`` arm is a
    plain ``binary`` (already ``bytes``), but ``binaryValue`` is a ``ByteBuffer``
    which thrift-python represents as a ``folly.iobuf.IOBuf`` -- iterate its
    chunks to flatten it."""
    if isinstance(raw, (bytes, bytearray, memoryview)):
        return bytes(raw)
    return b"".join(raw)  # IOBuf: iterates its (memoryview) chunks


def _as_text(raw: object) -> str:
    """Coerce a thrift text/binary payload to ``str`` (decoding bytes as UTF-8).
    UTF-8 validity is re-checked when the ``TextRecord`` is constructed."""
    if isinstance(raw, str):
        return raw
    return _as_bytes(raw).decode("utf-8")


def _scalar_record_from_protocol_value(value: Value) -> SerializableRecord | None:
    """The nine scalar arms of a protocol ``Value`` -> matching record arm, or
    ``None`` for the container/object arms (handled by the caller)."""
    arm = type(value).Type
    kind = value.type
    if kind == arm.boolValue:
        return BoolRecord(value.boolValue)
    if kind == arm.byteValue:
        return Int8Record(value.byteValue)
    if kind == arm.i16Value:
        return Int16Record(value.i16Value)
    if kind == arm.i32Value:
        return Int32Record(value.i32Value)
    if kind == arm.i64Value:
        return Int64Record(value.i64Value)
    if kind == arm.floatValue:
        return Float32Record(value.floatValue)
    if kind == arm.doubleValue:
        return Float64Record(value.doubleValue)
    if kind == arm.stringValue:
        return TextRecord(_as_text(value.stringValue))
    if kind == arm.binaryValue:
        return ByteArrayRecord(_as_bytes(value.binaryValue))
    return None


def _record_from_protocol_value(value: Value) -> SerializableRecord:
    """Convert a protocol ``Value`` (a field's custom default) to a
    ``SerializableRecord``, value-directed: the union's active arm determines the
    record kind."""
    scalar = _scalar_record_from_protocol_value(value)
    if scalar is not None:
        return scalar
    arm = type(value).Type
    kind = value.type
    if kind == arm.listValue:
        return ListRecord([_record_from_protocol_value(v) for v in value.listValue])
    if kind == arm.setValue:
        return SetRecord([_record_from_protocol_value(v) for v in value.setValue])
    if kind == arm.mapValue:
        return MapRecord(
            [
                (_record_from_protocol_value(k), _record_from_protocol_value(v))
                for k, v in value.mapValue.items()
            ]
        )
    if kind == arm.objectValue:
        members = value.objectValue.members
        return FieldSetRecord(
            {fid: _record_from_protocol_value(v) for fid, v in members.items()}
        )
    raise InvalidTypeError(f"cannot convert protocol Value (kind={kind!r}) to record")


def _primitive_record(primitive: _ast.Primitive, value: object) -> SerializableRecord:
    """Wrap a decoded annotation scalar in the record arm chosen by its declared
    primitive type (the decoded value alone can't distinguish int widths)."""
    primitives = _ast.Primitive
    if primitive == primitives.BOOL:
        return BoolRecord(bool(value))
    if primitive == primitives.BYTE:
        return Int8Record(int(value))
    if primitive == primitives.I16:
        return Int16Record(int(value))
    if primitive == primitives.I32:
        return Int32Record(int(value))
    if primitive == primitives.I64:
        return Int64Record(int(value))
    if primitive == primitives.FLOAT:
        return Float32Record(float(value))
    if primitive == primitives.DOUBLE:
        return Float64Record(float(value))
    if primitive == primitives.STRING:
        return TextRecord(_as_text(value))
    if primitive == primitives.BINARY:
        return ByteArrayRecord(_as_bytes(value))
    raise InvalidTypeError(f"unsupported annotation primitive: {primitive!r}")


def _record_from_annotation_value(ast_ref: object, value: object) -> SerializableRecord:
    """Convert a decoded annotation value to a ``SerializableRecord``,
    type-directed by the (typedef-erased) AST type."""
    assert isinstance(ast_ref, _ast.TypeRef)
    true_ref = ast_ref.true_type  # typedef erasure
    if isinstance(true_ref, _ast.PrimitiveTypeRef):
        return _primitive_record(true_ref.primitive, value)
    if isinstance(
        true_ref, (_ast.StructTypeRef, _ast.UnionTypeRef, _ast.ExceptionTypeRef)
    ):
        return _field_set_from_annotation(true_ref.node, value)
    if isinstance(true_ref, _ast.EnumTypeRef):
        return Int32Record(int(value))
    if isinstance(true_ref, _ast.ListTypeRef):
        return ListRecord(
            [
                _record_from_annotation_value(true_ref.element_type, v)
                for v in value  # pyre-ignore[16]: decoded list
            ]
        )
    if isinstance(true_ref, _ast.SetTypeRef):
        return SetRecord(
            [_record_from_annotation_value(true_ref.element_type, v) for v in value]
        )
    if isinstance(true_ref, _ast.MapTypeRef):
        return MapRecord(
            [
                (
                    _record_from_annotation_value(true_ref.key_type, k),
                    _record_from_annotation_value(true_ref.value_type, v),
                )
                for k, v in value.items()  # pyre-ignore[16]: decoded map
            ]
        )
    raise InvalidTypeError(f"cannot convert annotation value of type {true_ref!r}")


def _field_set_from_annotation(
    node: _ast.StructuredDefinition, value: object
) -> FieldSetRecord:
    """Re-key a structured annotation value to field ids. The decoded ``value`` is
    keyed by field *name* at the top level and by field *id* for nested objects;
    both are resolved against the annotation struct's own fields."""
    if not isinstance(value, Mapping):
        raise InvalidTypeError(f"structured annotation value must be a map: {value!r}")
    by_name = {field.name: field for field in node.fields}
    by_id = {field.id: field for field in node.fields}
    fields: dict[int, SerializableRecord] = {}
    for key, raw in value.items():
        field = by_name.get(key) if isinstance(key, str) else by_id.get(key)
        if field is None:
            raise InvalidTypeError(
                f"annotation field {key!r} not found in {node.uri!r}"
            )
        fields[field.id] = _record_from_annotation_value(field.type, raw)
    return FieldSetRecord(fields)


def _bridge_annotations(
    annotations: Sequence[_ast.Annotation],
) -> dict[str, SerializableRecord]:
    """Convert a definition's or field's structured annotations into a
    URI -> ``SerializableRecord`` map, dropping compiler-consumed ones."""
    result: dict[str, SerializableRecord] = {}
    for annotation in annotations:
        uri = _annotation_struct_uri(annotation)
        if _is_compiler_consumed_annotation(uri):
            continue
        result[uri] = _record_from_annotation_value(annotation.type, annotation.value)
    return result


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
                annotations=_bridge_annotations(ast_def.annotations),
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
        structured._set_annotations(_bridge_annotations(ast_def.annotations))
        return structured

    def _bridge_field(
        self,
        field: _ast.FieldNode,
        parent_name: str,
        is_union: bool,
        added_keys: set[bytes],
    ) -> FieldDefinition:
        default_value = field.default_value
        custom_default = (
            _record_from_protocol_value(default_value)
            if default_value is not None
            else None
        )
        return FieldDefinition(
            identity=FieldIdentity(field.id, field.name),
            presence=_ts_presence(field, is_union),
            type=self._bridge_type(
                field.type, added_keys, f"{parent_name}.{field.name}"
            ),
            custom_default=custom_default,
            annotations=_bridge_annotations(field.annotations),
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
