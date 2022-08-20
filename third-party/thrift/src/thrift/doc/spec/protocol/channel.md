---
state: draft
---

# Channels

The *channel* is the interface to Thrift RPC in each target language. While channels exist on both client and server, the term by itself commonly refers to the client channel. Here, the client and server are the two parties to the RPC where the client is the initiator.
A client channel is bound to a conceptual group of server channels that can respond to RPCs (e.g. a service or library instance). There may or may not be one or more remote hosts that requests can be routed to, or an in-process handler. An interface may return a client channel (by adapting a struct with instructions for creating it) to delegate future RPCs to a different service.
The concept of a channel does not have any relation to TCP or other connections to the server. Channel implementations might own or share ownership of one or several connections, might own no connections (opening them as needed or using an external pool), or might be communicating to an in-memory server without any connections involved.

# Client channel

## Structure

Users of a thrift service typically interact with a “generated client”, which is a native object in each target language on which service methods are called in a strongly-typed way. To avoid confusion, this document will call this “generated client” a *typed client channel*. The typed channel is a friendly interface that allows the target language to typecheck, autocomplete, and otherwise provide developer functionality. It **should** wrap an *untyped client channel*, which is a class responsible for implementing the mechanics of RPC. The typed channel is just responsible for type erasure via serializing the arguments and method info and deserializing the response and for interfacing with the native asynchrony primitives. Following this structure provides the best experience for users of the thrift implementation while minimizing the complexity of the generated code. Further, target language implementations of the *untyped client channel* **should** wrap the [C++ implementation](../../cpp/channel) whenever technically feasible, as direct implementation is involved and carries a significant ongoing maintenance burden.

## Interface

### Typed channel

The typed channel is responsible for the core abstraction of RPC, namely that the client is calling a regular method in its native language. It should be a native class/object with one or more methods corresponding to each method in the service.
Each method may have multiple overloads for supporting the optional RpcOptions argument and different asynchrony models (including blocking if desired). The target language’s idiomatic awaitable type should be used in at least one of the options, when applicable.
The methods should have an argument list that corresponds to that of the method in the IDL, accepting native and generated types.
The typed channel must support four kinds of methods. The return types are specified below, using the following placeholders:
- ResponseType, PayloadType, FinalResponseType: data types
- Awaitable: the target language's native primitive for asynchronous return values (e.g. Future, Awaitable)
- Generator: the target language's native primitive for asynchronous streams of values (e.g. AsyncGenerator, Flowable)

1. Request-response: Awaitable‹ResponseType›
2. Request-no-response: Awaitable‹void› completed when the request is sent or enqueued for sending. May also return void synchronously when only enqueuing is known and is always synchronous.
3. Stream: Awaitable‹ResponseType, Generator‹PayloadType››
    This kind of method behaves like a request-response RPC that returns a generator of values alongside the initial response. This generator asynchrously produces values until either the client cancels the stream or the server completes it.
    This native primitive must expose a mechanism for requesting cancellation (e.g. destruction or a method call).
    The stream is flow-controlled on a payload basis, and the native primitive should support propagating backpressure from the application level by requesting more payloads as the application consumes its internal buffer.
4. Sink: Awaitable‹ResponseType, (Generator‹PayloadType›→Awaitable‹FinalResponseType›)›
    This kind of method behaves like a request-response RPC that returns a (functor that accepts a generator of values and asynchronously returns an additional response) alongside the initial response.
    The caller passes its generator (again a native primitive like AsyncGenerator or Flowable) to the functor, which drives the generator and sends the produced payloads to the server until the generator completes or the server sends its final response. When the final response is received before the generator completes, the generator is destroyed without being completed.
    The stream is flow-controlled on a payload basis, and the functor (part of the Thrift implementation in the target language) is responsible for respecting this flow control.

