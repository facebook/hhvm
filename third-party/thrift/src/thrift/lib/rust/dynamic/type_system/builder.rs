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

//! Builder for constructing a `TypeSystem` from serializable Thrift definitions.

use std::collections::HashMap;
use std::sync::Arc;

use type_id::TypeId;

use crate::error::InvalidTypeError;
use crate::field::AnnotationsMap;
use crate::field::FieldDefinition;
use crate::field::FieldIdentity;
use crate::field::PresenceQualifier;
use crate::nodes::EnumNode;
use crate::nodes::EnumValue;
use crate::nodes::ListType;
use crate::nodes::MapType;
use crate::nodes::OpaqueAliasNode;
use crate::nodes::SetType;
use crate::nodes::StructNode;
use crate::nodes::UnionNode;
use crate::type_ref::DefinitionRef;
use crate::type_ref::TypeRef;
use crate::type_system::BasicTypeSystem;
use crate::type_system::DefinitionNode;
use crate::type_system::LayeredTypeSystem;
use crate::type_system::TypeSystem;

/// Builds a `TypeSystem` from serializable Thrift definitions.
pub struct TypeSystemBuilder {
    entries: HashMap<String, type_system::SerializableTypeDefinition>,
}

impl TypeSystemBuilder {
    pub fn new() -> Self {
        Self {
            entries: HashMap::new(),
        }
    }

    /// Add a single type definition entry.
    pub fn add_entry(
        &mut self,
        uri: String,
        entry: type_system::SerializableTypeDefinitionEntry,
    ) -> Result<(), InvalidTypeError> {
        if self.entries.contains_key(&uri) {
            return Err(InvalidTypeError::DuplicateUri(uri));
        }
        self.entries.insert(uri, entry.definition);
        Ok(())
    }

    /// Add all definitions from a serializable type system.
    pub fn add_type_system(
        &mut self,
        ts: type_system::SerializableTypeSystem,
    ) -> Result<(), InvalidTypeError> {
        for (uri, entry) in ts.types {
            self.add_entry(uri, entry)?;
        }
        Ok(())
    }

    /// Build a builder pre-populated from a serializable type system.
    pub fn from_serializable(ts: type_system::SerializableTypeSystem) -> Self {
        let mut builder = Self::new();
        for (uri, entry) in ts.types {
            builder.entries.insert(uri, entry.definition);
        }
        builder
    }

    /// Build a standalone type system.
    pub fn build(self) -> Result<BasicTypeSystem, InvalidTypeError> {
        build_basic(self.entries, None)
    }

    /// Build a type system layered on top of a base.
    ///
    /// The base is used during resolution to look up types not defined in this
    /// builder. The resulting `LayeredTypeSystem` delegates lookups to the
    /// overlay first, then falls back to the base.
    pub fn build_layered_on<T: TypeSystem>(
        self,
        base: T,
    ) -> Result<LayeredTypeSystem<T>, InvalidTypeError> {
        for uri in self.entries.keys() {
            if base.get(uri).is_some() {
                return Err(InvalidTypeError::DuplicateUri(uri.clone()));
            }
        }
        let overlay = build_basic(self.entries, Some(&base))?;
        Ok(LayeredTypeSystem::new(overlay, base))
    }
}

impl Default for TypeSystemBuilder {
    fn default() -> Self {
        Self::new()
    }
}

/// Two-phase build.
///
/// Phase 1 creates a shell node for every URI, so every name resolves to an
/// `Arc` we can downgrade into a `Weak` edge. Phase 2 resolves each definition's
/// field types (and opaque-alias targets) against those shells — and the base,
/// when layering — then installs them. This handles forward references and
/// cycles (recursive and mutually recursive types) without a fixpoint loop.
fn build_basic(
    entries: HashMap<String, type_system::SerializableTypeDefinition>,
    base: Option<&dyn TypeSystem>,
) -> Result<BasicTypeSystem, InvalidTypeError> {
    let mut shells: HashMap<String, DefinitionRef> = HashMap::with_capacity(entries.len());
    for (uri, def) in &entries {
        shells.insert(uri.clone(), create_shell(uri, def)?);
    }

    for (uri, def) in &entries {
        populate_definition(uri, def, &shells, base)?;
    }

    let mut definitions = HashMap::with_capacity(shells.len());
    for (uri, def_ref) in shells {
        let node = match def_ref {
            DefinitionRef::Struct(s) => DefinitionNode::Struct(s),
            DefinitionRef::Union(u) => DefinitionNode::Union(u),
            DefinitionRef::Enum(e) => DefinitionNode::Enum(e),
            DefinitionRef::OpaqueAlias(o) => DefinitionNode::OpaqueAlias(o),
        };
        definitions.insert(uri, node);
    }

    Ok(BasicTypeSystem::new(definitions))
}

