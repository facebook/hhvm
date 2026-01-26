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
namespace go 'github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata'

include "thrift/annotation/thrift.thrift"
include "thrift/annotation/cpp.thrift"

@thrift.AllowLegacyMissingUris
package;

cpp_include "thrift/lib/cpp2/util/ManagedStringView.h"
cpp_include "thrift/lib/thrift/RpcMetadata_extra.h"
cpp_include "folly/container/F14Map.h"

@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

enum ChecksumAlgorithm {
  NONE = 0,
  CRC32 = 1,
  XXH3_64 = 2,
}

// Checksum Metadata for RSocket Data field
struct Checksum {
  // The Algorthim used
  1: ChecksumAlgorithm algorithm;

  // The checksum of Payload Request Field
  2: i64 checksum;

  // prevent collision
  3: i64 salt;
}

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
  BIDIRECTIONAL_STREAM = 7,
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

// TTransform is the precursor to CompressionAlgorithm and was originally a
// C++-only enum. It was manually copied to other languages. Wherever
// convenient, any of those duplicate copies should be made to rely on this
// common thrift definition. This enum should be deprecated with THeader.
@cpp.EnumType{type = cpp.EnumUnderlyingType.U16}
enum TTransform {
  NONE = 0,

  ZLIB = 1,
  // HMAC = 2, Deprecated and no longer supported
  // SNAPPY = 3, Deprecated and no longer supported
  // QLZ = 4, Deprecated and no longer supported

  ZSTD = 5,

  // Values from this point onward are not directly suppored in THeader. These
  // values exist only so that the associated compression settings may be set
  // from ServiceRouter.

  LZ4 = 6,

  CUSTOM = 7,

  ZLIB_LESS = 8,
  ZSTD_LESS = 9,
  LZ4_LESS = 10,

  ZLIB_MORE = 11,
  ZSTD_MORE = 12,
  LZ4_MORE = 13,
}

enum CompressionAlgorithm {
  NONE = 0,

  ZLIB = 1,
  ZSTD = 2,
  LZ4 = 3,

  CUSTOM = 1000,

  // Same algorithms as above, but less compression/CPU/memory.
  ZLIB_LESS = 100000,
  ZSTD_LESS = 100001,
  LZ4_LESS = 100002,

  // Same algorithms as above, but more compression/CPU/memory.
  ZLIB_MORE = 200000,
  ZSTD_MORE = 200001,
  LZ4_MORE = 200002,
}

enum ZlibCompressionLevelPreset {
  DEFAULT = 0,
  LESS = 1, // Less compression/CPU/memory.
  MORE = 2, // More compression/CPU/memory.
}

enum ZstdCompressionLevelPreset {
  DEFAULT = 0,
  LESS = 1, // Less compression/CPU/memory.
  MORE = 2, // More compression/CPU/memory.
}

