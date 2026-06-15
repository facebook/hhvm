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

/// Core interface for looking up Thrift type definitions.
pub trait TypeSystem {
    /// Look up a user-defined type by URI.
    fn get(&self, uri: &str) -> Option<DefinitionRef>;

    /// Iterate over all known URIs.
    fn known_uris(&self) -> Box<dyn Iterator<Item = &str> + '_>;

    /// Resolve a serializable `TypeId` to a live `TypeRef`.
    fn resolve(&self, type_id: &TypeId) -> Result<TypeRef, InvalidTypeError>;
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

/// A flat collection of Thrift type definitions with URI-based lookup.
pub struct BasicTypeSystem {
    definitions: HashMap<String, DefinitionNode>,
}

impl BasicTypeSystem {
    pub(crate) fn new(definitions: HashMap<String, DefinitionNode>) -> Self {
        Self { definitions }
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
}

impl std::fmt::Debug for BasicTypeSystem {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BasicTypeSystem")
            .field("definitions", &self.definitions.keys().collect::<Vec<_>>())
            .finish()
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
