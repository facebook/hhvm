# FastTransport Rocket Server Handlers

## Overview

This directory contains pipeline handlers for the server-side RSocket
implementation in the **FastTransport** framework. These handlers manage
connection setup validation, request-response frame serialization, and
stream state for inbound requests and outbound responses.

---

## Handlers

| Handler | Purpose |
|---------|---------|
| `RocketServerFrameCodecHandler` | Parses raw IOBuf frames into ParsedFrame on the read path; passthrough on the write path |
| `RocketServerSetupFrameHandler` | Validates and consumes the RSocket SETUP frame on connection establishment |
| `RocketServerStreamStateHandler` | Manages stream IDs and request/response routing for client-initiated streams; consumes connection-level frames |
| `RocketServerRequestResponseFrameHandler` | Duplex handler for REQUEST_RESPONSE interactions (inbound request tracking + outbound response serialization) |

---

## Pipeline Architecture

```
Inbound:  Transport -> FrameHandler -> RocketServerFrameCodecHandler -> RocketServerSetupFrameHandler -> RocketServerRequestResponseFrameHandler -> RocketServerStreamStateHandler -> App
Outbound: Transport <- FrameHandler <- RocketServerFrameCodecHandler <- RocketServerSetupFrameHandler <- RocketServerRequestResponseFrameHandler <- RocketServerStreamStateHandler <- App
```

Note: `RocketServerFrameCodecHandler` parses raw IOBuf into ParsedFrame and
validates frames. `RocketServerSetupFrameHandler` validates the first frame is a valid
SETUP and consumes it (does not forward downstream), then becomes a near-zero-cost
passthrough. `RocketServerRequestResponseFrameHandler` tracks REQUEST_RESPONSE streams
and serializes their response frames. `RocketServerStreamStateHandler` manages active
stream state for all stream types and consumes connection-level frames (streamId == 0).

---

# RocketServerSetupFrameHandler

## Overview

The `RocketServerSetupFrameHandler` is a **duplex handler** responsible for
validating the RSocket SETUP frame when a client connection is established.

It uses a two-phase design:
- **Phase 1 (awaiting setup)**: Validates the first frame is SETUP with
  correct version and timer values. Rejects invalid setups with ERROR.
- **Phase 2 (setup complete)**: Passthrough for all frames, but rejects
  duplicate SETUP frames.

## Key Responsibilities

1. **Setup Validation**: Ensures the first frame is a SETUP frame with a
   supported major version, non-zero keepalive time, and non-zero max lifetime
2. **Parameter Storage**: Stores negotiated setup parameters (version,
   keepalive, lifetime, lease) for access by other components
3. **Error Reporting**: Sends ERROR frames with appropriate error codes for
   invalid or unsupported setups
4. **Connection Termination**: Closes the connection after sending setup errors,
   per RSocket spec
5. **SETUP Consumption**: Consumes valid SETUP frames (does not forward them
   downstream), since SETUP is a connection-level protocol frame

## Setup Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `kInvalidSetup` | 0x00000001 | First frame not SETUP, duplicate SETUP, or invalid parameters |
| `kUnsupportedSetup` | 0x00000002 | Unsupported RSocket major version |

## Stored Parameters

After successful setup, the following parameters are available via `setupParameters()`:

| Parameter | Type | Description |
|-----------|------|-------------|
| `majorVersion` | `uint16_t` | RSocket protocol major version |
| `minorVersion` | `uint16_t` | RSocket protocol minor version |
| `keepaliveTime` | `uint32_t` | Keepalive interval in milliseconds |
| `maxLifetime` | `uint32_t` | Max connection lifetime in milliseconds |
| `hasLease` | `bool` | Whether the client supports the LEASE mechanism |

## Usage Example

```cpp
#include <thrift/lib/cpp2/fast_thrift/rocket/server/RocketServerSetupFrameHandler.h>

using namespace apache::thrift::fast_thrift::rocket::server;

// Create handler
RocketServerSetupFrameHandler handler;

// Add to pipeline - SETUP frame will be validated on first inbound read
// pipeline.addHandler(std::move(handler));

// After setup completes, query negotiated parameters:
// if (handler.isSetupComplete()) {
//   auto& params = handler.setupParameters();
//   configureKeepalive(params.keepaliveTime);
// }
```

## Message Flow

### Setup Validation (First Frame)

