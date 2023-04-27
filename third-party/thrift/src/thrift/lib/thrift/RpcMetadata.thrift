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

/*
 * This file contains structs that are meant to be used in the Thrift transport
 * wire protocol. Keywords used in this file conform to the meanings in RFC 2119.
 */

namespace cpp2 apache.thrift
namespace java.swift org.apache.thrift
namespace java javadeprecated.org.apache.thrift
namespace php Thrift_RpcMetadata
namespace py thrift.lib.thrift.RpcMetadata
namespace py.asyncio thrift.lib.thrift.asyncio.RpcMetadata
namespace py3 thrift.lib.thrift
namespace go thrift.lib.thrift.RpcMetadata

cpp_include "thrift/lib/cpp2/util/ManagedStringView.h"
cpp_include "thrift/lib/thrift/RpcMetadata_extra.h"
cpp_include "folly/container/F14Map.h"

typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

enum ProtocolId {
  // The values must match those in thrift/lib/cpp/protocol/TProtocolTypes.h
  BINARY = 0,
  COMPACT = 2,
// Deprecated.
// FROZEN2 = 6,
}

enum RpcKind {
  SINGLE_REQUEST_SINGLE_RESPONSE = 0,
  SINGLE_REQUEST_NO_RESPONSE = 1,
  // Unused:
  // STREAMING_REQUEST_SINGLE_RESPONSE = 2,
  // STREAMING_REQUEST_NO_RESPONSE = 3,
  SINGLE_REQUEST_STREAMING_RESPONSE = 4,
  // STREAMING_REQUEST_STREAMING_RESPONSE = 5,
  SINK = 6,
}

enum RpcPriority {
  HIGH_IMPORTANT = 0,
  HIGH = 1,
  IMPORTANT = 2,
  NORMAL = 3,
  BEST_EFFORT = 4,
  // This should be the immediately after the last enumerator.
  N_PRIORITIES = 5,
}

enum CompressionAlgorithm {
  NONE = 0,
  ZLIB = 1,
  ZSTD = 2,
}

enum ErrorKind {
  UNSPECIFIED = 0,
  TRANSIENT = 1,
  STATEFUL = 2,
  PERMANENT = 3,
}

enum ErrorBlame {
  UNSPECIFIED = 0,
  SERVER = 1,
  CLIENT = 2,
}

enum ErrorSafety {
  UNSPECIFIED = 0,
  SAFE = 1,
}

struct ZlibCompressionCodecConfig {}

struct ZstdCompressionCodecConfig {}

union CodecConfig {
  1: ZlibCompressionCodecConfig zlibConfig;
  2: ZstdCompressionCodecConfig zstdConfig;
}

struct CompressionConfig {
  // Defines which codec SHOULD be used for compression and configuration
  // specific to that codec. SHOULD be set.
  1: optional CodecConfig codecConfig;
  // Defines the minimum payload size that SHOULD trigger compression.
  2: optional i64 compressionSizeLimit;
}

// A TLS extension used for thrift parameters negotiation during TLS handshake.
// All fields should be optional.
// For fields which are conceptually list<bool>, i64 bitmap is frequently preferred.
struct NegotiationParameters {
  // nth (zero-based) least significant bit set if CompressionAlgorithm = n + 1
  // is accepted. For example, 0b10 means ZSTD is accepted.
  1: optional i64 (cpp.type = "std::uint64_t") compressionAlgos;
  2: optional bool useStopTLS;
}

// String type optimized for generated code
typedef string (
  cpp.type = "::apache::thrift::ManagedStringViewWithConversions",
) ManagedStringViewField

struct InteractionCreate {
  // Client chosen interaction id. Interaction id MAY be reused after the
  // interaction previously using that id was terminated. MUST be unique per
  // connection. MUST be > 0.
  1: i64 interactionId;
  2: ManagedStringViewField interactionName;
}
struct InteractionTerminate {
  // Interaction id of an interaction previously created on the same
  // connection.
  1: i64 interactionId;
}

