---
state: draft
---

# Rocket Protocol

This document describes the Rocket transport protocol and how it is used by Thrift to achieve the [Interface Types](../../definition/index.md) by using [RSocket](https://rsocket.io/about/protocol).

## Thrift Protocol Negotiation

### Rocket/RSocket over TLS
If ALPN is supported by both client and server - "rs" MUST be used as a RSocket protocol identifier.

### Other transport (e.g. THeader) to Rocket/RSocket upgrade
Once a non-Rocket Thrift connection is established - an upgrade to Rocket/RSocket SHOULD be attempted by the client.

To perform an upgrade a client MUST use the [RocketUpgrade](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RocketUpgrade.thrift) interface immediately after opening the connection.

If the upgrade was successful - both server and client MUST treat the connection as a newly established RSocket connection.

## Versioning

Rocket follows a versioning scheme consisting of a single numeric version. This document describes protocol versions 8 through 10.

## Connection setup

### Rocket protocol version 9+
RSocket setup frame MUST use application/x-rocket-metadata+compact as Metadata Encoding MIME Type and application/x-rocket-payload as Data Encoding MIME Type.

RSocket setup payload MUST consist of a compact-serialized RequestSetupMetadata struct. Such compact-serialized RequestSetupMetadata MAY be prefixed by a 32-bit kRocketProtocolKey if Rocket client wants to remain compatible with Rocket server using Rocket protocol version 6-8.

If connection establishment was successful, the server MUST respond with a SetupResponse control message.

### Rocket protocol version 8
RSocket setup payload MUST consist of a 32-bit kRocketProtocolKey followed by a compact-serialized RequestSetupMetadata struct.

If connection establishment was successful, the server MUST respond with a SetupResponse control message.

## Request-Response

With an already established connection, the client must send a [REQUEST_RESPONSE](https://rsocket.io/about/protocol/#request_response-frame-0x04) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> A new stream ID should be generated for the request. It must comply with the requirements of [RSocket Stream Identifiers](https://rsocket.io/about/protocol/#stream-identifiers)
Frame type | 6 bits <br/> Must be REQUEST_RESPONSE (0x04)
Flags | 10 bits <br/> Metadata flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). This includes information such as: <br/> - Method name <br/> - Thrift serialization protocol <br/> - RPC kind (SINGLE_REQUEST_SINGLE_RESPONSE)
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

The Thrift server should then perform the method specified in the RequestRpcMetadata method name field. Once the result is ready, the server should send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame containing the result in the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> This Stream ID should be the same value as the request Stream ID
Frame type | 6 bits <br/> Must be PAYLOAD (0x0A)
Flags | 10 bits <br/> Metadata flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly) <br/> Complete flag must be set <br/> Next flag must be set
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [ResponseRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)
Data | Thrift serialized result from [Interface Protocol](index.md#response)

### Declared Exception

If the response is a [declared exception](../../definition/exception.md#exceptions), the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadDeclaredExceptionMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadDeclaredExceptionMetadata) struct.

### Undeclared Exception

If the response is an [undeclared exception](../../definition/exception.md#exceptions), the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadAppUnknownExceptionMetdata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadAppUnknownExceptionMetdata) struct.

### Any Exception

If the response is an Any exception, the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadAnyExceptionMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadAnyExceptionMetadata) struct.

### Internal Server Error

Internal server errors **must** be sent as an [ERROR](https://rsocket.io/about/protocol/#error-frame-0x0b) frame using the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> This Stream ID should be the same value as the request Stream ID
Frame type | 6 bits <br/> Must be ERROR (0x0B)
Flags | 10 bits <br/> No flags should be set
Error code | Should be one of [REJECTED](https://rsocket.io/about/protocol/#error-codes), [CANCELED](https://rsocket.io/about/protocol/#error-codes), or [INVALID](https://rsocket.io/about/protocol/#error-codes)
Error data | Compact Protocol serialized [ResponseRpcError struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcError)

The `ResponseErrorCode` in the `ResponseRpcError` struct is a mapping from the error code in the [Interface protocol](index.md#internal-server-error).

## Oneway Request (request no response)

With an already established connection, the client must send a [REQUEST_FNF](https://rsocket.io/about/protocol/#request_fnf-fire-n-forget-frame-0x05) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> A new stream ID should be generated for the request. It must comply with the requirements of [RSocket Stream Identifiers](https://rsocket.io/about/protocol/#stream-identifiers)
Frame type | 6 bits <br/> Must be REQUEST_FNF (0x05)
Flags | 10 bits <br/> Metadata flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). This includes information such as: <br/> - Method name <br/> - Thrift serialization protocol <br/> - RPC kind (SINGLE_REQUEST_NO_RESPONSE)
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

## Stream

With an already established connection, the client must send a [REQUEST_STREAM](https://rsocket.io/about/protocol/#request_stream-frame-0x06) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> A new stream ID should be generated for the request. It must comply with the requirements of [RSocket Stream Identifiers](https://rsocket.io/about/protocol/#stream-identifiers)
Frame type | 6 bits <br/> Must be REQUEST_STREAM (0x06)
Flags | 10 bits <br/> Metadata flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Initial request N | 32 bits <br/> Unsigned integer representing the initial number of stream payloads to request. Value MUST be > 0. Max value is 2^31 - 1.
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). This includes information such as: <br/> - Method name <br/> - Thrift serialization protocol <br/> - RPC kind (SINGLE_REQUEST_STREAMING_RESPONSE)
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

### Stream Initial Response

The initial response for a stream is distinct from [streaming responses](#stream-responses) and it must be sent from the server even if there is no initial response type specified in the IDL. The initial response should be sent using the same format as a regular [Request-Response response](#request-response) with the distinction that the Complete flag must only be set if the payload contains an exception.

### Stream Responses

Once a stream has been established, the server can send stream payloads to the client as long as it has credits remaining. Sending a stream payload must consume one credit on the server. To send a stream payload to the client, the server must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing stream on which you would like to send the payload
Frame type | 6 bits <br/> Must be PAYLOAD (0x0A)
Flags | 10 bits <br/> Metadata flag must be set <br/> Next flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)
Data | Thrift serialized result from [Interface Protocol](index.md#response) using the serialization protocol specified in [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)

### Stream Exception

Once a stream has been established, the server **may** terminate the stream by sending an exception to the client. The server **must not** send any more payloads to the client after it sends an exception. Stream exceptions **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) instead of a [ResponseRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). Additionally, if the exception is an internal server error, [StreamRpcError struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) **must** be used instead of [ResponseRpcError struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift).

### Stream Completion

The server must complete a stream once it is done sending all stream payloads. Completing a stream does not require credits on the server. To send a stream completion to the client, the server must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing stream you would like to complete
Frame type | 6 bits <br/> Must be PAYLOAD (0x0A)
Flags | 10 bits <br/> Complete flag must be set
Metadata & Data | Empty

### Stream Credit Mechanism

Thrift streaming is flow-controlled using the [RSocket credit mechanism](https://rsocket.io/about/protocol/#reactive-streams-semantics). When a client requests a stream, it sends the server an initial number of credits in the request. The client can send more credits to the server by sending a [REQUEST_N](https://rsocket.io/about/protocol/#request_n-frame-0x08) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing stream that you would like to send credits for
Frame type | 6 bits <br/> Must be REQUEST_N (0x08)
Flags | 10 bits <br/> No flags should be set
Request N | 32 bits <br/> Unsigned integer representing the additional number of stream payloads to request. Value MUST be > 0. Max value is 2^31 - 1.

Credits are cumulative on the server. The server must not send stream payloads if it has 0 credits until it receives more credits from the client.

### Cancellation

Thrift streaming supports cancellation from the client. Upon receiving the cancellation, the server should stop sending payloads to the client. The client can cancel a stream by sending a [CANCEL](https://rsocket.io/about/protocol/#cancel-frame-0x09) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing stream that you would like to cancel
Frame type | 6 bits <br/> Must be CANCEL (0x09)
Flags | 10 bits <br/> No flags should be set

## Sink

With an already established connection, the client must send a [REQUEST_CHANNEL](https://rsocket.io/about/protocol/#request_channel-frame-0x07) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> A new stream ID should be generated for the request. It must comply with the requirements of [RSocket Stream Identifiers](https://rsocket.io/about/protocol/#stream-identifiers)
Frame type | 6 bits <br/> Must be REQUEST_CHANNEL (0x07)
Flags | 10 bits <br/> Metadata flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Initial request N | 32 bits <br/> Unsigned integer representing the number of payloads the server can send to the client. Must be at least 2 (1 initial response, 1 final response). Max value is 2^31 - 1.
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). This includes information such as: <br/> - Method name <br/> - Thrift serialization protocol <br/> - RPC kind (SINK)
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

### Sink Initial Response

The initial response must be sent from the server even if there is no initial response type specified in the IDL. The initial response should be sent using the same format as a regular [Request-Response response](#request-response) with the distinction that the Complete flag must only be set if the payload contains an exception.

### Sink Payloads

Once a sink has been established, the client can send sink payloads to the server as long as it has [credits remaining](#sink-credit-mechanism). To send a sink payload to the server, the client must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing sink on which you would like to send the payload
Frame type | 6 bits <br/> Must be PAYLOAD (0x0A)
Flags | 10 bits <br/> Metadata flag must be set <br/> Next flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)
Data | Thrift serialized result from [Interface Protocol](index.md#response) using the serialization protocol specified in [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)

### Sink Exception

A client can terminate the sink early by sending an exception to the server. The client **must not** send any more payloads to the server after it sends an exception. The exception **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) instead of a [ResponseRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift).

### Final Response

The server can send a final response to the client any time after the sink has been established. The final response acts as the termination of the sink and the client should not send any more sink payloads to the server. To send a final response to the client, the server must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing sink on which you would like to send the final response
Frame type | 6 bits <br/> Must be PAYLOAD (0x0A)
Flags | 10 bits <br/> Metadata flag must be set <br/> Next flag must be set <br/> Complete flag must be set <br/> Follows flag must be set if the frame requires [fragmentation](https://rsocket.io/about/protocol/#fragmentation-and-reassembly)
Metadata size | 24 bit unsigned integer indicating the length of metadata
Metadata | Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)
Data | Thrift serialized result from [Interface Protocol](index.md#response) using the serialization protocol specified in [RequestRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)

The final response may be an exception in which case, it **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [StreamPayloadMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) instead of a [ResponseRpcMetadata struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift). Additionally, if the exception is an internal server error, [StreamRpcError struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) **must** be used instead of [ResponseRpcError struct](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift).

### Sink Credit Mechanism

Sinks are flow-controlled using the [RSocket credit mechanism](https://rsocket.io/about/protocol/#reactive-streams-semantics). The client will initially start with zero credits and must wait for the server to send credits before it can start sending payloads to the server. The server can send credits to the client by sending a [REQUEST_N](https://rsocket.io/about/protocol/#request_n-frame-0x08) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> The stream ID of the existing sink that you would like to send credits for
Frame type | 6 bits <br/> Must be REQUEST_N (0x08)
Flags | 10 bits <br/> No flags should be set
Request N | 32 bits <br/> Unsigned integer representing the additional number of sink payloads to request. Value MUST be > 0. Max value is 2^31 - 1.

Credits are cumulative on the client. The client must not send sink payloads if it has 0 credits until it receives more credits from the server. Sending one sink payload must consume one credit on the client.

## Interactions

### Factory Functions

Factory function requests should be sent the same way as their non-interaction counterpart with the `RequestRpcMetadata.createInteraction` field filled out (Note: `RequestRpcMetadata.interactionId` must not be set).

The `InteractionCreate` struct must contain a unique interaction ID as well as the name of the interaction (as defined in the IDL).

### Subsequent Requests

All subsequent requests should be sent the same way as their non-interaction counterpart with two important distinctions:

1. `RequestRpcMetadata.interactionId` must be set to the Interaction ID that was created by the Interaction factory function (Note: `RequestRpcMetadata.createInteraction` must not be set)
2. The request must be sent on the same connection as the original factory function request

### Termination

An interaction can only be terminated by the client. If the client has already sent the factory function request to the server before terminating the interaction, it should send a termination signal to the server. Once an interaction is terminated, the client must not send any more requests with that interaction ID.

To send a termination signal, the client must send a [METADATA_PUSH](https://rsocket.io/about/protocol/#metadata_push-frame-0x0c) frame of the following format:

Field | Notes
:---: | :---:
Frame size | 24 bit unsigned integer indicating the length of the *entire* frame
Stream ID | 32 bits <br/> Must be 0
Frame type | 6 bits <br/> Must be METADATA_PUSH (0x0C)
Flags | 10 bits <br/> Metadata flag must be set
Metadata | Compact Protocol serialized [ClientPushMetadata](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) union. This includes information such as: <br/> - `interactionTerminate` field must be set <br/> - The `InteractionTerminate` struct must contain the interaction ID of the interaction being terminated

## Control messages

All Control messages are sent via METADATA_PUSH RSocket frame.

### Control messages from the server
METADATA_PUSH frame sent by the server MUST contain a compact-serialized ServerPushMetadata struct.

### Control messages from the client
METADATA_PUSH frame sent by the client MUST contain a compact-serialized ClientPushMetadata struct.
