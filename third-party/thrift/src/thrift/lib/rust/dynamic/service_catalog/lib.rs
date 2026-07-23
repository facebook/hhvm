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

//! Runtime representation of Thrift service catalogs.
//!
//! These structs model the service-descriptor projection of
//! `service_catalog.thrift` and keep annotation payloads in serialized form.

use std::collections::BTreeMap;

mod digest;

#[cfg(test)]
mod tests;

use record::SerializableRecord;
pub use service_catalog::FunctionQualifier;
pub use service_catalog::RpcKind;
use type_id::TypeId;
pub use type_system_digest::Digest;
pub use type_system_digest::DigestMode;
use type_system_digest::TypeSystemDigest as _;

/// URI to serialized annotation record.
pub type AnnotationsMap = BTreeMap<String, SerializableRecord>;

/// The type universe referenced by service interfaces.
#[derive(Clone, Debug, PartialEq)]
pub enum TypeUniverse {
    Inline(type_system::SerializableTypeSystem),
    Digest(Digest),
}

impl TypeUniverse {
    pub fn type_digest_with_mode(&self, mode: DigestMode) -> Digest {
        match self {
            Self::Inline(types) => types.digest_with_mode(mode),
            Self::Digest(digest) => *digest,
        }
    }
}

impl From<type_system::SerializableTypeSystem> for TypeUniverse {
    fn from(types: type_system::SerializableTypeSystem) -> Self {
        Self::Inline(types)
    }
}

impl From<Digest> for TypeUniverse {
    fn from(digest: Digest) -> Self {
        Self::Digest(digest)
    }
}

/// Runtime service catalog projection.
#[derive(Clone, Debug, PartialEq)]
pub struct ServiceCatalog {
    pub type_universe: TypeUniverse,
    pub interfaces: BTreeMap<String, RpcInterfaceDefinition>,
}

/// Runtime service descriptor projection.
#[derive(Clone, Debug, PartialEq)]
pub struct ServiceDescriptor {
    pub service_uri: String,
    pub type_universe: TypeUniverse,
    pub functions: Vec<Function>,
    pub interactions: Vec<Interaction>,
    pub annotations: AnnotationsMap,
}

impl ServiceDescriptor {
    pub fn new(service_uri: impl Into<String>, type_universe: TypeUniverse) -> Self {
        Self {
            service_uri: service_uri.into(),
            type_universe,
            functions: Vec::new(),
            interactions: Vec::new(),
            annotations: AnnotationsMap::new(),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum RpcInterfaceDefinition {
    Service(ServiceDefinition),
    Interaction(InteractionDefinition),
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct ServiceDefinition {
    pub functions: Vec<Function>,
    pub base_service: Option<String>,
    pub annotations: AnnotationsMap,
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct InteractionDefinition {
    pub functions: Vec<Function>,
    pub annotations: AnnotationsMap,
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct Interaction {
    pub uri: String,
    pub functions: Vec<Function>,
    pub annotations: AnnotationsMap,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter {
    pub id: i16,
    pub name: String,
    pub type_id: TypeId,
    pub annotations: AnnotationsMap,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Exception {
    pub id: i16,
    pub name: String,
    pub type_id: TypeId,
    pub annotations: AnnotationsMap,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Stream {
    pub payload_type: TypeId,
    pub exceptions: Vec<Exception>,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Sink {
    pub payload_type: TypeId,
    pub final_response_type: Option<TypeId>,
    pub client_exceptions: Vec<Exception>,
    pub server_exceptions: Vec<Exception>,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Function {
    pub name: String,
    pub params: Vec<Parameter>,
    pub response_type: Option<TypeId>,
    pub exceptions: Vec<Exception>,
    pub stream: Option<Stream>,
    pub sink: Option<Sink>,
    pub qualifier: FunctionQualifier,
    pub rpc_kind: RpcKind,
    pub created_interaction_uri: Option<String>,
    pub annotations: AnnotationsMap,
}

impl Default for Function {
    fn default() -> Self {
        Self {
            name: String::new(),
            params: Vec::new(),
            response_type: None,
            exceptions: Vec::new(),
            stream: None,
            sink: None,
            qualifier: FunctionQualifier(0),
            rpc_kind: RpcKind(0),
            created_interaction_uri: None,
            annotations: AnnotationsMap::new(),
        }
    }
}
