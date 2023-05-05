---
state: draft
---

# Rocket Protocol

This document describes the Rocket transport protocol and how it is used by Thrift to achieve the [Interface Types](/idl/interfaces.md) by using [RSocket](https://rsocket.io/about/protocol).

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

RSocket setup payload MUST consist of a compact-serialized [`RequestSetupMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct. Such compact-serialized [`RequestSetupMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) MAY be prefixed by a 32-bit [`kRocketProtocolKey`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) if Rocket client wants to remain compatible with Rocket server using Rocket protocol version 6-8.

If connection establishment was successful, the server MUST respond with a SetupResponse control message.

### Rocket protocol version 8
RSocket setup payload MUST consist of a 32-bit [`kRocketProtocolKey`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) followed by a compact-serialized [`RequestSetupMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct.

If connection establishment was successful, the server MUST respond with a SetupResponse control message.

## Request-Response

With an already established connection, the client must send a [REQUEST_RESPONSE](https://rsocket.io/about/protocol/#request_response-frame-0x04) frame of the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

The Thrift server should then perform the method specified in the RequestRpcMetadata method name field. Once the result is ready, the server should send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame containing the result in the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol](index.md#response) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

### Declared Exception

If the response is a [declared exception](/features/exception.md#exceptions), the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadDeclaredExceptionMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadDeclaredExceptionMetadata) struct.

### Undeclared Exception

If the response is an [undeclared exception](/features/exception.md#exceptions), the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadAppUnknownExceptionMetdata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadAppUnknownExceptionMetdata) struct.

### Any Exception

If the response is an Any exception, the [`PayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadMetadata) in the [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcMetadata) struct **must** contain a [`PayloadExceptionMetadataBase`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadExceptionMetadataBase) which contains a [`PayloadAnyExceptionMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#PayloadAnyExceptionMetadata) struct.

### Internal Server Error

Internal server errors **must** be sent as an [ERROR](https://rsocket.io/about/protocol/#error-frame-0x0b) frame using the following format:

Field | Notes
:---: | :---:
Error code | Should be one of [REJECTED](https://rsocket.io/about/protocol/#error-codes), [CANCELED](https://rsocket.io/about/protocol/#error-codes), or [INVALID](https://rsocket.io/about/protocol/#error-codes)
Error data | Compact Protocol serialized [`ResponseRpcError`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ResponseRpcError) struct

The `ResponseErrorCode` in the `ResponseRpcError` struct is a mapping from the error code in the [Interface protocol](index.md#internal-server-error).

## Oneway Request (request no response)

With an already established connection, the client must send a [REQUEST_FNF](https://rsocket.io/about/protocol/#request_fnf-fire-n-forget-frame-0x05) frame of the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

## Stream

With an already established connection, the client must send a [REQUEST_STREAM](https://rsocket.io/about/protocol/#request_stream-frame-0x06) frame of the following format:

Field | Notes
:---: | :---:
Initial request N | Value MUST be > 0. First credit is always consumed by initial response
Metadata | Compact Protocol serialized [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

### Stream Initial Response

The initial response for a stream is distinct from [streaming responses](#stream-responses) and it must be sent from the server even if there is no initial response type specified in the IDL. The initial response is a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame with the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol/Stream Initial Response](index.md#stream-initial-response) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

### Stream Responses

Once a stream has been established, the server can send stream payloads to the client as long as it has credits remaining. Sending a stream payload must consume one credit on the server. To send a stream payload to the client, the server must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol/Stream Payloads](index.md#stream-payloads) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

### Stream Exception

Once a stream has been established, the server **may** terminate the stream by sending an exception to the client. The server **must not** send any more payloads to the client after it sends an exception. Stream exceptions **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct instead of a [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct. Additionally, if the exception is an internal server error, [`StreamRpcError`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct **must** be used instead of [`ResponseRpcError`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct.

### Flow Control

Thrift stream is flow-controlled using the [RSocket credit mechanism](https://rsocket.io/about/protocol/#reactive-streams-semantics).

Note that request SHOULD always come with at least one credit, which will be consumed by initial response.

### Cancellation

Thrift stream supports cancellation from the client using [RSocket cancellation mechanism](https://rsocket.io/about/protocol/#cancel-frame-0x09).

## Sink

With an already established connection, the client must send a [REQUEST_CHANNEL](https://rsocket.io/about/protocol/#request_channel-frame-0x07) frame of the following format:

Field | Notes
:---: | :---:
Initial request N | Must be at least 2 (1 initial response, 1 final response)
Metadata | Compact Protocol serialized [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized arguments from [Interface Protocol](index.md#request)

### Sink Initial Response

The initial response must be sent from the server even if there is no initial response type specified in the IDL. The initial response is a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame with the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol/Sink Initial Response](index.md#sink-initial-response) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

### Sink Payloads

Once a sink has been established, the client can send sink payloads to the server as long as it has credits remaining. To send a sink payload to the server, the client must send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol/Sink Payloads](index.md#sink-payloads) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

### Sink Exception

A client can terminate the sink early by sending an exception to the server. The client **must not** send any more payloads to the server after it sends an exception. The exception **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct instead of a [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct.

### Final Response

The server MAY send a final response to the client any time after the sink has been established. The final response acts as the termination of the sink and the client should not send any more sink payloads to the server. To send a final response to the client, the server MUST send a [PAYLOAD](https://rsocket.io/about/protocol/#payload-frame-0x0a) frame of the following format:

Field | Notes
:---: | :---:
Metadata | Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct
Data | Thrift serialized result from [Interface Protocol](index.md#response) using the serialization protocol specified in [`RequestRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct

The final response may be an exception in which case, it **must** be sent using the format specified in [Request-Response](#request-response) with the distinction that the Metadata **must** contain a Compact Protocol serialized [`StreamPayloadMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct instead of a [`ResponseRpcMetadata`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct. Additionally, if the exception is an internal server error, [`StreamRpcError`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct **must** be used instead of [`ResponseRpcError`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) struct.

### Flow Control

Sink payloads are flow-controlled using the [RSocket credit mechanism](https://rsocket.io/about/protocol/#reactive-streams-semantics).

Sink responses are NOT flow-controlled. The request SHOULD always come with at least two credits, which will be consumed by initial response and final response.

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
Metadata | Compact Protocol serialized [ClientPushMetadata](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift) union

## Control messages

All Control messages are sent via METADATA_PUSH RSocket frame.

### Control messages from the server
METADATA_PUSH frame sent by the server MUST contain a compact-serialized ServerPushMetadata struct.

### Control messages from the client
METADATA_PUSH frame sent by the client MUST contain a compact-serialized ClientPushMetadata struct.
