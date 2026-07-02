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

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
include "thrift/lib/thrift/type_id.thrift"
include "thrift/lib/thrift/type_system.thrift"

package "facebook.com/thrift/type_system"

namespace cpp2 apache.thrift.type_system
namespace go thrift.lib.thrift.service_catalog
namespace py3 apache.thrift.type_system

/** The retry-safety contract of a function. */
enum FunctionQualifier {
  Unspecified = 0,
  Idempotent = 1,
  ReadOnly = 2,
}

/**
 * The shape of an RPC. Streaming kinds carry their payloads in the response's
 * `streaming` channel.
 */
enum RpcKind {
  Unary = 0,
  OneWay = 1,
  Stream = 2,
  Sink = 3,
  BidirectionalStream = 4,
}

/**
 * Parameters form the request payload as a struct whose fields carry these
 * identities.
 */
struct SerializableParameter {
  1: type_system.FieldIdentity identity;
  2: type_id.TypeId type;
}

/** The identity is the exception's position in the throws clause. */
struct SerializableException {
  1: type_system.FieldIdentity identity;
  2: type_id.TypeId type;
}

/** A stream of values returned to the client after the initial response. */
struct SerializableStream {
  1: type_id.TypeId payloadType;
  2: list<SerializableException> exceptions;
}

/**
 * A sink the client streams into, terminated by a final response from the
 * server.
 */
struct SerializableSink {
  1: type_id.TypeId payloadType;
  2: optional type_id.TypeId finalResponseType;
  3: list<SerializableException> clientExceptions;
  4: list<SerializableException> serverExceptions;
}

/**
 * The client streams into a sink while the server streams responses back
 * concurrently.
 */
struct SerializableBidirectionalStream {
  1: type_id.TypeId sinkPayloadType;
  2: type_id.TypeId streamPayloadType;
  3: list<SerializableException> sinkExceptions;
  4: list<SerializableException> streamExceptions;
}

union SerializableStreamingResponse {
  1: SerializableStream serverStream;
  2: SerializableSink clientSink;
  3: SerializableBidirectionalStream bidirectionalStream;
}

/**
 * Absent `initialResponseType` denotes a `void` response. `createsInteraction`,
 * when present, is the URI of the interaction this function begins.
 */
struct SerializableFunctionResponse {
  1: optional type_id.TypeId initialResponseType;
  2: optional SerializableStreamingResponse streaming;
  3: optional type_id.Uri createsInteraction;
}

struct SerializableFunction {
  1: string name;
  2: FunctionQualifier qualifier;
  3: list<SerializableParameter> params;
  4: SerializableFunctionResponse response;
  5: list<SerializableException> exceptions;
  6: RpcKind rpcKind;
}

/** `baseService` is the URI of the service this one inherits from. */
struct SerializableServiceDefinition {
  1: list<SerializableFunction> functions;
  2: optional type_id.Uri baseService;
}

/**
 * An RPC interface scoped to the lifetime of an interaction begun by a
 * function.
 */
struct SerializableInteractionDefinition {
  1: list<SerializableFunction> functions;
}

union SerializableRpcInterfaceDefinition {
  1: SerializableServiceDefinition serviceDef;
  2: SerializableInteractionDefinition interactionDef;
}

/**
 * A self-contained schema for a set of Thrift RPC interfaces: every `TypeId`
 * and interface URI reachable from `interfaces` resolves within the embedded
 * `types`, so it can be shared and rebuilt without external lookups.
 */
struct SerializableServiceCatalog {
  1: type_system.SerializableTypeSystem types;
  2: map<type_id.Uri, SerializableRpcInterfaceDefinition> interfaces;
}
