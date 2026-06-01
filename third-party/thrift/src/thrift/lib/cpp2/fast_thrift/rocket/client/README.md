# FastTransport Rocket Client Handlers

## Overview

This directory contains pipeline handlers for the client-side RSocket
implementation in the **FastTransport** framework. These handlers manage
connection setup and stream state for outbound requests and inbound responses.

---

## Handlers

| Handler | Purpose |
|---------|---------|
| `RocketClientSetupFrameHandler` | Sends the RSocket SETUP frame on connect |
| `RocketClientStreamStateHandler` | Manages stream IDs and request/response correlation |
| `RocketClientRequestResponseFrameHandler` | Duplex handler for REQUEST_RESPONSE interactions (outbound serialization + inbound response handling) |

---

## Pipeline Architecture

```
Outbound: App Layer -> RocketClientStreamStateHandler -> RocketClientRequestResponseFrameHandler -> Transport
Inbound:  App Layer <- RocketClientSetupFrameHandler <- RocketClientStreamStateHandler <- FrameHandler <- Transport
```

Note: `RocketClientSetupFrameHandler` is an inbound handler that sends the setup frame on connect.
All reads pass through unchanged.

---

# RocketClientSetupFrameHandler

## Overview

The `RocketClientSetupFrameHandler` is an **inbound handler** responsible for
sending the RSocket SETUP frame when a client connection is established.

The setup frame is sent immediately via the `onPipelineActivated` event, before any
application requests.

## Key Responsibilities

1. **Setup Frame Serialization**: Serializes and sends the SETUP frame with
   protocol version and connection parameters on connect
2. **Immediate Send**: Sends the SETUP frame immediately when `onPipelineActivated` is
   called (correct RSocket semantics)
3. **Factory Support**: Accepts a factory function for dynamic metadata
   creation at connection time
4. **Passthrough**: All frames pass through unchanged

## Protocol Constants

The handler uses the following RSocket protocol constants:

| Constant | Value | Description |
|----------|-------|-----------|
| `kRSocketMajorVersion` | 1 | RSocket protocol major version |
| `kRSocketMinorVersion` | 0 | RSocket protocol minor version |
| `kMaxKeepaliveTime` | 2^31 - 1 | Max keepalive time (client-side keepalive not supported) |
| `kMaxLifetime` | 2^31 - 1 | Max connection lifetime (not enforced) |

Note: Resume and Lease features are not currently supported, matching the
existing Thrift RSocket implementation.

## Usage Example

```cpp
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientSetupFrameHandler.h>

using namespace apache::thrift::fast_thrift::rocket;

// Create handler with factory function for metadata creation
RocketClientSetupFrameHandler handler(
    // Factory: metadata created at connect time
    [&]() {
      auto metadata = createDynamicMetadata();
      return std::make_pair(std::move(metadata), nullptr);
    });

// Add to pipeline - SETUP frame will be sent when pipeline->activate() is called
// pipeline.addHandler(std::move(handler));
```

## Message Flow

### On Connect (Setup Frame Sent)

```
┌─────────────────────────────────────┐
│ pipeline->activate()                │
└─────────┬───────────────────────────┘
          │
          ▼
┌─────────────────────────────────────┐
│ RocketClientSetupFrameHandler::onPipelineActivated  │
│ 1. Fire connect to next handler     │
│ 2. Call factory to create metadata  │
│ 3. Serialize SETUP frame            │
│ 4. Fire write to send setup frame   │
└─────────┬───────────────────────────┘
          │
          ▼
┌─────────────────────┐
│ SETUP Frame         │
│ (to transport)      │
└─────────────────────┘
```

