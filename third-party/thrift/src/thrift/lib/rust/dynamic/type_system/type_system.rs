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

//! The `TypeSystem` trait and `BasicTypeSystem`.

use std::collections::HashMap;
use std::sync::Arc;

use type_id::TypeId;

use crate::error::InvalidTypeError;
use crate::nodes::EnumNode;
use crate::nodes::OpaqueAliasNode;
use crate::nodes::StructNode;
use crate::nodes::UnionNode;
use crate::type_ref::DefinitionRef;
use crate::type_ref::TypeRef;

/// Source location of a type definition (Thrift file + definition name).
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SourceIdentifier {
    pub location: String,
    pub name: String,
}

/// Core interface for looking up Thrift type definitions.
pub trait TypeSystem {
    /// Look up a user-defined type by URI.
    fn get(&self, uri: &str) -> Option<DefinitionRef>;

    /// Iterate over all known URIs.
    fn known_uris(&self) -> Box<dyn Iterator<Item = &str> + '_>;

    /// Resolve a serializable `TypeId` to a live `TypeRef`.
    fn resolve(&self, type_id: &TypeId) -> Result<TypeRef, InvalidTypeError>;

    /// Look up a definition by URI, returning an error if not found.
    fn get_or_err(&self, uri: &str) -> Result<DefinitionRef, InvalidTypeError> {
        self.get(uri)
            .ok_or_else(|| InvalidTypeError::UnknownUri(uri.to_owned()))
    }

    /// Convert this type system to its serializable representation.
    fn to_serializable(&self) -> type_system::SerializableTypeSystem {
        crate::serialize::to_serializable(self)
    }
}

/// Internal storage for a definition node within the type system.
pub(crate) enum DefinitionNode {
    Struct(Arc<StructNode>),
    Union(Arc<UnionNode>),
    Enum(Arc<EnumNode>),
    OpaqueAlias(Arc<OpaqueAliasNode>),
}

impl DefinitionNode {
    fn to_definition_ref(&self) -> DefinitionRef {
        match self {
            Self::Struct(s) => DefinitionRef::Struct(Arc::clone(s)),
            Self::Union(u) => DefinitionRef::Union(Arc::clone(u)),
            Self::Enum(e) => DefinitionRef::Enum(Arc::clone(e)),
            Self::OpaqueAlias(o) => DefinitionRef::OpaqueAlias(Arc::clone(o)),
        }
    }
}

/// Extension of [`TypeSystem`] that supports source-identifier lookups.
pub trait SourceIndexedTypeSystem: TypeSystem {
    /// Look up a definition by its source file location and name.
    fn get_by_source(&self, location: &str, name: &str) -> Option<DefinitionRef>;

    /// Get the source identifier for a definition URI.
    fn source_identifier(&self, uri: &str) -> Option<&SourceIdentifier>;

    /// Get all definitions originating from a given source location.
    fn definitions_at_location(&self, location: &str) -> HashMap<String, DefinitionRef>;
}

/// A flat collection of Thrift type definitions with URI-based lookup.
pub struct BasicTypeSystem {
    definitions: HashMap<String, DefinitionNode>,
    source_by_location: HashMap<String, HashMap<String, String>>,
    uri_to_source: HashMap<String, SourceIdentifier>,
}

impl BasicTypeSystem {
    pub(crate) fn new(
        definitions: HashMap<String, DefinitionNode>,
        uri_to_source: HashMap<String, SourceIdentifier>,
    ) -> Self {
        let mut source_by_location: HashMap<String, HashMap<String, String>> = HashMap::new();
        for (uri, src) in &uri_to_source {
            source_by_location
                .entry(src.location.clone())
                .or_default()
                .insert(src.name.clone(), uri.clone());
        }
        Self {
            definitions,
            source_by_location,
            uri_to_source,
        }
    }
}

impl TypeSystem for BasicTypeSystem {
    fn get(&self, uri: &str) -> Option<DefinitionRef> {
        self.definitions.get(uri).map(|n| n.to_definition_ref())
    }

    fn known_uris(&self) -> Box<dyn Iterator<Item = &str> + '_> {
        Box::new(self.definitions.keys().map(|s| s.as_str()))
    }

    fn resolve(&self, type_id: &TypeId) -> Result<TypeRef, InvalidTypeError> {
        resolve_type_id(self, type_id)
    }

    fn to_serializable(&self) -> type_system::SerializableTypeSystem {
        crate::serialize::to_serializable_with_source(self, &self.uri_to_source)
    }
}

impl SourceIndexedTypeSystem for BasicTypeSystem {
    fn get_by_source(&self, location: &str, name: &str) -> Option<DefinitionRef> {
        self.source_by_location
            .get(location)
            .and_then(|names| names.get(name))
            .and_then(|uri| self.get(uri))
    }

    fn source_identifier(&self, uri: &str) -> Option<&SourceIdentifier> {
        self.uri_to_source.get(uri)
    }

    fn definitions_at_location(&self, location: &str) -> HashMap<String, DefinitionRef> {
        self.source_by_location
            .get(location)
            .map(|names| {
                names
                    .iter()
                    .filter_map(|(name, uri)| self.get(uri).map(|def| (name.clone(), def)))
                    .collect()
            })
            .unwrap_or_default()
    }
}

