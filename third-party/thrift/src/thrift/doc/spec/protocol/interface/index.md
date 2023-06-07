---
state: draft
sidebar_position: 2
---

# Interface Protocols

This document describes the layer immediately preceding the transport protocol (eg. [Rocket](rocket.md)). This layer specifies how the request/response data must be serialized and formatted before being wrapped in the underlying transport protocol layer's message format.

## Request

The client **must** take the user provided arguments to the Interface method and serialize the request as described in Serialization Details below. The client **may** compress the serialized request as described in [Request Compression](#request-compression). The name of the Interface method being requested as well as the [serialization protocol](/features/serialization/protocols.md) that was used to serialize the request **must** be included in the metadata associated with the request. The client may detect an exception while attempting to perform the request before a valid response is received from the server, in which case, it **must** be raised as a [Client Detected Exception](#client-detected-exceptions).

The client **must** specify the method name in the request as follows:
- For methods inside an interaction, the method name is `‹InteractionName›.‹MethodName›` where `‹InteractionName›` and `‹MethodName›` are the names in the IDL of the interaction and method respectively.
- For all other methods (including factory methods for interactions), the method name matches the name in the IDL.

### Serialization Details

The parameters to an Interface method **must** be treated as fields of a Thrift struct with an empty name (`""`). The Field IDs **must** be the same as those specified in the IDL. If the Interface method has no parameters then the struct **must** have no fields. To prepare for sending the request through one of the underlying transport protocols, this unnamed struct **must** be serialized with one of Thrift [serialization protocols](/features/serialization/protocols.md).

For example, this method:

```
i32 foo(1: i32 a, 2: string b)
```

might be called with these fields set in the metadata:

```
  protocol = ProtocolId.COMPACT;
  name = "foo";
  kind = RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE;
  compression = CompressionAlgorithm.ZSTD;
```

in which case it will have as its serialized request the result of placing the arguments in a

```
struct ‹Anonymous› {
  1: i32 a;
  2: string b;
}
```

passing the `‹Anonymous›` to a serializer for the [Compact Protocol](/features/serialization/protocols.md#compact-protocol) and passing the resulting string to a compressor for [zstd](https://facebook.github.io/zstd/).

## Response

A response may be one of the following types:

- Declared response - responses that actually represent the response type of an Interface declared in the IDL. For example, the return type of a request-response method or the payload type of a stream.
- Declared exception
- Undeclared exception
- Any exception
- Internal server error

The server **must** serialize the response depending on the response type as described below. If the response is a declared response, declared exception, or undeclared exception, the server **should** compress the serialized response as described in [Response Compression](#response-compression).

### Declared Response and Declared Exception

The Thrift struct used to represent declared responses and [declared exceptions](/features/exception.md#exceptions) **must** be a union with an empty name (`""`). The first field in the union with Field ID of `0` **must** be for the declared response. If the declared response type is `void` then the field for the declared response **must** be skipped. The Thrift struct **must** also have a field for each declared exception for that Interface with Field IDs matching those specified in the IDL. The correct field **must** be filled in by the server and then serialized using one of Thrift’s data protocols.

For example, this method:

```
i32 foo(1: i32 a, 2: string b) throws (1: MyException e1, 2: OtherException e2);
```

would use the following union as its response struct:

```
union <Anonymous> {
  0: i32 response;
  1: MyException e1;
  2: OtherException e2;
}
```

If the response is a declared response, the response metadata **must** indicate that it is a declared response.

If the response is a declared exception, the response metadata **must** indicate that it is a declared exception and it **should** contain a [`ErrorClassification`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift#ErrorClassification) struct.

### Undeclared Exception

[Undeclared exceptions](/features/exception.md#exceptions) **must** be sent through the response metadata and the metadata **must** indicate that the response is an undeclared exception and **must** contain the exception name and the message.

### Any Exception

The response may also be an Any exception. This is an exception that is serialized using one of Thrift's data protocols and stored in a [`SemiAnyStruct`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/any_rep.thrift#SemiAnyStruct). This `SemiAnyStruct` then must be serialized using one of Thrift's data protocols. If the protocol used to serialize the exception and the protocol used to serialize the `SemiAnyStruct` do not match, the `protocol` field in the `SemiAnyStruct` **must** be the data protocol that was used to serialize the exception. The response metadata **must** indicate that the response is an Any exception.

### Internal Server Error

In addition to responses from the server handler, the server can also decide to reject a request for a variety of reasons, for example, load-shedding, invalid request, etc. In this case the server **should** send an exception containing one of the pre-defined error codes [here](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/ResponseChannel.cpp).

## Interfaces

### Request-Response

To initiate a request-response, the client **must** make a request for a request-response method defined in the IDL. The request **must** follow the format defined in [Request](#request). After sending the request, the client **should** wait for a response from the server or throw a [Client Detected Exception](#client-detected-exceptions).

The server **must** execute the server handler for the requested method or reject the request with an [internal server error](#internal-server-error). Once the server handler finishes executing or throws an exception, the response **must** be sent using the format defined in [Response](#response).

### Stream

#### Stream Request

To initiate a stream, the client **must** make a request for a stream method defined in the IDL. The request **must** follow the format defined in [Request](#request). After sending the request, the client **should** wait for a response from the server or throw a [Client Detected Exception](#client-detected-exceptions).

#### Stream Initial Response

The server **must** execute the server handler for the stream method or reject the request with an [internal server error](#internal-server-error). If the server handler finishes executing successfully, the server **must** send an initial response to the client containing the initial response type specified in the IDL (assumed to be `void` if no initial response type is specified). This initial response **must** be formatted as specified in [Response](#response) and will establish the stream.

If the request was rejected with an [internal server error](#internal-server-error) or the server handler threw an exception (declared or undeclared) the stream **must not** be established and the error **must** be sent to the client using the format specified in [Response](#response). In this case, the server **must not** send any stream payloads to the client.

#### Stream Payloads

Once the initial response is sent and the server has credits, the server **should** start sending stream payloads to the client. The stream payloads **must** be an actual payload, a declared exception, or an undeclared exception and they follow the format of these response types as defined in [Response](#response).

If the server sends a declared exception or an undeclared exception to the client, it acts as the termination of the stream and the server **must not** send any more payloads to the client.

#### Stream Cancellation

The client **may** send a cancellation signal to the server at any point after the stream is established. If the server receives a cancellation, it **should** stop sending stream payloads to the client.

#### Stream Completion

If the stream was established, the server **must** send a completion signal to the client once it is done sending all stream payloads to the client. The completion signal acts as a termination of the stream and the server **must not** send any payloads to the client after it has sent the completion signal.

#### Stream Flow Control

Thrift streaming **must** be flow-controlled using a credit mechanism. The client **must** give the server an initial set of credits that the server can use to send stream payloads. Each stream payload **must** consume one credit on the server. The server **must not** send any payloads if it has no credits until it receives more from the client.

#### Stream Expire Time

The server **may** specify a stream expire time which is the time the server **should** wait after exhausting all of its credits to receive more credits from the client. If the server doesn’t receive any credits after the stream expire time has elapsed, the server **should** terminate the stream with an [internal server error](#internal-server-error).

#### Stream Chunk Timeout

The client **may** specify a chunk timeout which is the time the client **should** wait for receiving a new stream payload from the server. If the client doesn’t receive a payload from the server for longer than the chunk timeout, it **should** cancel the stream and raise a [`TTransportException`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/TTransportException.h) with the type `TIMEOUT`.

### Sink

#### Sink Request

To initiate a sink, the client **must** make a request for a sink method defined in the IDL. The request **must** follow the format defined in [Request](#request). After sending the request, the client **should** wait for a response from the server or throw a [Client Detected Exception](#client-detected-exceptions).

#### Sink Initial Response

The server **must** execute the server handler for the sink method or reject the request with an [internal server error](#internal-server-error). If the server handler finishes executing successfully, the server **must** send an initial response to the client containing the initial response type specified in the IDL (assumed to be `void` if no initial response type is specified). This initial response **must** be formatted as specified in [Response](#response) and will establish the sink.

If the request was rejected with an [internal server error](#internal-server-error) or the server handler threw an exception (declared or undeclared) the sink **must not** be established and the error **must** be sent to the client using the format specified in #Response. In this case, the client **must not** send any sink payloads to the server and the server **must not** send a final response to the client.

#### Sink Payloads

Once the initial response is received and the client has received an initial set of credits from the server, the client **should** start sending sink payloads to the server. The sink payloads **must** be an actual payload, a declared exception, or an undeclared exception and they follow the serialization format of these response types as defined in [Response](#response).

If the client sends a declared exception or an undeclared exception to the server, it **must not** send any more payloads to the server. The server **may** still respond with a final response.

#### Sink Completion

If the sink was established, the client **must** send a completion signal to the server once it is done sending all sink payloads to the server. The client **must not** send any payloads to the server after it has sent the completion signal.

#### Sink Final Response

If the sink was established, the server **must** send a final response to the client. The final response acts as a termination of the sink. The final response **may** be any one of the response types mentioned in [Response](#response). The server **may** send this final response before it receives the completion signal from the client.

#### Sink Flow Control

Thrift sinks **must** be flow-controlled using a credit mechanism. The server **must** send the client an initial set of credits after it receives a sink request. Each sink payload **must** consume one credit on the client. The client **must not** send any payloads if it has no credits until it receives more from the server.

#### Sink Chunk Timeout

The server **may** specify a chunk timeout which is the time the server **should** wait for receiving a new sink payload from the client. If the server doesn’t receive a payload from the client for longer than the chunk timeout, it **should** terminate the sink by sending an [internal server error](#internal-server-error).

### Interactions

Requests that are part of the interaction **must** be sent over the same connection to the server. Each interaction **must** have an ID that is unique to the connection. The server **must** maintain the state that is associated with each interaction so that subsequent requests can use that state.

All requests in an interaction, including the factory function if used, **must** reach the server in the order they are sent by the client. The server **may** begin processing subsequent requests before the earlier ones complete, provided they are started in order.

The RPC contracts of member requests are otherwise handled the same as if the requests were not in an interaction. A single interaction **may** include multiple streams/sinks, and these are flow-controlled independently.

#### Creation

Interactions can be created in two different ways as described below.

##### Factory Functions

Factory functions are an eager way to create an interaction, in that the interaction is created when the factory function is called. The factory function request **must** have metadata indicating that it is creating an interaction. The server **must** execute the server handler for the factory function or reject the request with an [internal server error](#internal-server-error). If the server handler finishes executing successfully, the server **must** send a response to the client containing the response type specified in the IDL (assumed to be `void` if no response type is specified). Subsequent requests in the interaction **must** contain the interaction ID in their metadata.

##### Constructors (Deprecated)

Interaction constructors are a lazy way to create an interaction, in that the interaction is not created until the first request that is part of the interaction is sent. This first request **must** have metadata indicating that it is the first request. Subsequent requests **must** contain the interaction ID in their metadata.

#### Termination

Only the client can terminate the interaction and it can do so at any time after the interaction has been created, including with requests outstanding. When an interaction is terminated, the client **must** send a termination signal to the server. The server **must** finish processing any outstanding requests when the termination signal is received, including waiting for streams/sinks to complete normally; terminating an interaction **must not** cancel member streams/sinks. The server **must** invoke the following two hooks:
- onTermination: scheduled immediately when termination signal is received, without waiting for outstanding requests to complete.
- destructor: scheduled once all responses have been sent back to the client and streams/sinks completed.

#### Serial interactions

For interactions annotated `(serial)` the server **must** ensure no more than one request is being executed at a time by buffering concurrent requests and executing them sequentially. The client **may** send multiple requests in such an interaction without waiting for responses in between.

### Oneway Requests (Deprecated)

To perform a oneway request, the client **must** make a request for a oneway request method defined in the IDL. The request **must** follow the format defined in [Request](#request). The client **must not** wait for a response from the server. The client **should** raise a [Client Detected Exception](#client-detected-exceptions) if there is an exception while the client is attempting to send the request. The server **must** execute the server handler for the requested method or drop the request.

## Timeouts

### Client Timeout

The client **may** specify a timeout value for a request which is the amount of time it is willing to wait for a response from the server before giving up and raising a [`TTransportException`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/TTransportException.h) with the type `TIMEOUT`. This timeout value **should** also be sent to the server in the request metadata. If set, the server **should** only start processing the request from the request queue if the client timeout has not already expired. If it has already expired, the server **should** instead drop the request and send an [internal server error](#internal-server-error) with the code [`kTaskExpiredErrorCode`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/ResponseChannel.cpp).

### Queue Timeout

The client **may** specify a queue timeout value in the request metadata. The queue timeout is the amount of time the server **should** allow the request to sit in its request queue before giving up and dropping the request. If the request sits in the request queue for longer than the queue timeout value, the server **should not** start processing the request and instead send an [internal server error](#internal-server-error) with the code [`kServerQueueTimeoutErrorCode`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/ResponseChannel.cpp).

## Compression

Request and response compression is supported in Thrift using one of the following algorithms:

- ZLIB
- ZSTD

### Request Compression

Requests **may** be compressed by the client after they have been serialized as described in #Request-serialization. If the request was compressed, the compression algorithm **must** be specified in the request metadata.

### Response Compression

The client **may** request the server to compress responses based on the size of the response. If requesting compression, the client **should** specify the compression algorithm and the minimum response size that **should** trigger compression in the request metadata. If requested by the client, the server **should** compress the response after it has been serialized using the compression algorithm specified by the client. If not requested by the client, the server **may** compress the response and it **may** use any supported compression algorithm. If the response was compressed, the compression algorithm **must** be specified in the response metadata.

## Client Detected Exceptions

Exceptions that are detected purely by the client (rather than being sent over the wire from the server) are represented as [`TTransportException`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/TTransportException.h). Some of the reasons why a client detected exception may be raised include:

- Timeout (server non-responsive)
- Corrupted data (checksum mismatch)
- Socket errors
- [Many more](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/TTransportException.h#TTransportExceptionType)

## Underlying Transport Protocols

The Thrift Interface protocol is a higher level abstraction that defines the behavior of various interfaces, however, it must utilize a lower level transport protocol to actually send requests and receive responses.

### Rocket Protocol

[Rocket protocol](rocket.md) is an implementation of the Thrift Interface protocol using the [RSocket protocol](https://rsocket.io/).

### Header Protocol (Deprecated)

Header protocol is a deprecated transport protocol.