## Testing

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/client/test:client_setup_frame_handler_test
```

---

# Client Stream State Handler

## Overview

The Client Stream State Handler is a pipeline handler for managing client-side
RSocket stream state in the **FastTransport** framework. It sits between the
application layer and the framing layer, handling stream ID generation and
lifecycle management for outbound requests and inbound responses.

---

## Architecture

### Pipeline Position

```
App Layer <-> RocketClientStreamStateHandler <-> FrameHandler <-> Transport
```

The handler bridges the gap between application-level request/response semantics
and the stream-based RSocket protocol, abstracting away stream ID management
from the application.

### Key Responsibilities

1. **Stream ID Generation**: Generates odd-numbered stream IDs (1, 3, 5, ...)
   per RSocket protocol specification for client-initiated streams
2. **Stream State Tracking**: Maintains a mapping of active stream IDs to their
   associated context (e.g., request callbacks)
3. **Response Correlation**: Matches inbound frames to their originating
   requests using stream IDs
4. **Lifecycle Management**: Cleans up stream state when terminal frames are
   received (ERROR, CANCEL, or PAYLOAD with complete flag)

---

## Message Types

### RocketRequestMessage

Outbound request from the application layer (~30 bytes with pragma pack).
Frame-specific parameters are inlined directly into the struct rather than
using a separate TypeErasedBox, allowing the struct to fit within
TypeErasedBox's inline capacity of 56 bytes.

```cpp
#pragma pack(push, 1)
struct RocketRequestMessage {
  std::unique_ptr<folly::IOBuf> metadata;  // 8B - Request metadata
  std::unique_ptr<folly::IOBuf> data;      // 8B - Request data
  uint32_t streamId{kInvalidStreamId};     // 4B - Assigned by StreamStateHandler
  uint32_t requestHandle{kNoRequestHandle}; // 4B - Opaque handle for response correlation
  uint32_t initialRequestN{0};              // 4B - Initial request N for streaming frames
  frame::FrameType frameType;             // 1B - REQUEST_RESPONSE, REQUEST_STREAM, etc.
  bool complete{false};                     // 1B - Complete flag for REQUEST_CHANNEL
};
#pragma pack(pop)
```

### RocketResponseMessage

Inbound response delivered to the application layer (~45 bytes with pragma pack).

```cpp
#pragma pack(push, 1)
struct RocketResponseMessage {
  frame::read::ParsedFrame frame;              // 40B - The parsed inbound frame
  uint32_t requestHandle{kNoRequestHandle}; // 4B - Opaque handle for response correlation
  frame::FrameType requestFrameType;      // 1B - The original request's frame type
};
#pragma pack(pop)
```

---

## Message Flow

### Outbound (Application → Transport)

```
┌───────────────────────┐
│ RocketRequestMessage  │
│ - metadata            │
│ - data                │
│ - streamId            │
│ - requestHandle       │
│ - initialRequestN     │
│ - frameType           │
│ - complete            │
└─────────┬─────────────┘
          │
          ▼
┌─────────────────────────────────────┐
│ RocketClientStreamStateHandler            │
│ 1. Generate streamId (1, 3, 5, ...) │
│ 2. Store streamId → handle mapping  │
│ 3. Assign streamId to message       │
└─────────┬───────────────────────────┘
          │
          ▼
┌───────────────────────┐
│ RocketRequestMessage  │
│ - metadata            │
│ - data                │
│ - streamId (assigned) │
│ - requestHandle       │
│ - initialRequestN     │
│ - frameType           │
│ - complete            │
└───────────────────────┘
          │
          ▼
    [FrameHandler]
```

### Inbound (Transport → Application)

```
    [FrameHandler]
          │
          ▼
┌─────────────────────┐
│ ParsedFrame         │
│ - streamId          │
│ - frameType         │
│ - payload           │
└─────────┬───────────┘
          │
          ▼
┌──────────────────────────────────────────┐
│ RocketClientStreamStateHandler                 │
│ 1. Check if connection frame → pass thru │
│ 2. Check if terminal frame               │
│ 3. Look up handle by streamId            │
│ 4. Remove from active streams            │
└─────────┬────────────────────────────────┘
          │
          ▼
┌───────────────────────┐
│ RocketResponseMessage │
│ - frame               │
│ - requestHandle       │
│ - requestFrameType    │
└───────────────────────┘
          │
          ▼
    [Application]
```

---

## Usage Example

```cpp
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

using namespace apache::thrift::fast_thrift::rocket;
using channel_pipeline::erase_and_box;

// Create handler
RocketClientStreamStateHandler handler;

// The handler is typically added to a channel pipeline:
// pipeline.addHandler<RocketClientStreamStateHandler>();

// Outbound: Application sends a request
RocketRequestMessage request{
    .metadata = std::move(serializedMetadata),
    .data = std::move(serializedData),
    .requestHandle = myRequestHandle,  // Opaque 4-byte handle for correlation
    .initialRequestN = 0,              // Not used for REQUEST_RESPONSE
    .frameType = frame::FrameType::REQUEST_RESPONSE,
    .complete = false,                 // Not used for REQUEST_RESPONSE
};

// The handler assigns a streamId and stores the streamId → handle mapping
// The message is forwarded to the next handler with streamId assigned

// Inbound: Handler receives RocketResponseMessage and:
// - Passes through connection frames (streamId == 0)
// - Passes through non-terminal frames
// - For terminal frames: looks up handle, removes stream, attaches handle to response
```

---

## Terminal Frame Detection

A frame is considered terminal (ending the stream) if it is one of:

- **ERROR frame**: Stream failed with an error
- **CANCEL frame**: Stream was cancelled
- **PAYLOAD frame with complete flag**: Normal stream completion

Non-terminal frames (e.g., REQUEST_N for flow control) pass through without
affecting stream state.

---

## Error Handling

### Exception Handling Design

Exception handling methods return `void` because exceptions must always be
handled/propagated — they represent terminal failures, not flow control signals:

```cpp
template <typename Context>
void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
  // Fail all active streams with the error
  failAllActiveStreams(ctx, e);
  ctx.fireException(std::move(e));
}
```

### Backpressure Semantics

`Result::Backpressure` is a **soft signal**, not an error:
- The current operation **was accepted** and will be processed
- It is an explicit signal telling the handler to **slow down**
- Handlers **may or may not** choose to respect this signal
- Unlike errors, backpressure does not require terminating or failing the stream

### Connection-Level Errors

When `onException` is called (e.g., transport failure), the handler:

1. Iterates over all active streams
2. Fires a `ClientResponseMessage` with the error for each stream
3. Clears the active streams map
4. Propagates the exception to the next handler

### Unknown Stream ID

If a frame arrives for an unknown stream ID, the handler logs an error and
drops the frame. This can happen if:

- The stream was already terminated
- A protocol error occurred
- The frame is malformed

---

## Testing

Run the unit tests with:

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/client/test:client_stream_state_handler_test
```