/// Phase 1: allocate an (empty) node for `uri`. Enums carry no type references,
/// so they are built completely here; the other kinds are populated in phase 2.
fn create_shell(
    uri: &str,
    def: &type_system::SerializableTypeDefinition,
) -> Result<DefinitionRef, InvalidTypeError> {
    Ok(match def {
        type_system::SerializableTypeDefinition::structDef(s) => {
            let annotations = convert_annotations(&s.annotations);
            DefinitionRef::Struct(Arc::new(StructNode::new_shell(
                uri.to_owned(),
                s.isSealed,
                annotations,
            )))
        }

        type_system::SerializableTypeDefinition::unionDef(u) => {
            let annotations = convert_annotations(&u.annotations);
            DefinitionRef::Union(Arc::new(UnionNode::new_shell(
                uri.to_owned(),
                u.isSealed,
                annotations,
            )))
        }

        type_system::SerializableTypeDefinition::enumDef(e) => {
            let values = e
                .values
                .iter()
                .map(|v| {
                    EnumValue::new(v.name.clone(), v.datum, convert_annotations(&v.annotations))
                })
                .collect();
            let annotations = convert_annotations(&e.annotations);
            DefinitionRef::Enum(Arc::new(EnumNode::new(
                uri.to_owned(),
                values,
                annotations,
            )?))
        }

        type_system::SerializableTypeDefinition::opaqueAliasDef(o) => {
            let annotations = convert_annotations(&o.annotations);
            DefinitionRef::OpaqueAlias(Arc::new(OpaqueAliasNode::new_shell(
                uri.to_owned(),
                annotations,
            )))
        }

        type_system::SerializableTypeDefinition::UnknownField(id) => {
            return Err(InvalidTypeError::UnknownUri(format!(
                "unknown definition variant {id} for {uri}"
            )));
        }
    })
}

/// Phase 2: resolve and install field types / opaque-alias targets.
fn populate_definition(
    uri: &str,
    def: &type_system::SerializableTypeDefinition,
    shells: &HashMap<String, DefinitionRef>,
    base: Option<&dyn TypeSystem>,
) -> Result<(), InvalidTypeError> {
    match def {
        type_system::SerializableTypeDefinition::structDef(s) => {
            let fields = s
                .fields
                .iter()
                .map(|f| resolve_field(f, shells, base))
                .collect::<Result<Vec<_>, _>>()?;
            shells[uri]
                .as_struct()
                .expect("shell created as struct")
                .set_fields(fields)?;
        }

        type_system::SerializableTypeDefinition::unionDef(u) => {
            let fields = u
                .fields
                .iter()
                .map(|f| resolve_field(f, shells, base))
                .collect::<Result<Vec<_>, _>>()?;
            shells[uri]
                .as_union()
                .expect("shell created as union")
                .set_fields(fields)?;
        }

        // Enums are fully built in phase 1.
        type_system::SerializableTypeDefinition::enumDef(_) => {}

        type_system::SerializableTypeDefinition::opaqueAliasDef(o) => {
            let target = resolve_type_id(&o.targetType, shells, base)?;
            shells[uri]
                .as_opaque_alias()
                .expect("shell created as opaque alias")
                .set_target(target)?;
        }

        type_system::SerializableTypeDefinition::UnknownField(id) => {
            return Err(InvalidTypeError::UnknownUri(format!(
                "unknown definition variant {id} for {uri}"
            )));
        }
    }
    Ok(())
}

fn resolve_field(
    field: &type_system::SerializableFieldDefinition,
    shells: &HashMap<String, DefinitionRef>,
    base: Option<&dyn TypeSystem>,
) -> Result<FieldDefinition, InvalidTypeError> {
    let type_ref = resolve_type_id(&field.r#type, shells, base)?;

    Ok(FieldDefinition::new(
        FieldIdentity {
            id: field.identity.id,
            name: field.identity.name.clone(),
        },
        PresenceQualifier::from(field.presence),
        type_ref,
        field.customDefaultPartialRecord.clone(),
        convert_annotations(&field.annotations),
    ))
}

fn resolve_type_id(
    type_id: &TypeId,
    shells: &HashMap<String, DefinitionRef>,
    base: Option<&dyn TypeSystem>,
) -> Result<TypeRef, InvalidTypeError> {
    match type_id {
        TypeId::boolType(_) => Ok(TypeRef::Bool),
        TypeId::byteType(_) => Ok(TypeRef::Byte),
        TypeId::i16Type(_) => Ok(TypeRef::I16),
        TypeId::i32Type(_) => Ok(TypeRef::I32),
        TypeId::i64Type(_) => Ok(TypeRef::I64),
        TypeId::floatType(_) => Ok(TypeRef::Float),
        TypeId::doubleType(_) => Ok(TypeRef::Double),
        TypeId::stringType(_) => Ok(TypeRef::String),
        TypeId::binaryType(_) => Ok(TypeRef::Binary),
        TypeId::anyType(_) => Ok(TypeRef::Any),

        TypeId::userDefinedType(uri) => {
            if let Some(def) = shells.get(uri.as_str()) {
                return Ok(TypeRef::from(def.clone()));
            }
            if let Some(base) = base {
                if let Some(def) = base.get(uri) {
                    return Ok(TypeRef::from(def));
                }
            }
            Err(InvalidTypeError::UnknownUri(uri.clone()))
        }

        TypeId::listType(list_id) => {
            let element = list_id
                .elementType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let element_ref = resolve_type_id(element, shells, base)?;
            Ok(TypeRef::List(Arc::new(ListType::new(element_ref))))
        }
        TypeId::setType(set_id) => {
            let element = set_id
                .elementType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let element_ref = resolve_type_id(element, shells, base)?;
            Ok(TypeRef::Set(Arc::new(SetType::new(element_ref))))
        }
        TypeId::mapType(map_id) => {
            let key = map_id
                .keyType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let value = map_id
                .valueType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let key_ref = resolve_type_id(key, shells, base)?;
            let value_ref = resolve_type_id(value, shells, base)?;
            Ok(TypeRef::Map(Arc::new(MapType::new(key_ref, value_ref))))
        }

        TypeId::UnknownField(id) => Err(InvalidTypeError::UnknownUri(format!(
            "<unknown TypeId variant {id}>"
        ))),
    }
}

fn convert_annotations(
    annotations: &std::collections::BTreeMap<String, record::SerializableRecord>,
) -> AnnotationsMap {
    annotations
        .iter()
        .map(|(k, v)| (k.clone(), v.clone()))
        .collect()
}