```
┌─────────────────────┐
│ ParsedFrame (SETUP) │
│ from FrameHandler   │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────────────────────────┐
│ RocketServerSetupFrameHandler::onRead         │
│ 1. Verify frame type is SETUP           │
│ 2. Validate major version               │
│ 3. Validate keepaliveTime > 0           │
│ 4. Validate maxLifetime > 0             │
│ 5. Store parameters                     │
│ 6. Consume frame (not forwarded)        │
└─────────────────────────────────────────┘
```

### Invalid Setup (Error Response)

```
┌─────────────────────┐
│ Invalid SETUP frame │
└─────────┬───────────┘
          │
          ▼
┌─────────────────────────────────────────┐
│ RocketServerSetupFrameHandler::sendError      │
│ 1. Serialize ERROR frame                │
│ 2. Fire write to send ERROR to client   │
│ 3. Close the connection                 │
└─────────────────────────────────────────┘
```

## Testing

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/server/test:server_setup_frame_handler_test
```

---

# RocketServerStreamStateHandler

## Overview

The `RocketServerStreamStateHandler` is a **duplex handler** that manages server-side
RSocket stream state. It tracks active streams initiated by clients and routes
responses back through the pipeline.

## Key Responsibilities

1. **Stream Registration**: Registers new streams on request-initiating frames
   (REQUEST_RESPONSE, REQUEST_FNF, REQUEST_STREAM, REQUEST_CHANNEL)
2. **Stream ID Tracking**: Tracks client-assigned odd-numbered stream IDs
   (1, 3, 5, ...)
3. **Terminal Frame Handling**: Removes streams on CANCEL, ERROR, or complete
   PAYLOAD frames
4. **Response Routing**: Forwards `RocketResponseMessage` from the app layer
   to the framing layer
5. **Transactional Rollback**: Re-adds streams if write fails after removal

## Message Types

### RocketRequestMessage (Inbound)

Delivered to the application layer when a client sends a request:

```cpp
struct RocketRequestMessage {
  frame::read::ParsedFrame frame;
  folly::exception_wrapper error;  // Set on connection failure
  uint32_t streamId{0};
};
```

### RocketResponseMessage (Outbound)

Sent by the application layer to respond to a client request:

```cpp
struct RocketResponseMessage {
  std::unique_ptr<folly::IOBuf> payload;
  std::unique_ptr<folly::IOBuf> metadata;
  uint32_t streamId{0};
  uint32_t errorCode{0};
  bool complete{true};  // Terminal response removes the stream
};
```

## Message Flow

### Inbound (Transport → Application)

```
┌─────────────────────┐
│ ParsedFrame         │
│ - streamId          │
│ - frameType         │
│ - payload           │
└─────────┬───────────┘
          │
          ▼
┌──────────────────────────────────────────┐
│ RocketServerStreamStateHandler                 │
│ 1. Connection frame (streamId=0) → drop  │
│ 2. Request frame → register stream       │
│ 3. Terminal frame → remove stream        │
│ 4. Other frame → pass if stream active   │
│ 5. Unknown streamId → log and drop       │
└─────────┬────────────────────────────────┘
          │
          ▼
┌───────────────────────┐
│ RocketRequestMessage  │
│ - streamId            │
│ - frame               │
└───────────────────────┘
          │
          ▼
    [Application]
```

### Outbound (Application → Transport)

```
    [Application]
          │
          ▼
┌───────────────────────┐
│ RocketResponseMessage │
│ - streamId            │
│ - payload             │
│ - complete            │
└─────────┬─────────────┘
          │
          ▼
┌──────────────────────────────────────────┐
│ RocketServerStreamStateHandler                 │
│ 1. Validate streamId is active           │
│ 2. If complete, remove stream            │
│ 3. Forward RocketResponseMessage         │
│ 4. Roll back removal if write fails      │
└─────────┬────────────────────────────────┘
          │
          ▼
┌───────────────────────┐
│ RocketResponseMessage │
│ - streamId            │
│ - payload             │
└───────────────────────┘
          │
          ▼
    [RocketServerRequestResponseFrameHandler]
```

## Error Handling

### Connection-Level Errors

When `onException` is called (e.g., transport failure), the handler:

1. Clears all active streams
2. Propagates the exception to the next handler

### Unknown Stream ID

If a frame arrives for an unknown stream ID, the handler logs a warning and
drops the frame. This can happen if:

- The stream was already terminated
- A protocol error occurred
- The frame is malformed

## Testing

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/server/test:server_stream_state_handler_test
```

