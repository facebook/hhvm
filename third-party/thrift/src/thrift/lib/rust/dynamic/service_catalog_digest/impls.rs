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

//! [`ServiceCatalogDigest`] trait implementations for service catalog types.
//!
//! Every implementation here must produce byte-identical hash output to the
//! corresponding C++ code in `ServiceCatalogDigest.cpp`.

use std::collections::BTreeMap;

use record::SerializableRecord;
use service_catalog::SerializableBidirectionalStream;
use service_catalog::SerializableException;
use service_catalog::SerializableFunction;
use service_catalog::SerializableFunctionResponse;
use service_catalog::SerializableInteractionDefinition;
use service_catalog::SerializableParameter;
use service_catalog::SerializableRpcInterfaceDefinition;
use service_catalog::SerializableServiceCatalog;
use service_catalog::SerializableServiceDefinition;
use service_catalog::SerializableSink;
use service_catalog::SerializableStream;
use service_catalog::SerializableStreamingResponse;
use type_id::TypeId;
use type_system_digest::TypeSystemDigest as _;

use crate::BIDIRECTIONAL_STREAM_FIELD_ID;
use crate::CLIENT_SINK_FIELD_ID;
use crate::Digest;
use crate::Exception;
use crate::Function;
use crate::INTERACTION_DEF_FIELD_ID;
use crate::Interaction;
use crate::InteractionDefinition;
use crate::Parameter;
use crate::RpcInterfaceDefinition;
use crate::SERVER_STREAM_FIELD_ID;
use crate::SERVICE_CATALOG_DIGEST_VERSION;
use crate::SERVICE_DEF_FIELD_ID;
use crate::ServiceCatalog;
use crate::ServiceCatalogDigest;
use crate::ServiceDefinition;
use crate::ServiceDescriptor;
use crate::Sink;
use crate::Stream;
use crate::TypeUniverse;
use crate::hasher::DigestHasherExt as _;
use crate::hasher::Hasher;

impl ServiceCatalogDigest for TypeId {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for SerializableRecord {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for TypeUniverse {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.type_digest_with_mode(h.mode()));
    }
}

impl ServiceCatalogDigest for ServiceCatalog {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&SERVICE_CATALOG_DIGEST_VERSION);
        h.hash(&self.type_universe);
        h.hash(&self.interfaces);
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
        h.hash(self.functions.as_slice());
        h.hash(&self.base_service);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for InteractionDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.functions.as_slice());
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for Interaction {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.functions.as_slice());
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
        h.hash(self.exceptions.as_slice());
    }
}

impl ServiceCatalogDigest for Sink {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.payload_type);
        h.hash(&self.final_response_type);
        h.hash(self.client_exceptions.as_slice());
        h.hash(self.server_exceptions.as_slice());
    }
}

impl ServiceCatalogDigest for Function {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.name.as_str());
        h.hash(&self.qualifier.0);
        h.hash(self.params.as_slice());
        h.hash(&FunctionResponse(self));
        h.hash(self.exceptions.as_slice());
        h.hash(&self.rpc_kind.0);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for SerializableServiceCatalog {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&SERVICE_CATALOG_DIGEST_VERSION);
        match &self.types {
            Some(types) => h.update(&types.digest_with_mode(h.mode())),
            None => h.update(&external_type_system_digest(&self.typesDigest)),
        }
        h.hash(&self.interfaces);
    }
}

impl ServiceCatalogDigest for SerializableRpcInterfaceDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&rpc_interface_field_id(self));
        match self {
            Self::serviceDef(service) => h.hash(service),
            Self::interactionDef(interaction) => h.hash(interaction),
            Self::UnknownField(id) => {
                panic!(
                    "ServiceCatalogDigest: unhandled SerializableRpcInterfaceDefinition variant (field {id})"
                )
            }
        }
    }
}

impl ServiceCatalogDigest for SerializableServiceDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.functions.as_slice());
        h.hash(&self.baseService);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for SerializableInteractionDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.functions.as_slice());
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for SerializableFunction {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.name.as_str());
        h.hash(&self.qualifier.0);
        h.hash(self.params.as_slice());
        h.hash(&self.response);
        h.hash(self.exceptions.as_slice());
        h.hash(&self.rpcKind.0);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for SerializableFunctionResponse {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.initialResponseType);
        h.hash(&self.streaming);
        h.hash(&self.createsInteraction);
    }
}

impl ServiceCatalogDigest for SerializableStreamingResponse {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&streaming_response_field_id(self));
        match self {
            Self::serverStream(stream) => h.hash(stream),
            Self::clientSink(sink) => h.hash(sink),
            Self::bidirectionalStream(bidi) => h.hash(bidi),
            Self::UnknownField(id) => {
                panic!(
                    "ServiceCatalogDigest: unhandled SerializableStreamingResponse variant (field {id})"
                )
            }
        }
    }
}