struct RequestRpcMetadata {
  // The protocol using which the request payload has been serialized. MUST be
  // set.
  // TODO: this should be extended to support unset protocol field (e.g. unset
  // could mean compact).
  1: optional ProtocolId protocol;
  // The name of the RPC function. MUST be set.
  2: optional ManagedStringViewField name;
  // The kind of RPC. MUST be set.
  // TODO: this should be extended to support unset kind field (e.g. RPC kind
  // could be derived from the underlying transport message type).
  3: optional RpcKind kind;
  // 4: Deprecated
  // The amount of time that a client will wait for a response from
  // the server. MAY be set.
  5: optional i32 clientTimeoutMs;
  // The maximum amount of time that a request SHOULD wait at the
  // server before it is handled. MAY be set.
  6: optional i32 queueTimeoutMs;
  // Overrides the default priority of this RPC. MAY be set.
  7: optional RpcPriority priority;
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  8: optional map<string, string> (
    cpp.template = "folly::F14NodeMap",
  ) otherMetadata;
  // 9: Deprecated
  // 10: Deprecated
  // The CRC32C of the uncompressed request payload. MAY be set. SHOULD be
  // validated if set.
  11: optional i32 (cpp.type = "std::uint32_t") crc32c;
  // 12: Deprecated
  // The name of the load metric that should be queried as part of this
  // request. MAY be set. If set, load field SHOULD be set in response
  // metadata.
  13: optional string loadMetric;
  // The compression algorithm that was used to compress request payload. MUST
  // be set iff request payload is compressed.
  14: optional CompressionAlgorithm compression;
  // Requested compression policy for the response. MAY be set.
  15: optional CompressionConfig compressionConfig;
  // Id of an existing interaction which this RPC belongs to. MUST NOT be set
  // if interactionCreate is set. One of interactionId and interactionCreate
  // MUST be set iff requested method belongs to an interaction.
  16: optional i64 interactionId;
  // Information needed to create a new interaction which this RPC will be
  // assigned to. MUST NOT be set if interactionId is set. One of interactionId
  // and interactionCreate MUST be set iff requested method belongs to an
  // interaction.
  17: optional InteractionCreate interactionCreate;
  18: optional string clientId;
  19: optional string serviceTraceMeta;
  // Thrift is typically used within a larger framework.
  // This field is for storing framework-specific metadata.
  20: optional IOBufPtr frameworkMetadata;
}

struct ErrorClassification {
  1: optional ErrorKind kind;
  2: optional ErrorBlame blame;
  3: optional ErrorSafety safety;
}

struct PayloadResponseMetadata {}

struct PayloadDeclaredExceptionMetadata {
  1: optional ErrorClassification errorClassification;
}

struct PayloadProxyExceptionMetadata {}

struct PayloadProxiedExceptionMetadata {}

struct PayloadAppUnknownExceptionMetdata {
  1: optional ErrorClassification errorClassification;
}

struct PayloadAnyExceptionMetadata {}

union PayloadExceptionMetadata {
  // If set response payload MUST contain serialized declared exception.
  1: PayloadDeclaredExceptionMetadata declaredException;
  // If set response payload SHOULD contain exception serialized in a format
  // defined by a proxy.
  // Deprecated:
  // replaced by PayloadAnyExceptionMetadata
  2: PayloadProxyExceptionMetadata DEPRECATED_proxyException;
  // 3: Deprecated
  // 4: Deprecated
  // 5: Deprecated
  // If set response payload SHOULD be empty.
  // Supported in Rocket protocol version 8+
  6: PayloadAppUnknownExceptionMetdata appUnknownException;
  // If set response payload MUST contain a serialized SemiAnyStruct
  // (see thrift/lib/thrift/any_rep.thrift) containing an exception.
  // If SemiAnyStruct doesn't have the protocol set, the protocol MUST
  // match the protocol used to serialize the SemiAnyStruct.
  // Supported in Rocket protocol version 10+
  7: PayloadAnyExceptionMetadata anyException;
}

struct PayloadExceptionMetadataBase {
  1: optional string name_utf8;
  2: optional string what_utf8;
  // MAY be set. If not set SHOULD be treated as empty
  // PayloadAppUnknownExceptionMetdata.
  3: optional PayloadExceptionMetadata metadata;
}

union PayloadMetadata {
  // If set response payload MUST contain serialized response.
  1: PayloadResponseMetadata responseMetadata;
  2: PayloadExceptionMetadataBase exceptionMetadata;
}

struct ProxiedPayloadMetadata {}

struct QueueMetadata {
  // Total time in milliseconds spent in the executor's queue
  1: i32 queueingTimeMs;
  // Whether queue timeout was set (either by the client or the server).
  // If it is set, this will contain the actual value.
  2: optional i32 queueTimeoutMs;
}