---

---

# RocketClientRequestResponseFrameHandler

## Overview

The `RocketClientRequestResponseFrameHandler` is a **duplex handler** that manages both
outbound request serialization and inbound response handling for REQUEST_RESPONSE
streams. It tracks which stream IDs belong to request-response interactions to
properly route inbound frames.

---

## Architecture

### Pipeline Position

```
App <-> RocketClientSetupFrameHandler <-> RocketClientStreamStateHandler <-> RocketClientRequestResponseFrameHandler <-> Transport
```

The handler receives `StreamRequestMessage` from `RocketClientStreamStateHandler` on the
outbound path and serializes it into wire-format frames. On the inbound path, it
checks if frames belong to tracked request-response streams and handles them
accordingly.

### Key Responsibilities

1. **Outbound Serialization**: Converts `StreamRequestMessage` with
   `REQUEST_RESPONSE` frame type into wire-format frames
2. **Stream Tracking**: Maintains a set of stream IDs for active request-response
   interactions
3. **Response Routing**: Intercepts inbound frames for tracked streams and validates
   them before forwarding
4. **Frame Validation**: Ensures response payloads have proper flags (next or complete)

---

## Message Flow

### Outbound (Application → Transport)

```
┌──────────────────────┐
│ StreamRequestMessage │
│ - streamId           │
│ - frameType          │
│ - payload            │
└─────────┬────────────┘
          │
          ▼
┌─────────────────────────────────────────┐
│ RocketClientRequestResponseFrameHandler       │
│ 1. Check if REQUEST_RESPONSE frame type │
│ 2. Track stream ID                      │
│ 3. Serialize to wire format             │
└─────────┬───────────────────────────────┘
          │
          ▼
┌─────────────────────┐
│ std::unique_ptr<    │
│   folly::IOBuf>     │
│ (serialized frame)  │
└─────────────────────┘
          │
          ▼
    [Transport]
```

### Inbound (Transport → Application)

```
    [Transport]
          │
          ▼
┌─────────────────────┐
│ ParsedFrame         │
│ - streamId          │
│ - frameType         │
│ - payload           │
└─────────┬───────────┘
          │
          ▼
┌──────────────────────────────────────────────┐
│ RocketClientRequestResponseFrameHandler            │
│ 1. Check if stream is tracked                │
│    - If not: forward to next handler         │
│ 2. Remove from tracking (response received)  │
│ 3. Validate frame type and flags             │
│ 4. Forward to next handler                   │
└─────────┬────────────────────────────────────┘
          │
          ▼
    [Next Handler]
```

---

## Response Frame Validation

The handler validates inbound frames for request-response streams:

| Frame Type | Validation | Action |
|------------|------------|--------|
| PAYLOAD | Must have `next` or `complete` flag | Forward if valid, error otherwise |
| ERROR | Always valid | Forward |
| Other | Invalid for request-response | Return error |

---

## Usage Example

```cpp
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

using namespace apache::thrift::fast_thrift::rocket;

// Create handler
RocketClientRequestResponseFrameHandler handler;

// The handler is typically added to a channel pipeline after RocketClientStreamStateHandler
// pipeline.addHandler<RocketClientRequestResponseFrameHandler>();

// Outbound: Handler receives StreamRequestMessage from RocketClientStreamStateHandler
// and serializes REQUEST_RESPONSE frames to wire format

// Inbound: Handler intercepts responses for tracked streams
// and forwards them after validation
```

---

## Error Handling

### Write Failures

If `fireWrite` fails when sending a request frame:
- The stream ID is removed from tracking
- The error result is propagated to the caller

### Invalid Responses

If an inbound frame has invalid flags or unexpected frame type:
- An error is logged
- `Result::Error` is returned
- The stream is removed from tracking

### Connection Errors

When `onException` is called:
- All tracked streams are cleared
- The exception is propagated to the next handler

---

## Testing

Run the unit tests with:

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/client/test:client_request_response_frame_handler_test
```

---

## Code Standards

All code follows:
- Thrift C++ Coding Guidelines
- C++ Core Guidelines naming conventions
