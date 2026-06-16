/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! Conversion from a live `TypeSystem` to `SerializableTypeSystem`.

use std::collections::BTreeMap;

use crate::field::FieldDefinition;
use crate::structured_node::StructuredNode;
use crate::type_ref::DefinitionRef;
use crate::type_system::TypeSystem;

pub fn to_serializable(ts: &(impl TypeSystem + ?Sized)) -> type_system::SerializableTypeSystem {
    let mut types = BTreeMap::new();
    for uri in ts.known_uris() {
        if let Some(def) = ts.get(uri) {
            types.insert(
                uri.to_owned(),
                type_system::SerializableTypeDefinitionEntry {
                    definition: serialize_definition_ref(&def),
                    sourceInfo: None,
                    ..Default::default()
                },
            );
        }
    }
    type_system::SerializableTypeSystem {
        types,
        ..Default::default()
    }
}

pub(crate) fn serialize_definition_ref(
    def: &DefinitionRef,
) -> type_system::SerializableTypeDefinition {
    match def {
        DefinitionRef::Struct(s) => {
            type_system::SerializableTypeDefinition::structDef(serialize_struct(s.as_ref()))
        }
        DefinitionRef::Union(u) => {
            type_system::SerializableTypeDefinition::unionDef(serialize_union(u.as_ref()))
        }
        DefinitionRef::Enum(e) => {
            type_system::SerializableTypeDefinition::enumDef(serialize_enum(e.as_ref()))
        }
        DefinitionRef::OpaqueAlias(o) => type_system::SerializableTypeDefinition::opaqueAliasDef(
            serialize_opaque_alias(o.as_ref()),
        ),
    }
}

fn serialize_fields(fields: &[FieldDefinition]) -> Vec<type_system::SerializableFieldDefinition> {
    fields
        .iter()
        .map(|f| type_system::SerializableFieldDefinition {
            identity: type_system::FieldIdentity {
                id: f.id(),
                name: f.name().to_owned(),
                ..Default::default()
            },
            presence: type_system::PresenceQualifier(f.presence().0),
            r#type: f.type_ref().id(),
            customDefaultPartialRecord: f.custom_default().cloned(),
            annotations: f
                .annotations()
                .iter()
                .map(|(k, v)| (k.clone(), v.clone()))
                .collect(),
            ..Default::default()
        })
        .collect()
}

fn serialize_struct(node: &dyn StructuredNode) -> type_system::SerializableStructDefinition {
    type_system::SerializableStructDefinition {
        fields: serialize_fields(node.fields()),
        isSealed: node.is_sealed(),
        annotations: node
            .annotations()
            .iter()
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect(),
        ..Default::default()
    }
}

fn serialize_union(node: &dyn StructuredNode) -> type_system::SerializableUnionDefinition {
    type_system::SerializableUnionDefinition {
        fields: serialize_fields(node.fields()),
        isSealed: node.is_sealed(),
        annotations: node
            .annotations()
            .iter()
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect(),
        ..Default::default()
    }
}

fn serialize_enum(node: &crate::nodes::EnumNode) -> type_system::SerializableEnumDefinition {
    type_system::SerializableEnumDefinition {
        values: node
            .values()
            .iter()
            .map(|v| type_system::SerializableEnumValueDefinition {
                name: v.name.clone(),
                datum: v.value,
                annotations: v
                    .annotations()
                    .iter()
                    .map(|(k, v)| (k.clone(), v.clone()))
                    .collect(),
                ..Default::default()
            })
            .collect(),
        annotations: node
            .annotations()
            .iter()
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect(),
        ..Default::default()
    }
}

fn serialize_opaque_alias(
    node: &crate::nodes::OpaqueAliasNode,
) -> type_system::SerializableOpaqueAliasDefinition {
    type_system::SerializableOpaqueAliasDefinition {
        targetType: node.target_type().id(),
        annotations: node
            .annotations()
            .iter()
            .map(|(k, v)| (k.clone(), v.clone()))
            .collect(),
        ..Default::default()
    }
}