struct ResponseRpcMetadata {
  // 1: Deprecated
  // 2: Deprecated
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  3: optional map<string, string> (
    cpp.template = "folly::F14NodeMap",
  ) otherMetadata;
  // Server load. SHOULD be set iff loadMetric was set in RequestRpcMetadata
  4: optional i64 load;
  // The CRC32C of the uncompressed response payload. MAY be set. SHOULD be
  // validated if set.
  5: optional i32 (cpp.type = "std::uint32_t") crc32c;
  // The compression algorithm that was used to compress response payload. MUST
  // be set iff response payload is compressed. SHOULD match the algorithm set
  // in compressionConfig in RequestRpcMetadata
  6: optional CompressionAlgorithm compression;
  // Metadata describing the type of response payload. MUST be set.
  7: optional PayloadMetadata payloadMetadata;
  // Metadata describing proxied requests. SHOULD be set iff request is
  // proxied.
  8: optional ProxiedPayloadMetadata proxiedPayloadMetadata;
  // SHOULD be set for streaming RPCs. MUST match the stream id used by the
  // underlying transport (e.g. RSocket).
  9: optional i32 streamId;
  // Set on a sampled basis for tracking queueing times.
  10: optional QueueMetadata queueMetadata;
}

enum ResponseRpcErrorCategory {
  // Server failed processing the request.
  // Server may have started processing the request.
  INTERNAL_ERROR = 0,
  // Server didn't process the request because the request was invalid.
  INVALID_REQUEST = 1,
  // Server didn't process the request because it didn't have the resources.
  // Request can be safely retried to a different server, or the same server
  // later.
  LOADSHEDDING = 2,
  // Server didn't process request because it was shutting down.
  // Request can be safely retried to a different server. Request should not
  // be retried to the same server.
  SHUTDOWN = 3,
}

// Each ResponseRpcErrorCode belongs to exactly one ResponseRpcErrorCategory.
// Default category is INTERNAL_ERROR unless specified otherwise.
enum ResponseRpcErrorCode {
  UNKNOWN = 0,
  // Server rejected the request because it is overloaded.
  // ResponseRpcErrorCategory::LOADSHEDDING
  OVERLOAD = 1,
  // Task timeout was hit before the server finished processing the request.
  TASK_EXPIRED = 2,
  // Server rejected the request because the queue was full.
  // ResponseRpcErrorCategory::LOADSHEDDING
  QUEUE_OVERLOADED = 3,
  // Server rejected the request because it was starting/shutting down.
  // ResponseRpcErrorCategory::SHUTDOWN
  SHUTDOWN = 4,
  INJECTED_FAILURE = 5,
  // ResponseRpcErrorCategory::INVALID_REQUEST
  REQUEST_PARSING_FAILURE = 6,
  // Server rejected the request because it spent too much time in the queue.
  // ResponseRpcErrorCategory::LOADSHEDDING
  QUEUE_TIMEOUT = 7,
  RESPONSE_TOO_BIG = 8,
  // Server rejected the request because the RPC kind for a given method name
  // didn't match the RPC kind of the method supported by the server.
  // ResponseRpcErrorCategory::INVALID_REQUEST
  WRONG_RPC_KIND = 9,
  // Server rejected the request because the requested method name is not known
  // to the server.
  // ResponseRpcErrorCategory::INVALID_REQUEST
  UNKNOWN_METHOD = 10,
  // ResponseRpcErrorCategory::INVALID_REQUEST
  CHECKSUM_MISMATCH = 11,
  INTERRUPTION = 12,
  // Server rejected the request because it is overloaded based on the
  // application defined dynamic overload criteria.
  // ResponseRpcErrorCategory::LOADSHEDDING
  APP_OVERLOAD = 13,
  UNKNOWN_INTERACTION_ID = 14,
  INTERACTION_CONSTRUCTOR_ERROR = 15,
  // Server rejected the request because the requested method name is not
  // implemented in the application handler.
  // ResponseRpcErrorCategory::INVALID_REQUEST
  UNIMPLEMENTED_METHOD = 16,
  // ...
  // ResponseRpcErrorCategory::LOADSHEDDING
  TENANT_QUOTA_EXCEEDED = 17,
}

struct ResponseRpcError {
  1: optional string name_utf8;
  2: optional string what_utf8;
  3: optional ResponseRpcErrorCategory category;
  4: optional ResponseRpcErrorCode code;
  // Server load. Returned to client if loadMetric was set in RequestRpcMetadata
  5: optional i64 load;
}

enum StreamRpcErrorCode {
  UNKNOWN = 0,
  CREDIT_TIMEOUT = 1,
  CHUNK_TIMEOUT = 2,
}

struct StreamRpcError {
  1: optional string name_utf8;
  2: optional string what_utf8;
  4: optional StreamRpcErrorCode code;
}

