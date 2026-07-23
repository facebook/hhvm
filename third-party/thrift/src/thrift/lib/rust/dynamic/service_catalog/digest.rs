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

//! `ServiceCatalogDigest` impls for runtime service catalog types.

use std::collections::BTreeMap;

use service_catalog_digest::BIDIRECTIONAL_STREAM_FIELD_ID;
use service_catalog_digest::CLIENT_SINK_FIELD_ID;
use service_catalog_digest::INTERACTION_DEF_FIELD_ID;
use service_catalog_digest::SERVER_STREAM_FIELD_ID;
use service_catalog_digest::SERVICE_CATALOG_DIGEST_VERSION;
use service_catalog_digest::SERVICE_DEF_FIELD_ID;
use service_catalog_digest::ServiceCatalogDigest;
use service_catalog_digest::hasher::DigestHasherExt as _;
use service_catalog_digest::hasher::Hasher;

use crate::Exception;
use crate::Function;
use crate::Interaction;
use crate::InteractionDefinition;
use crate::Parameter;
use crate::RpcInterfaceDefinition;
use crate::ServiceCatalog;
use crate::ServiceDefinition;
use crate::ServiceDescriptor;
use crate::Sink;
use crate::Stream;
use crate::TypeUniverse;

impl ServiceCatalogDigest for TypeUniverse {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.type_digest_with_mode(h.mode()));
    }
}

impl ServiceCatalogDigest for ServiceCatalog {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&SERVICE_CATALOG_DIGEST_VERSION);
        h.hash(&self.type_universe);
        h.hash(&CatalogInterfaces(&self.interfaces));
    }
}

impl ServiceCatalogDigest for ServiceDescriptor {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&SERVICE_CATALOG_DIGEST_VERSION);
        h.hash(&self.type_universe);
        h.hash(&DescriptorInterfaces(self));
    }
}

impl ServiceCatalogDigest for RpcInterfaceDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        match self {
            Self::Service(service) => {
                h.hash(&SERVICE_DEF_FIELD_ID);
                h.hash(service);
            }
            Self::Interaction(interaction) => {
                h.hash(&INTERACTION_DEF_FIELD_ID);
                h.hash(interaction);
            }
        }
    }
}

impl ServiceCatalogDigest for ServiceDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&Functions(&self.functions));
        h.hash(&self.base_service);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for InteractionDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&Functions(&self.functions));
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for Interaction {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&Functions(&self.functions));
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for Parameter {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.id);
        h.hash(self.name.as_str());
        h.hash(&self.type_id);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for Exception {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.id);
        h.hash(self.name.as_str());
        h.hash(&self.type_id);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for Stream {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.payload_type);
        h.hash(&Exceptions(&self.exceptions));
    }
}

impl ServiceCatalogDigest for Sink {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.payload_type);
        h.hash(&self.final_response_type);
        h.hash(&Exceptions(&self.client_exceptions));
        h.hash(&Exceptions(&self.server_exceptions));
    }
}

impl ServiceCatalogDigest for Function {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.name.as_str());
        h.hash(&self.qualifier.0);
        h.hash(&Parameters(&self.params));
        h.hash(&FunctionResponse(self));
        h.hash(&Exceptions(&self.exceptions));
        h.hash(&self.rpc_kind.0);
        h.hash(&self.annotations);
    }
}

struct CatalogInterfaces<'a>(&'a BTreeMap<String, RpcInterfaceDefinition>);

impl ServiceCatalogDigest for CatalogInterfaces<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_in_key_order(self.0, |sub_h, uri, interface| {
            sub_h.hash(uri.as_str());
            sub_h.hash(interface);
        });
    }
}

struct Functions<'a>(&'a [Function]);

impl ServiceCatalogDigest for Functions<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.0.iter(),
            |function| function.name.as_str(),
            |sub_h, _, function| {
                sub_h.hash(*function);
            },
        );
    }
}

struct Parameters<'a>(&'a [Parameter]);

impl ServiceCatalogDigest for Parameters<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.0.iter(),
            |param| param.id,
            |sub_h, _, param| {
                sub_h.hash(*param);
            },
        );
    }
}

struct Exceptions<'a>(&'a [Exception]);

impl ServiceCatalogDigest for Exceptions<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.0.iter(),
            |ex| ex.id,
            |sub_h, _, ex| {
                sub_h.hash(*ex);
            },
        );
    }
}

struct DescriptorInterfaces<'a>(&'a ServiceDescriptor);

impl ServiceCatalogDigest for DescriptorInterfaces<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        enum InterfaceEntry<'a> {
            Service(&'a ServiceDescriptor),
            Interaction(&'a Interaction),
        }

        impl InterfaceEntry<'_> {
            fn uri(&self) -> &str {
                match self {
                    Self::Service(descriptor) => descriptor.service_uri.as_str(),
                    Self::Interaction(interaction) => interaction.uri.as_str(),
                }
            }
        }

        let descriptor = self.0;
        let mut interfaces = Vec::with_capacity(1 + descriptor.interactions.len());
        interfaces.push(InterfaceEntry::Service(descriptor));
        interfaces.extend(
            descriptor
                .interactions
                .iter()
                .map(InterfaceEntry::Interaction),
        );
        h.hash_ordered_by_key(
            interfaces,
            |interface| interface.uri().to_owned(),
            |sub_h, uri, interface| {
                sub_h.hash(uri.as_str());
                match interface {
                    InterfaceEntry::Service(descriptor) => {
                        sub_h.hash(&SERVICE_DEF_FIELD_ID);
                        sub_h.hash(&DescriptorServiceDefinition(descriptor));
                    }
                    InterfaceEntry::Interaction(interaction) => {
                        sub_h.hash(&INTERACTION_DEF_FIELD_ID);
                        sub_h.hash(*interaction);
                    }
                }
            },
        );
    }
}

struct DescriptorServiceDefinition<'a>(&'a ServiceDescriptor);

impl ServiceCatalogDigest for DescriptorServiceDefinition<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&Functions(&self.0.functions));
        h.hash(&false);
        h.hash(&self.0.annotations);
    }
}

struct FunctionResponse<'a>(&'a Function);

impl ServiceCatalogDigest for FunctionResponse<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        let function = self.0;
        h.hash(&function.response_type);
        match (&function.stream, &function.sink) {
            (Some(stream), Some(sink)) => {
                h.hash(&true);
                h.hash(&BIDIRECTIONAL_STREAM_FIELD_ID);
                h.hash(&BidirectionalStreamingResponse { stream, sink });
            }
            (Some(stream), None) => {
                h.hash(&true);
                h.hash(&SERVER_STREAM_FIELD_ID);
                h.hash(stream);
            }
            (None, Some(sink)) => {
                h.hash(&true);
                h.hash(&CLIENT_SINK_FIELD_ID);
                h.hash(sink);
            }
            (None, None) => h.hash(&false),
        }
        h.hash(&function.created_interaction_uri);
    }
}

struct BidirectionalStreamingResponse<'a> {
    stream: &'a Stream,
    sink: &'a Sink,
}

impl ServiceCatalogDigest for BidirectionalStreamingResponse<'_> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.sink.payload_type);
        h.hash(&self.stream.payload_type);
        h.hash(&Exceptions(&self.sink.client_exceptions));
        h.hash(&Exceptions(&self.stream.exceptions));
    }
}