impl ServiceCatalogDigest for SerializableStream {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.payloadType);
        h.hash(self.exceptions.as_slice());
    }
}

impl ServiceCatalogDigest for SerializableSink {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.payloadType);
        h.hash(&self.finalResponseType);
        h.hash(self.clientExceptions.as_slice());
        h.hash(self.serverExceptions.as_slice());
    }
}

impl ServiceCatalogDigest for SerializableBidirectionalStream {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.sinkPayloadType);
        h.hash(&self.streamPayloadType);
        h.hash(self.sinkExceptions.as_slice());
        h.hash(self.streamExceptions.as_slice());
    }
}

impl ServiceCatalogDigest for SerializableParameter {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.identity.id);
        h.hash(self.identity.name.as_str());
        h.hash(&self.r#type);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for SerializableException {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.identity.id);
        h.hash(self.identity.name.as_str());
        h.hash(&self.r#type);
        h.hash(&self.annotations);
    }
}

impl ServiceCatalogDigest for BTreeMap<String, RpcInterfaceDefinition> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_in_key_order(self, |sub_h, uri, interface| {
            sub_h.hash(uri.as_str());
            sub_h.hash(interface);
        });
    }
}

impl ServiceCatalogDigest for BTreeMap<String, SerializableRpcInterfaceDefinition> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_in_key_order(self, |sub_h, uri, interface| {
            sub_h.hash(uri.as_str());
            sub_h.hash(interface);
        });
    }
}

impl ServiceCatalogDigest for BTreeMap<String, SerializableRecord> {
    fn hash_into(&self, h: &mut Hasher) {
        if !h.include_annotations() {
            return;
        }
        h.hash_unordered_by_digest(self.iter(), |sub_h, (key, value)| {
            sub_h.hash(key.as_str());
            sub_h.hash(*value);
        });
    }
}

impl ServiceCatalogDigest for [Function] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |function| function.name.as_str(),
            |sub_h, _, function| {
                sub_h.hash(*function);
            },
        );
    }
}

impl ServiceCatalogDigest for [SerializableFunction] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |function| function.name.as_str(),
            |sub_h, _, function| {
                sub_h.hash(*function);
            },
        );
    }
}

impl ServiceCatalogDigest for [Parameter] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |param| param.id,
            |sub_h, _, param| {
                sub_h.hash(*param);
            },
        );
    }
}

impl ServiceCatalogDigest for [SerializableParameter] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |param| param.identity.id,
            |sub_h, _, param| {
                sub_h.hash(*param);
            },
        );
    }
}

impl ServiceCatalogDigest for [Exception] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |ex| ex.id,
            |sub_h, _, ex| {
                sub_h.hash(*ex);
            },
        );
    }
}

impl ServiceCatalogDigest for [SerializableException] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_ordered_by_key(
            self.iter(),
            |ex| ex.identity.id,
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
        h.hash(self.0.functions.as_slice());
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
        h.hash(self.sink.client_exceptions.as_slice());
        h.hash(self.stream.exceptions.as_slice());
    }
}

fn external_type_system_digest(bytes: &[u8]) -> Digest {
    bytes.try_into().unwrap_or_else(|_| {
        panic!(
            "SerializableServiceCatalog has no valid type system digest: expected {} bytes, got {}",
            std::mem::size_of::<Digest>(),
            bytes.len()
        )
    })
}

fn streaming_response_field_id(response: &SerializableStreamingResponse) -> i32 {
    match response {
        SerializableStreamingResponse::serverStream(_) => SERVER_STREAM_FIELD_ID,
        SerializableStreamingResponse::clientSink(_) => CLIENT_SINK_FIELD_ID,
        SerializableStreamingResponse::bidirectionalStream(_) => BIDIRECTIONAL_STREAM_FIELD_ID,
        SerializableStreamingResponse::UnknownField(id) => {
            panic!(
                "ServiceCatalogDigest: unhandled SerializableStreamingResponse variant (field {id})"
            )
        }
    }
}

fn rpc_interface_field_id(interface: &SerializableRpcInterfaceDefinition) -> i32 {
    match interface {
        SerializableRpcInterfaceDefinition::serviceDef(_) => SERVICE_DEF_FIELD_ID,
        SerializableRpcInterfaceDefinition::interactionDef(_) => INTERACTION_DEF_FIELD_ID,
        SerializableRpcInterfaceDefinition::UnknownField(id) => {
            panic!(
                "ServiceCatalogDigest: unhandled SerializableRpcInterfaceDefinition variant (field {id})"
            )
        }
    }
}