enum Lz4CompressionLevelPreset {
  DEFAULT = 0,
  LESS = 1, // Less compression/CPU/memory.
  MORE = 2, // More compression/CPU/memory.
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

struct ZlibCompressionCodecConfig {
  1: optional ZlibCompressionLevelPreset levelPreset;
}

struct ZstdCompressionCodecConfig {
  1: optional ZstdCompressionLevelPreset levelPreset;
}

struct Lz4CompressionCodecConfig {
  1: optional Lz4CompressionLevelPreset levelPreset;
}

struct CustomCompressionCodecConfig {
  1: optional binary payload;
}

union CodecConfig {
  1: ZlibCompressionCodecConfig zlibConfig;
  2: ZstdCompressionCodecConfig zstdConfig;
  3: CustomCompressionCodecConfig customConfig;
  4: Lz4CompressionCodecConfig lz4Config;
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
  @cpp.Type{name = "std::uint64_t"}
  1: optional i64 compressionAlgos;
  2: optional bool useStopTLS;
  3: optional bool useStopTLSV2;
  4: optional bool useStopTLSForTTLSTunnel;
}

// String type optimized for generated code
@cpp.Type{name = "::apache::thrift::ManagedStringViewWithConversions"}
typedef string ManagedStringViewField

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

struct FdMetadata {
  1: optional i64 fdSeqNum; // Counter for # of FDs sent via socket, can wrap
  2: optional i32 numFds; // Linux currently limits this to SCM_MAX_FD (253).
}

struct QuotaReportConfig {
  1: i64 backendId;
  2: list<string> reportingHierarchy;
}

struct LoggingContext {
  1: i64 logSampleRatio = 0;
  2: i64 logErrorSampleRatio = 0;
  3: optional string requestId; // unique request id generated by the client
  4: optional string routingTarget;
  5: i32 requestAttemptId;
}

@thrift.ReserveIds{ids = [4, 9, 10, 12, 21]}
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
  @cpp.Type{template = "folly::F14NodeMap"}
  8: optional map<string, string> otherMetadata;
  // The CRC32C of the uncompressed request payload. MAY be set. SHOULD be
  // validated if set.
  @cpp.Type{name = "std::uint32_t"}
  11: optional i32 crc32c;
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
  @thrift.Box
  22: optional FdMetadata fdMetadata;
  // store client logging metadata such as log sampling ratios, request id, etc. MAY be set.
  23: optional LoggingContext loggingContext;
  // Pass tenantId to support multi-tenant platforms.
  24: optional string tenantId;
  // checksum metadata
  25: optional Checksum checksum;
  // require checksumed response
  26: optional bool checksumResponse;
  // FB303 metric name to allow SR to fetch secondary load.
  27: optional string secondaryLoadMetric;
  // Quota report config
  28: optional QuotaReportConfig quotaReportConfig;
  // FB303 metric name to allow SR to restrict convergence
  29: optional string stopperMetric;
  // FB303 metric name to allow SR to fetch global routing load counter.
  30: optional string grLoadMetric;
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

struct PayloadAppUnknownExceptionMetdata {
  1: optional ErrorClassification errorClassification;
}

struct PayloadAnyExceptionMetadata {}

@thrift.ReserveIds{ids = [3, 4, 5]}
union PayloadExceptionMetadata {
  // If set response payload MUST contain serialized declared exception.
  1: PayloadDeclaredExceptionMetadata declaredException;
  // If set response payload SHOULD contain exception serialized in a format
  // defined by a proxy.
  // Deprecated:
  // replaced by PayloadAnyExceptionMetadata
  2: PayloadProxyExceptionMetadata DEPRECATED_proxyException;
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

@thrift.ReserveIds{ids = [1, 2, 11]}
struct ResponseRpcMetadata {
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  @cpp.Type{template = "folly::F14NodeMap"}
  3: optional map<string, string> otherMetadata;
  // Server load. SHOULD be set iff loadMetric was set in RequestRpcMetadata
  4: optional i64 load;
  // The CRC32C of the uncompressed response payload. MAY be set. SHOULD be
  // validated if set.
  @cpp.Type{name = "std::uint32_t"}
  5: optional i32 crc32c;
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
  @thrift.Box
  12: optional FdMetadata fdMetadata;
  13: optional IOBufPtr frameworkMetadata;
  // checksum metadata
  14: optional Checksum checksum;
  // Load returned by server in response to fb303 metric from
  // request header 'secondary_load'
  15: optional i64 secondaryLoad;
  // stopper Metric value returned by server in response
  16: optional i64 stopperMetric;
  // Global routing load counter value returned by server in response
  17: optional i64 grLoad;
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
  // 18 : DEPRECATED
  // Interaction request rejected because interaction is already load-shedded
  // ResponseRpcErrorCategory::LOADSHEDDING
  INTERACTION_LOADSHEDDED = 19,
  // Interaction request rejected due to OVERLOAD and interaction marked load-shedded
  // ResponseRpcErrorCategory::LOADSHEDDING
  INTERACTION_LOADSHEDDED_OVERLOAD = 20,
  // Interaction request rejected due to APP_OVERLOAD and interaction marked load-shedded
  // ResponseRpcErrorCategory::LOADSHEDDING
  INTERACTION_LOADSHEDDED_APP_OVERLOAD = 21,
  // Interaction request rejected due to QUEUE_TIMEOUT and interaction marked load-shedded
  // ResponseRpcErrorCategory::LOADSHEDDING
  INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT = 22,
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
  SERVER_CLOSING_CONNECTION = 3,
}

struct StreamRpcError {
  1: optional string name_utf8;
  2: optional string what_utf8;
  4: optional StreamRpcErrorCode code;
}

@thrift.ReserveIds{ids = [4]}
struct StreamPayloadMetadata {
  // The compression algorithm that was used to compress stream payload. MUST
  // be set iff stream payload is compressed. SHOULD match the algorithm set
  // in compressionConfig in RequestRpcMetadata
  1: optional CompressionAlgorithm compression;
  // Arbitrary metadata that MAY be used by application or Thrift extensions.
  // MAY be set.
  @cpp.Type{template = "folly::F14NodeMap"}
  2: optional map<string, string_4852> otherMetadata;
  // Metadata describing the type of stream payload. MUST be set for protocol
  // version 8+.
  3: optional PayloadMetadata payloadMetadata;
  @thrift.Box
  5: optional FdMetadata fdMetadata;
  // checksum metadata
  6: optional Checksum checksum;
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
const i64 kRocketProtocolKeyWithSBE = 0xf09f9a81;

struct ClientMetadata {
  1: optional string agent;
  2: optional string hostname;
  @cpp.Type{template = "folly::F14NodeMap"}
  3: optional map<string, string> otherMetadata;
}

struct CustomCompressionSetupRequest {
  1: string compressorName;
  2: optional binary payload;
}

union CompressionSetupRequest {
  1: CustomCompressionSetupRequest custom;
}

struct CustomCompressionSetupResponse {
  1: string compressorName;
  2: optional binary payload;
}

union CompressionSetupResponse {
  1: CustomCompressionSetupResponse custom;
}

struct RequestSetupMetadata {
  @cpp.Type{template = "apache::thrift::MetadataOpaqueMap"}
  1: optional map<string, binary> opaque;
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
  // RSocket KeepAliveFrame Timeout
  12: optional i32 keepAliveTimeoutMs;
  // Encode Thrift Metadata using Binary
  13: optional bool encodeMetadataUsingBinary;
  // If provided, we attempt to perform initial negotiation for compression algorithm.
  14: optional CompressionSetupRequest compressionSetupRequest;
} // next-id: 15

// Describes the security policy exchanged from server to client during connection setup (via SetupResponse).
struct SecurityPolicy {
  // Defines the level of authorization (permission checking) enforcement on the server.
  1: optional SecurityPolicyStatus authorization;