---

# RocketServerRequestResponseFrameHandler

## Overview

The `RocketServerRequestResponseFrameHandler` is a **duplex handler** that manages both
inbound request tracking and outbound response serialization for REQUEST_RESPONSE
streams. It tracks which stream IDs belong to request-response interactions to
properly serialize outbound response frames.

---

## Architecture

### Pipeline Position

```
App <-> RocketServerStreamStateHandler <-> RocketServerRequestResponseFrameHandler <-> RocketServerSetupFrameHandler <-> FrameHandler <-> Transport
```

The handler receives `ParsedFrame` from `RocketServerSetupFrameHandler` on the inbound
path and tracks REQUEST_RESPONSE streams. On the outbound path, it receives
`RocketResponseMessage` from `RocketServerStreamStateHandler` and serializes tracked
request-response responses into wire-format PAYLOAD frames.

### Key Responsibilities

1. **Inbound Request Tracking**: Detects incoming REQUEST_RESPONSE frames and
   tracks their stream IDs
2. **Stream Tracking**: Maintains a set of stream IDs for active request-response
   interactions
3. **Outbound Serialization**: Converts `RocketResponseMessage` for tracked streams
   into wire-format PAYLOAD frames with complete and next flags set
4. **Terminal Frame Cleanup**: Removes tracking when CANCEL or ERROR frames arrive
   for tracked streams

---

## Message Flow

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
┌──────────────────────────────────────────────────┐
│ RocketServerRequestResponseFrameHandler                │
│ 1. Check if REQUEST_RESPONSE frame type          │
│    - If yes: track stream ID, forward            │
│    - Roll back tracking if downstream fails      │
│ 2. Check if terminal frame (CANCEL, ERROR)       │
│    - If yes: remove tracking for stream          │
│ 3. Forward frame to next handler                 │
└─────────┬────────────────────────────────────────┘
          │
          ▼
    [RocketServerStreamStateHandler]
```

### Outbound (Application → Transport)

```
    [RocketServerStreamStateHandler]
          │
          ▼
┌───────────────────────┐
│ RocketResponseMessage │
│ - streamId            │
│ - payload             │
└─────────┬─────────────┘
          │
          ▼
┌──────────────────────────────────────────────────┐
│ RocketServerRequestResponseFrameHandler                │
│ 1. Check if stream is tracked                    │
│    - If not: forward unchanged                   │
│ 2. Remove from tracking                          │
│ 3. Serialize payload as PAYLOAD frame            │
│    (complete=true, next=true)                    │
│ 4. Re-add tracking if write fails                │
└─────────┬────────────────────────────────────────┘
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

---

## Usage Example

```cpp
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

using namespace apache::thrift::fast_thrift::rocket::server;

// Create handler
RocketServerRequestResponseFrameHandler handler;

// The handler is typically added to a channel pipeline after RocketServerSetupFrameHandler
// and before RocketServerStreamStateHandler
// pipeline.addHandler<RocketServerRequestResponseFrameHandler>();

// Inbound: Handler tracks REQUEST_RESPONSE streams and forwards frames
// to RocketServerStreamStateHandler

// Outbound: Handler serializes responses for tracked streams into
// PAYLOAD frames and forwards to transport
```

---

## Error Handling

### Write Failures

If `fireWrite` fails when sending a response frame:
- The stream ID is re-added to tracking (app can retry)
- The error result is propagated to the caller

### Backpressure Semantics

`Result::Backpressure` is a **soft signal**, not an error:
- The current operation **was accepted** and will be processed
- It is an explicit signal telling the handler to **slow down**
- Handlers **may or may not** choose to respect this signal
- The stream remains active and valid; no rollback or retry is needed

### Terminal Frames

When a CANCEL or ERROR frame arrives for a tracked stream:
- The stream ID is removed from tracking
- The frame is forwarded to the next handler

### Connection Errors

When `onException` is called:
- All tracked streams are cleared
- The exception is propagated to the next handler

---

## Testing

Run the unit tests with:

```bash
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/rocket/server/test:server_request_response_frame_handler_test
```

---

## Code Standards

All code follows:
- Thrift C++ Coding Guidelines
- C++ Core Guidelines naming conventions