Metadata / headers may be attached to any payload in any of these requests.
Any of these method kinds except oneway / request-no-response can also create [Interactions](#interactions)

The typed channel’s methods should arguments corresponding to those declared in the IDL, as well as an optional first argument of type [RpcOptions](#rpcoptions) which the typed channel copies and sets the interaction id on if applicable and passes through to the untyped channel.

### Untyped channel

The untyped channel has a structure corresponding to this schema:
```
service UntypedChannel {
  binary sendRequestResponse(
    1: RpcOptions options,
    2: RequestRpcMetadata metadata,
    3: binary serializedRequest
  )
  oneway void sendRequestNoResponse(
    1: RpcOptions options,
    2: RequestRpcMetadata metadata,
    3: binary serializedRequest
  )
  binary, stream<binary> sendRequestStream(
    1: RpcOptions options,
    2: RequestRpcMetadata metadata,
    3: binary serializedRequest
  )
  binary, sink<binary, binary> sendRequestSink(
    1: RpcOptions options,
    2: RequestRpcMetadata metadata,
    3: binary serializedRequest
  )

  InteractionId createInteraction(1: string name)
  void terminateInteraction(1: InteractionId id)
}
```

Implementations have freedom with bridging to their language's async primitives. For example, all methods in C++ return void and take a callback parameter that receives the return type.

#### RpcOptions

Existing implementations use this to set some per-request options like timeouts and stream buffer size. Unless using [Interactions](#interactions), all fields are optional. Potentially relevant fields are documented [inline](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/RpcOptions.h)

#### Metadata

Documented [inline](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/RpcMetadata.thrift)
Note the first few fields are mandatory:

- Protocol used to serialize the request body (compact/binary)
- Compression used on serialized request, if applicable (zlib / zstd)
- Method name and kind

Common other fields:

- timeouts (queue and overall)
- unstructured headers (string to string map)
- checksum

#### SerializedRequest

Method arguments wrapped in an anonymous struct and serialized as described in the [interface spec](interface/index.md#serialization-details).

#### Response

A serialized response is received for all methods except oneway (including void-returning methods) that may contain a serialized representation of the declared response type, one of the exceptions declared in the `throws` list of the method, or a `TApplicationException` for undeclared exceptions. In case of exception, the following information will be placed in the unstructured response headers:

- ‘ex’ : an error code from [this list](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/ResponseChannel.cpp)
- ‘uex’: the type name of the exception
- ‘uexw’: a description of the exception

If the exception comes from the second hop of a proxied request a ‘p’ is prepended to these magic keys.

The content of the serialized response is described in the [interface spec](interface/index.md#response).

## Interactions

A method can create an interaction and then behaves like one of the four other method kinds. It returns an interaction handle, which is a typed channel that also owns the interaction. Destroying or disposing of this handle sends a signal to the server that terminates the interaction, allowing it to free resources.
One of the design goals of interactions is that it is possible to complete the entire lifecycle in a single round trip. The untyped interface permits sending requests on and then terminating the interaction before the response is received; this is why IDs are client-generated. Languages in which performance-sensitive applications are written will likely want to support returning the interaction handle alongside the [first] response of the method and immediately (e.g. `(Awaitable‹InteractionHandle, ResponseStruct›)` and `(InteractionHandle, Awaitable‹ResponseStruct›)`. respectively) in their typed channel. Otherwise, the first structure is generally considered simpler.

## Ordering

Multiple requests sent over one channel are not ordered with one another. Requests sent in a single interaction are ordered with one another, but responses (including stream payloads) are not ordered with those for other requests in the same interaction. The payloads of a single stream are ordered.


# Server channel

Implementations **should** similarly generate a typed server channel interface in the target language using native types and asynchrony that is called into by an untyped channel. The details here are still in active development, as the server team comes up with an architecture that minimizes the implementation burden of non-C++ servers via an in-process or IPC server proxy. The server channel interface in C++ is currently called AsyncProcessorFactory.
