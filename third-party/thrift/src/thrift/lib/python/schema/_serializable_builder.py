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
``from_serializable`` -- the canonical ``SerializableTypeSystem`` wire form ->
a self-contained runtime ``TypeSystem``.
"""

from __future__ import annotations

from collections.abc import Sequence

from apache.thrift.type_system.type_system.thrift_types import (
    SerializableFieldDefinition,
    SerializableTypeDefinition,
    SerializableTypeDefinitionEntry,
    SerializableTypeSystem,
)
from thrift.lib.python.schema._serializable import (
    annotations_from_wire,
    from_wire_record,
    resolve_type_id,
)
from thrift.lib.python.schema.type_system import (
    DefinitionNode,
    EnumerableTypeSystem,
    EnumNode,
    EnumValue,
    FieldDefinition,
    FieldIdentity,
    IndexedTypeSystem,
    InvalidTypeError,
    OpaqueAliasNode,
    PresenceQualifier,
    SourceInfo,
    StructNode,
    TypeSystem,
    UnionNode,
)
from thrift.lib.python.schema.type_system_builder import (
    _is_user_defined,
    _validate_enum_values,
)


def from_serializable(sts: SerializableTypeSystem) -> EnumerableTypeSystem:
    """Build an independent ``IndexedTypeSystem`` from the canonical
    ``SerializableTypeSystem`` wire form.

    Two-phase (placeholders -> populate) so cyclic and mutually-recursive types
    resolve against stable node identities. Validators (each raising
    ``InvalidTypeError``): unique field id, unique field name, union fields
    optional, unique enum name, unique enum datum, opaque-alias target not
    user-defined, unknown/unresolvable URI; plus the annotation-metadata checks
    (each annotation URI must resolve to a **struct**, and each value must be
    a **field-set** record).
    """
    types = sts.types
    arm = SerializableTypeDefinition.Type

    # Phase 1: one empty placeholder per URI, of the correct kind.
    nodes: dict[str, DefinitionNode] = {}
    for uri, entry in types.items():
        definition = entry.definition
        kind = definition.type
        if kind == arm.structDef:
            nodes[uri] = StructNode(uri=uri, is_sealed=definition.structDef.isSealed)
        elif kind == arm.unionDef:
            nodes[uri] = UnionNode(uri=uri, is_sealed=definition.unionDef.isSealed)
        elif kind == arm.enumDef:
            nodes[uri] = EnumNode(uri=uri)
        elif kind == arm.opaqueAliasDef:
            nodes[uri] = OpaqueAliasNode(uri=uri)
        else:
            raise InvalidTypeError(
                f"Unknown SerializableTypeDefinition for {uri!r}: {kind!r}"
            )

    # The populated nodes are mutated in place; this view resolves URI references
    # (incl. annotation URIs) against the stable placeholders.
    source = IndexedTypeSystem(nodes)
    resolve_uri = source.get_user_defined_type

    # Phase 2: resolve + validate, mutating the placeholders in place.
    for uri, entry in types.items():
        definition = entry.definition
        kind = definition.type
        node = nodes[uri]
        if kind == arm.structDef:
            assert isinstance(node, StructNode)
            struct_def = definition.structDef
            node._set_fields(
                _fields_from_serializable(
                    uri, struct_def.fields, source, is_union=False
                )
            )
            node._set_annotations(
                annotations_from_wire(struct_def.annotations, resolve_uri)
            )
        elif kind == arm.unionDef:
            assert isinstance(node, UnionNode)
            union_def = definition.unionDef
            node._set_fields(
                _fields_from_serializable(uri, union_def.fields, source, is_union=True)
            )
            node._set_annotations(
                annotations_from_wire(union_def.annotations, resolve_uri)
            )
        elif kind == arm.enumDef:
            assert isinstance(node, EnumNode)
            enum_def = definition.enumDef
            values = [
                EnumValue(
                    v.name,
                    v.datum,
                    annotations_from_wire(v.annotations, resolve_uri),
                )
                for v in enum_def.values
            ]
            _validate_enum_values(uri, values)
            node._set_values(values)
            node._set_annotations(
                annotations_from_wire(enum_def.annotations, resolve_uri)
            )
        elif kind == arm.opaqueAliasDef:
            assert isinstance(node, OpaqueAliasNode)
            opaque_def = definition.opaqueAliasDef
            target = resolve_type_id(opaque_def.targetType, source)
            if _is_user_defined(target):
                raise InvalidTypeError(
                    f"opaque alias {uri!r} target must not be user-defined "
                    "(got a struct/union/enum/opaque-alias)"
                )
            node._set_target_type(target)
            node._set_annotations(
                annotations_from_wire(opaque_def.annotations, resolve_uri)
            )

        _apply_wire_source_info(node, entry)

    return source


def _apply_wire_source_info(
    node: DefinitionNode, entry: SerializableTypeDefinitionEntry
) -> None:
    """Copy an entry's wire ``sourceInfo`` onto ``node`` when present. Source
    info is identity-neutral, so it is read for every node kind."""
    wire = entry.sourceInfo
    if wire is not None:
        node._set_source_info(SourceInfo(wire.locator, wire.name))


def _fields_from_serializable(
    uri: str,
    wire_fields: Sequence[SerializableFieldDefinition],
    source: TypeSystem,
    *,
    is_union: bool,
) -> list[FieldDefinition]:
    """Resolve and validate a structured node's wire fields. For unions, the
    non-optional check fires before duplicate-id/name."""
    result: list[FieldDefinition] = []
    seen_ids: set[int] = set()
    seen_names: set[str] = set()
    for wire_field in wire_fields:
        field_id = wire_field.identity.id
        field_name = wire_field.identity.name
        try:
            presence = PresenceQualifier(wire_field.presence.value)
        except ValueError as e:
            raise InvalidTypeError(
                f"Invalid presence qualifier in {uri!r} field {field_name!r}: "
                f"{wire_field.presence!r}"
            ) from e
        if is_union and presence != PresenceQualifier.OPTIONAL:
            raise InvalidTypeError(
                f"Union {uri!r} field {field_name!r} must be OPTIONAL, "
                f"not {presence.name}"
            )
        if field_id in seen_ids:
            raise InvalidTypeError(f"Duplicate field id {field_id} in {uri!r}")
        if field_name in seen_names:
            raise InvalidTypeError(f"Duplicate field name {field_name!r} in {uri!r}")
        seen_ids.add(field_id)
        seen_names.add(field_name)
        default = wire_field.customDefaultPartialRecord
        result.append(
            FieldDefinition(
                identity=FieldIdentity(field_id, field_name),
                presence=presence,
                type=resolve_type_id(wire_field.type, source),
                custom_default=(
                    from_wire_record(default) if default is not None else None
                ),
                annotations=annotations_from_wire(
                    wire_field.annotations, source.get_user_defined_type
                ),
            )
        )
    return result
