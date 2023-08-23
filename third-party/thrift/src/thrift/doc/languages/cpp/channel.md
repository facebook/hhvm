---
state: draft
description: Channel in C++
---

# Channels in C++

Prerequisite: [Thrift Channels](/fb/server/channels.md)

## Untyped channel in C++

The C++ API is called RequestChannel and provides several methods with this signature pattern:

```
void sendRequest{Kind}(
      RpcOptions&&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr‹apache::thrift::transport::THeader›,
      {CallbackPtr});
```


CallbackPtr has the following type:

|{Kind}|{CallbackPtr}|
|---|---|
|Response|std::unique_ptr‹RequestClientCallback,Deleter›|
|Stream|StreamClientCallback*|
|Sink|SinkClientCallback*|
|NoReponse (deprecated)|std::unique_ptr‹RequestClientCallback,Deleter›|

Callbacks are fundamental to asynchrony in C++, and provide the flexibility to integrate with the asynchrony pattern in the target language. The callback can fulfill a Promise, trigger an Event, or send a Message for example. Each callback has a specific contract which the channel guarantees to respect, documented under [Callback](#callback)
Callback implementations are typically self-managed (i.e. they are heap-allocated and call `delete this` when completed), though this is not required. This is possible because of the strictness of the aforementioned contracts, which guarantee that a terminal state will be reached once the channel has taken in the callback so that freeing the callback in the terminal state is sufficient to prevent leaks.

### Metadata

Metadata is split between MethodMetadata (which contains information intrinsic to the method like name, kind, and retriability) and THeader, which contains most other fields including serialization protocol and compression information.
[These fields](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp/transport/THeader.cpp) are specified by using a magic key in the unstructured header map.

### Callback

#### Request-response:

```
class RequestClientCallback {
 public:
  virtual void onResponse(ClientReceiveState&&) noexcept = 0;
  virtual void onResponseError(folly::exception_wrapper) noexcept = 0;
}
```

onResponse is called for responses and declared exceptions, and passed a struct containing the serialized response buffer and some metadata. onResponseError is called for undeclared exceptions, including transport errors. It is guaranteed that exactly one of these will be called.

#### Streaming:

```
class StreamClientCallback {
 public:
  FOLLY_NODISCARD virtual bool onFirstResponse(
      FirstResponsePayload&&, folly::EventBase*, StreamServerCallback*) = 0;
  virtual void onFirstResponseError(folly::exception_wrapper) = 0;

  FOLLY_NODISCARD virtual bool onStreamNext(StreamPayload&&) = 0;
  virtual void onStreamError(folly::exception_wrapper) = 0;
  virtual void onStreamComplete() = 0;

  // not terminating
  virtual void resetServerCallback(StreamServerCallback&) = 0;
};
```

onFirstResponse and onFirstResponseError are equivalent to onResponse and onResponseError for request-response, with the exception that onFirstResponse also receives a server callback for sending outgoing payloads and events. The methods form the edges of a state machine documented [here.](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/StreamCallbacks.h) It is guaranteed that the state machine will transition to the terminal state. It is the implementer’s responsibility to respect the per-stream flow control and to return the correct boolean value.
Sink has a similar structure documented [here.](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/async/StreamCallbacks.h)

`StreamServerCallback` callbacks must be called on the EventBase passed into `onFirstResponse`.

### Interactions

To send methods over an interaction you must call `createInteraction,` store the returned object and pass it to `RpcOptions::setInteractionId` for each request, then pass it to `terminateInteraction` to dispose of it. (It will crash the program if destroyed any other way). It is not necessary to wait for responses before calling terminateInteraction.

### Ordering

Multiple requests sent over one channel are not ordered with one another. Requests sent in a single interaction are ordered with one another, but responses (including stream payloads) are not ordered with those for other requests in the same interaction. The payloads of a single stream are ordered.

### Threading

The thread-safety of a RequestChannel is determined by the return value of its `getEventBase()` method.
Channels that return non-null can only be accessed from that EventBase’s thread, including for construction and destruction. (These channels are typically constructed from a `folly::AsyncTransport` and will return the EventBase associated with that transport).
Channels that return nullptr are thread safe: they can be accessed from any thread, including concurrently from multiple threads.