struct StreamPayloadMetadata {
  // The compression algorithm that was used to compress stream payload. MUST
  // be set iff stream payload is compressed. SHOULD match the algorithm set
  // in compressionConfig in RequestRpcMetadata
  1: optional CompressionAlgorithm compression;
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  2: optional map<string, string (java.swift.binary_string)> (
    cpp.template = "folly::F14NodeMap",
  ) otherMetadata;
  // Metadata describing the type of stream payload. MUST be set for protocol
  // version 8+.
  3: optional PayloadMetadata payloadMetadata;
}

// Setup metadata sent from the client to the server at the time
// of initial connection.
enum InterfaceKind {
  USER = 0,
  DEBUGGING = 1,
  MONITORING = 2,
  PROFILING = 3,
}

// The key is 32-bit, using a 64-bit constant here to make it work with signed
// types.
const i64 kRocketProtocolKey = 0xf09f9a80;

struct ClientMetadata {
  1: optional string agent;
  2: optional string hostname;
  3: optional map<string, string> (
    cpp.template = "folly::F14NodeMap",
  ) otherMetadata;
}

struct RequestSetupMetadata {
  1: optional map<string, binary> (
    cpp.template = "apache::thrift::MetadataOpaqueMap",
  ) opaque;
  // Indicates interface kind that MUST be used by this connection. If not set
  // or if server doesn't support requested interface kind, USER interface kind
  // SHOULD be used. MAY be set.
  2: optional InterfaceKind interfaceKind;
  // Minimum Rocket protocol version supported by the client. If server only
  // supports Rocket protocol version lower than minVersion connection MUST be
  // rejected. SHOULD be set.
  3: optional i32 minVersion;
  // Maximum Rocket protocol version supported by the client. If server only
  // supports Rocket protocol version higher than maxVersion connection MUST be
  // rejected. SHOULD be set.
  4: optional i32 maxVersion;
  // The DSCP and SO_MARK values that the server SHOULD apply to its end of
  // the connection. MAY be set.
  5: optional i32 dscpToReflect;
  6: optional i32 markToReflect;
  10: optional i32 qosUseCaseId;
  11: optional i32 qosPolicyId;
  // Other metadata
  9: optional ClientMetadata clientMetadata;
} // next-id: 12

struct SetupResponse {
  // The Rocket protocol version that server picked. SHOULD be set. MUST be a
  // value between minVersion and maxVersion. If not set client SHOULD assume
  // that any Rocket protocol version between minVersion and maxVersion MAY be
  // used by the server.
  1: optional i32 version;
  // Whether ZSTD compression for requests and responses is supported by the
  // server. SHOULD be set.
  // If not set (or if false) client SHOULD not use ZSTD compression.
  2: optional bool zstdSupported;
}

struct StreamHeadersPush {
  // MUST be set. MUST match stream id of a live stream created over the same
  // connection.
  1: optional i32 streamId;
  2: optional HeadersPayloadContent headersPayloadContent;
}

struct TransportMetadataPush {
  // Those are the transport metadata in key:value string pairs. They can
  // be any interesting data from the underlying transport for which the client
  // wants to send over to the server. This push frame is generated after the
  // client has received the response from the server for the setup frame. Thus
  // dynamic information such as the result of the TLS handshake can also be
  // added to this metadata map.
  1: optional map<string, string> (
    cpp.template = "folly::F14NodeMap",
  ) transportMetadata;
}

enum DrainCompleteCode {
  EXCEEDED_INGRESS_MEM_LIMIT = 1,
}

struct DrainCompletePush {
  1: optional DrainCompleteCode drainCompleteCode;
}

union ServerPushMetadata {
  // First non-error message from the server MUST be SetupResponse.
  1: SetupResponse setupResponse;
  // Supported in Rocket protocol version 7+
  2: StreamHeadersPush streamHeadersPush;
  // If set means that server finished processing all the requests that it
  // ever started processing and sent all the responses for those requests.
  // Server MUST not start processing any new requests after sending this.
  // Supported in Rocket protocol version 7+
  3: DrainCompletePush drainCompletePush;
}

union ClientPushMetadata {
  // Supported in Rocket protocol version 7+
  1: InteractionTerminate interactionTerminate;
  // Supported in Rocket protocol version 7+
  2: StreamHeadersPush streamHeadersPush;
  3: TransportMetadataPush transportMetadataPush;
}

struct HeadersPayloadContent {
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  1: optional map<string, string (java.swift.binary_string)> (
    cpp.template = "folly::F14NodeMap",
  ) otherMetadata;
}

struct HeadersPayloadMetadata {
  // The compression algorithm that was used to compress headers payload. MUST
  // be set iff headers payload is compressed. SHOULD match the algorithm set
  // in compressionConfig in RequestRpcMetadata
  1: optional CompressionAlgorithm compression;
}