impl type_system_digest::TypeSystemDigest for BasicTypeSystem {
    fn hash_into(&self, hasher: &mut type_system_digest::hasher::Hasher) {
        crate::digest::hash_type_system_into(self, hasher);
    }
}

impl std::fmt::Debug for BasicTypeSystem {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BasicTypeSystem")
            .field("definitions", &self.definitions.keys().collect::<Vec<_>>())
            .finish()
    }
}

/// A type system that overlays definitions on top of a base.
///
/// Lookups check the overlay first, then fall back to the base.
/// Generic over the base, enabling both static and dynamic dispatch:
///
/// - `LayeredTypeSystem<BasicTypeSystem>` — zero-cost static dispatch
/// - `LayeredTypeSystem<Arc<dyn TypeSystem + Send + Sync>>` — dynamic dispatch
/// - `LayeredTypeSystem<LayeredTypeSystem<BasicTypeSystem>>` — multi-layer
pub struct LayeredTypeSystem<T: TypeSystem> {
    overlay: BasicTypeSystem,
    base: T,
}

impl<T: TypeSystem> LayeredTypeSystem<T> {
    pub fn new(overlay: BasicTypeSystem, base: T) -> Self {
        Self { overlay, base }
    }

    /// Access the base type system.
    pub fn base(&self) -> &T {
        &self.base
    }
}

impl<T: TypeSystem> TypeSystem for LayeredTypeSystem<T> {
    fn get(&self, uri: &str) -> Option<DefinitionRef> {
        self.overlay.get(uri).or_else(|| self.base.get(uri))
    }

    fn known_uris(&self) -> Box<dyn Iterator<Item = &str> + '_> {
        Box::new(
            self.overlay.known_uris().chain(
                self.base
                    .known_uris()
                    .filter(|uri| self.overlay.get(uri).is_none()),
            ),
        )
    }

    fn resolve(&self, type_id: &TypeId) -> Result<TypeRef, InvalidTypeError> {
        resolve_type_id(self, type_id)
    }

    fn to_serializable(&self) -> type_system::SerializableTypeSystem {
        // Merge base then overlay so source identifiers from every layer are
        // preserved; overlay entries win on URI collisions, matching the
        // overlay-first lookup precedence.
        let mut serialized = self.base.to_serializable();
        let overlay = self.overlay.to_serializable();
        serialized.types.extend(overlay.types);
        serialized
    }
}

impl<T: SourceIndexedTypeSystem> SourceIndexedTypeSystem for LayeredTypeSystem<T> {
    fn get_by_source(&self, location: &str, name: &str) -> Option<DefinitionRef> {
        self.overlay
            .get_by_source(location, name)
            .or_else(|| self.base.get_by_source(location, name))
    }

    fn source_identifier(&self, uri: &str) -> Option<&SourceIdentifier> {
        self.overlay
            .source_identifier(uri)
            .or_else(|| self.base.source_identifier(uri))
    }

    fn definitions_at_location(&self, location: &str) -> HashMap<String, DefinitionRef> {
        let mut result = self.base.definitions_at_location(location);
        for (name, def) in self.overlay.definitions_at_location(location) {
            result.insert(name, def);
        }
        result
    }
}

impl<T: TypeSystem> type_system_digest::TypeSystemDigest for LayeredTypeSystem<T> {
    fn hash_into(&self, hasher: &mut type_system_digest::hasher::Hasher) {
        crate::digest::hash_type_system_into(self, hasher);
    }
}

impl<T: TypeSystem + std::fmt::Debug> std::fmt::Debug for LayeredTypeSystem<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("LayeredTypeSystem")
            .field("overlay", &self.overlay)
            .field("base", &self.base)
            .finish()
    }
}

// Also implement TypeSystem for Arc<dyn TypeSystem> so it can be used as a base.
impl TypeSystem for Arc<dyn TypeSystem + Send + Sync> {
    fn get(&self, uri: &str) -> Option<DefinitionRef> {
        (**self).get(uri)
    }

    fn known_uris(&self) -> Box<dyn Iterator<Item = &str> + '_> {
        (**self).known_uris()
    }

    fn resolve(&self, type_id: &TypeId) -> Result<TypeRef, InvalidTypeError> {
        (**self).resolve(type_id)
    }
}

/// Default `resolve` implementation usable by any `TypeSystem`.
pub(crate) fn resolve_type_id(
    ts: &(impl TypeSystem + ?Sized),
    type_id: &TypeId,
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
            let def = ts
                .get(uri)
                .ok_or_else(|| InvalidTypeError::UnknownUri(uri.clone()))?;
            Ok(TypeRef::from(def))
        }

        TypeId::listType(list_id) => {
            let element = list_id
                .elementType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let element_ref = resolve_type_id(ts, element)?;
            Ok(TypeRef::list_of(element_ref))
        }
        TypeId::setType(set_id) => {
            let element = set_id
                .elementType
                .as_ref()
                .ok_or(InvalidTypeError::EmptyTypeId)?;
            let element_ref = resolve_type_id(ts, element)?;
            Ok(TypeRef::set_of(element_ref))
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
            let key_ref = resolve_type_id(ts, key)?;
            let value_ref = resolve_type_id(ts, value)?;
            Ok(TypeRef::map_of(key_ref, value_ref))
        }

        TypeId::UnknownField(id) => Err(InvalidTypeError::UnknownUri(format!(
            "<unknown TypeId variant {id}>"
        ))),
    }
}
