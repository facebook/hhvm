# Thrift Client Layer

This directory contains the Thrift-layer client implementation for fast_thrift.
It provides the bridge between Apache Thrift's `RequestChannel` API and the
fast_thrift channel pipeline.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Application Layer                                   │
│                    (Generated Thrift Client Code)                            │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                       ThriftClientChannel                                    │
│  - Implements apache::thrift::RequestChannel                                 │
│  - Manages pending callbacks keyed by requestId                              │
│  - Receives responses via onMessage() (ClientInboundAppAdapter)                    │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │ ThriftClientRequest           │
                    │ ThriftClientResponse          │
                    └───────────────┬───────────────┘
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                  ThriftClientRequestResponseHandler                          │
│  - Builds ClientReceiveState from ThriftResponseMessage                      │
│  - Populates THeader from ResponseRpcMetadata                                │
│  - Pass-through for outbound messages                                        │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │ ThriftClientRequest           │
                    │ ThriftResponseMessage         │
                    └───────────────┬───────────────┘
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ThriftClientMetadataHandler                               │
│  - Serializes RequestRpcMetadata (outbound)                                  │
│  - Deserializes ResponseRpcMetadata (inbound)                                │
│  - Passes requestHandle through to Rocket layer                              │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │ RocketRequestMessage          │
                    │ RocketResponseMessage         │
                    └───────────────┬───────────────┘
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Rocket Layer                                       │
│                    (../rocket/client/...)                                    │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Files

### Messages.h

Defines the message types used for communication between layers:

| Type | Direction | Description |
|------|-----------|-------------|
| `ThriftClientRequest` | Outbound | Channel → Pipeline entry point with structured metadata |
| `ThriftClientResponse` | Inbound | Pipeline → Channel final response with `ClientReceiveState` or error |
| `ThriftResponseMessage` | Internal | Handler-to-handler with parsed `ResponseRpcMetadata` |

### ThriftClientChannel.h / .cpp

The main entry point implementing `apache::thrift::RequestChannel`.

**Key Features:**
- Factory method: `ThriftClientChannel::newChannel(socket, protocolId)`
- Owns `TransportHandler` (created from socket in constructor)
- Request-response RPC via `sendRequestResponse()`
- Callback correlation using `pendingCallbacks_` map keyed by `requestId`
- Receives pipeline responses via `onMessage()` (ClientInboundAppAdapter concept)
- Handles unsupported frame types by logging error and closing pipeline

**Usage:**
```cpp
// Create channel with socket - channel owns the TransportHandler
auto channel = ThriftClientChannel::newChannel(std::move(socket));

// Build pipeline using channel's transport handler
auto pipeline = PipelineBuilder<ThriftClientChannel, TransportHandler, Allocator>()
    .setEventBase(channel->getEventBase())
    .setHead(channel.get())
    .setTail(channel->transportHandler())
    .setAllocator(&allocator)
    // ... add handlers ...
    .build();

channel->setPipeline(std::move(pipeline));

// Use with generated Thrift client
auto client = MyService::newClient(std::move(channel));
co_await client->co_myMethod(request);
```

**Internal Methods:**
- `handleRequestResponse()` - Processes REQUEST_RESPONSE frames, correlates responses with pending callbacks

**Currently Implemented:**
- ✅ Request-Response (`sendRequestResponse`)

**Not Yet Implemented:**
- ❌ One-way (`sendRequestNoResponse`)
- ❌ Streaming (`sendRequestStream`)
- ❌ Sink (`sendRequestSink`)

### ThriftClientMetadataHandler.h

Pipeline handler for RPC metadata serialization/deserialization at the
Thrift/Rocket boundary.

**Outbound (onWrite):**
1. Receives `ThriftClientRequest` with structured `RequestRpcMetadata`
2. Serializes metadata using Binary protocol
3. Passes `requestHandle` through for response correlation
4. Produces `RocketRequestMessage`

**Inbound (onRead):**
1. Receives `RocketResponseMessage` from Rocket layer
2. Extracts `requestHandle` from the response
3. Deserializes `ResponseRpcMetadata` from frame metadata
4. Processes payload metadata (handles declared/undeclared exceptions)
5. Produces `ThriftResponseMessage`

### ThriftClientRequestResponseHandler.h

Inbound-only handler that processes request-response results.

**Responsibilities:**
- Builds `THeader` from `ResponseRpcMetadata`
- Creates `ClientReceiveState` with correct `protocolId` (from constructor) and `MessageType`
- Distinguishes error responses (undeclared exceptions) from payload responses
- Produces `ThriftClientResponse` for delivery to `ThriftClientChannel`

**Pass-through:** Outbound messages are forwarded unchanged.

## Key Concepts

### Request Handle

The `requestHandle` is a `uint32_t` opaque identifier used to correlate responses
with their original requests:

```cpp
// Messages use requestHandle for callback correlation
struct ThriftClientRequest {
  uint32_t requestHandle{rocket::kNoRequestHandle};  // Set by channel
  // ...
};
```

- The channel generates unique `requestHandle` values for each request
- The Rocket layer stores `requestHandle` by `streamId` for response correlation
- Responses carry the `requestHandle` back to the channel for callback lookup

### Protocol ID

`protocolId` (T_COMPACT_PROTOCOL, T_BINARY_PROTOCOL, etc.) is a per-connection
setting, not per-request. Since generated code sets `protocolId` on the channel
after construction, `ThriftClientRequestResponseHandler` takes a getter function
(not a fixed value) at construction time:

```cpp
// Handler is constructed with a getter that fetches protocolId from the channel
ThriftClientRequestResponseHandler handler(
    [channel]() { return channel->getProtocolId(); });
```

This getter is invoked when creating `ClientReceiveState` for each response.

## Testing

Tests are located in `test/`:

```bash
# Run all tests
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/test:

# Run specific test target
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/test:thrift_client_channel_test
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/test:thrift_client_metadata_handler_test
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/test:thrift_client_request_response_handler_test
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/test:thrift_client_integration_test
```

### Test Files

| File | Coverage |
|------|----------|
| `ThriftClientChannelTest.cpp` | Factory, pipeline integration, callback correlation, error handling |
| `ThriftClientMetadataHandlerTest.cpp` | Metadata serialization, deserialization, exception handling |
| `ThriftClientRequestResponseHandlerTest.cpp` | ClientReceiveState construction, error/payload paths |
| `ThriftClientIntegrationTest.cpp` | End-to-end integration tests for the Thrift client pipeline |

### Integration Tests

The `ThriftClientIntegrationTest.cpp` file provides end-to-end integration tests that verify
the complete Thrift client pipeline works correctly with actual request-response flows.
These tests exercise the full handler chain from `ThriftClientChannel` through the
metadata and request-response handlers to the Rocket layer.

## Dependencies

- **channel_pipeline** - Pipeline framework (`PipelineImpl`, `TypeErasedBox`, `Result`)
- **fast_thrift/frame** - Frame types and parsing (`FrameType`, `ParsedFrame`)
- **fast_thrift/rocket/client** - Rocket-layer messages (`RocketRequestMessage`, `RocketResponseMessage`)
- **thrift/lib/cpp2** - Thrift types (`RequestChannel`, `ClientReceiveState`, `RpcMetadata`)