  // Defines the AuthWall enforcement status on the server.
  2: optional SecurityPolicyStatus authWall;
}

// Defines the enforcement level of a security policy.
enum SecurityPolicyStatus {
  // Security policy is not installed or is explicitly turned off.
  DISABLED = 0,

  // Security policy is installed and operational. However, it may be in logging mode,
  // which allows non-compliant connections or requests to proceed while violations are logged but not blocked.
  ENABLED = 1,

  // Security policy is fully enforced. Connections or requests that do not comply with the policy will be blocked or terminated.
  ENFORCED = 2,
}

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
  // If compression setup is successful, the setup response will be returned here.
  3: optional CompressionSetupResponse compressionSetupResponse;
  // Describes the server's security policies and their enforcement levels.
  4: optional SecurityPolicy securityPolicy;
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
  @cpp.Type{template = "folly::F14NodeMap"}
  1: optional map<string, string> transportMetadata;
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
  @cpp.Type{template = "folly::F14NodeMap"}
  1: optional map<string, string_4852> otherMetadata;
}

struct HeadersPayloadMetadata {
  // The compression algorithm that was used to compress headers payload. MUST
  // be set iff headers payload is compressed. SHOULD match the algorithm set
  // in compressionConfig in RequestRpcMetadata
  1: optional CompressionAlgorithm compression;
}

// The following were automatically generated and may benefit from renaming.
@thrift.DeprecatedUnvalidatedAnnotations{
  items = {"java.swift.binary_string": "1"},
}
typedef string string_4852
