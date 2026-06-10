# FastThrift Framing

High-performance RSocket frame parsing and serialization designed for the Channel Pipeline architecture.

## Overview

This module provides efficient frame handling for RSocket protocol frames used in Thrift transport. The design prioritizes:

- **Parse-once semantics**: Common header fields cached on first parse
- **Inline storage compatibility**: 32-byte `FrameMetadata` fits in `TypeErasedBox`
- **Lazy field access**: Frame-specific fields parsed on-demand via typed views
- **Zero-copy operations**: Move semantics throughout, no cloning
- **Clear separation of concerns**: Reading and writing code in separate directories

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ Frame Flow                                                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Writing (write/)                      Reading (read/)                │
│   ──────────────────                      ──────────────────                │
│                                                                             │
│   ┌──────────────┐                        ┌──────────────┐                  │
│   │ FrameHeaders │                        │    IOBuf     │                  │
│   │   (typed)    │                        │   (bytes)    │                  │
│   └──────┬───────┘                        └──────┬───────┘                  │
│          │                                       │                          │
│          ▼                                       ▼                          │
│   ┌──────────────┐                        ┌──────────────┐                  │
│   │ serialize()  │                        │ parseFrame() │                  │
│   │ (FrameWriter)│                        │(FrameParser) │                  │
│   └──────┬───────┘                        └──────┬───────┘                  │
│          │                                       │                          │
│          ▼                                       ▼                          │
│   ┌──────────────┐                        ┌──────────────┐                  │
│   │    IOBuf     │                        │ ParsedFrame  │                  │
│   │   (bytes)    │                        │  (32 bytes)  │◄─── Fits in      │
│   └──────────────┘                        └──────┬───────┘     TypeErasedBox│
│                                                  │                          │
│                                                  ▼                          │
│                                           ┌──────────────┐                  │
│                                           │ *View types  │ ◄── Lazy parsing │
│                                           │ (on-demand)  │     of extra     │
│                                           └──────────────┘     fields       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
frame/
├── BUCK                      # Build configuration (shared protocol targets)
├── README.md                 # This file
├── FrameType.h               # FrameType enum, Flags class, protocol constants
├── FrameDescriptor.h         # Flyweight frame type metadata
│
├── handler/               # Shared wire codec
│   ├── BUCK
│   └── FrameCodecHandler.h    # BytesPtr ↔ ParsedFrame/ComposedFrame
│
├── read/                  # Incoming frame handling
│   ├── BUCK                  # Reading library targets
│   ├── FrameMetadata.h       # 32-byte cached header fields
│   ├── ParsedFrame.h         # Main frame representation
│   ├── FrameParser.h         # parseFrame() / tryParseFrame()
│   ├── FrameViews.h          # Lazy typed views (*View classes)
│   └── test/
│       ├── BUCK
│       ├── FrameMetadataTest.cpp
│       ├── FrameParserTest.cpp
│       ├── FrameViewsTest.cpp
│       └── ParsedFrameTest.cpp
│
├── write/                  # Outgoing frame handling
│   ├── BUCK                  # Writing library targets
│   ├── FrameHeaders.h        # Typed header structs for writing
│   ├── FrameWriter.h         # serialize() function declarations
│   ├── FrameWriter.cpp       # serialize() implementations
│   └── test/
│       ├── BUCK
│       └── FrameWriterTest.cpp
│
├── bench/                    # Performance benchmarks
│   ├── BUCK
│   ├── FrameParsingBench.cpp
│   └── FrameWritingBench.cpp
│
└── test/                     # Shared protocol tests
    ├── BUCK
    └── FrameDescriptorTest.cpp
```

## Components

### Shared Protocol Types (frame/)

| Component | File | Purpose |
|-----------|------|---------|
| `FrameType` | `FrameType.h` | Enum of all RSocket frame types |
| `Flags` | `FrameType.h` | 10-bit frame flags (metadata, follows, complete, etc.) |
| `kBaseHeaderSize` | `FrameType.h` | Protocol constants (header sizes) |
| `FrameDescriptor` | `FrameDescriptor.h` | Flyweight with type metadata (shared, immutable) |

### Reading (read/)

| Component | File | Purpose |
|-----------|------|---------|
| `FrameMetadata` | `read/FrameMetadata.h` | Cached header fields (32 bytes, inline-storable) |
| `ParsedFrame` | `read/ParsedFrame.h` | Owns buffer + metadata, main frame representation |
| `parseFrame()` | `read/FrameParser.h` | One-shot parsing, caches common fields |
| `tryParseFrame()` | `read/FrameParser.h` | Defensive variant, returns empty on failure |
| `*View` types | `read/FrameViews.h` | Lazy accessors for frame-specific fields |

### Writing (write/)

| Component | File | Purpose |
|-----------|------|---------|
| `*Header` structs | `write/FrameHeaders.h` | Typed headers for each frame type |
| `serialize()` | `write/FrameWriter.h` | Overloaded functions for each frame type |

## Usage

### Parsing Frames

```cpp
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>

// Parse a frame (one-time header parsing)
auto frame = parseFrame(std::move(buffer));

// Access cached fields (no parsing, O(1))
uint32_t streamId = frame.streamId();
FrameType type = frame.type();
bool hasMetadata = frame.flags().metadata();

// Access frame-specific fields via views (lazy parsing)
if (type == FrameType::REQUEST_STREAM) {
  RequestStreamView view(frame);
  uint32_t initialN = view.initialRequestN();  // Parsed on-demand
}

// Access payload
auto [meta, data] = frame.payload();  // Returns cursors into buffer
```

### Writing Frames

```cpp
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

// Create frame with typed header + payload (move semantics)
auto frame = serialize(
    RequestStreamHeader{
        .streamId = 42,
        .initialRequestN = 100,
        .follows = false
    },
    std::move(metadata),  // Ownership transferred
    std::move(data)       // Ownership transferred
);

// Header-only frames
auto requestN = serialize(RequestNHeader{.streamId = 42, .requestN = 50});
auto cancel = serialize(CancelHeader{.streamId = 42});
```

### Pipeline Integration

```cpp
// ParsedFrame fits in TypeErasedBox (32-byte inline storage)
TypeErasedBox box = TypeErasedBox::create<ParsedFrame>(parseFrame(buffer));

// Handler can route without full parsing
void handleRead(Context& ctx, TypeErasedBox msg) {
  auto& frame = msg.as<ParsedFrame>();

  // Fast routing based on cached fields
  switch (frame.type()) {
    case FrameType::REQUEST_RESPONSE:
      requestHandler_.handle(frame);
      break;
    case FrameType::PAYLOAD:
      payloadHandler_.handle(frame);
      break;
    // ...
  }
}
```

## Frame Types Supported

| Frame Type | Header Struct | View Type | Has Payload |
|------------|---------------|-----------|-------------|
| `SETUP` | `SetupHeader` | `SetupView` | ✓ |
| `REQUEST_RESPONSE` | `RequestResponseHeader` | - | ✓ |
| `REQUEST_FNF` | `RequestFnfHeader` | - | ✓ |
| `REQUEST_STREAM` | `RequestStreamHeader` | `RequestStreamView` | ✓ |
| `REQUEST_CHANNEL` | `RequestChannelHeader` | `RequestChannelView` | ✓ |
| `REQUEST_N` | `RequestNHeader` | `RequestNView` | ✗ |
| `CANCEL` | `CancelHeader` | - | ✗ |
| `PAYLOAD` | `PayloadHeader` | - | ✓ |
| `ERROR` | `ErrorHeader` | `ErrorView` | ✓ |
| `KEEPALIVE` | `KeepAliveHeader` | `KeepAliveView` | ✓ |
| `METADATA_PUSH` | `MetadataPushHeader` | - | ✓ (metadata only) |
| `EXT` | `ExtHeader` | `ExtView` | ✓ |

## Design Decisions

### Why 32-byte FrameMetadata?

`TypeErasedBox` in Channel Pipeline has 32 bytes of inline storage. By keeping
`FrameMetadata` at exactly 32 bytes, frames flow through the pipeline without
heap allocation:

```cpp
struct FrameMetadata {
  const FrameDescriptor* descriptor;  // 8 bytes - flyweight pointer
  uint32_t streamId;                  // 4 bytes - cached
  Flags flags;                        // 2 bytes - cached
  uint16_t metadataSize;              // 2 bytes - cached
  uint32_t payloadOffset;             // 4 bytes - cached
  uint32_t payloadSize;               // 4 bytes - cached
  uint64_t reserved_;                 // 8 bytes - alignment/future use
};  // Total: 32 bytes
```

### Why Flyweight Pattern for FrameDescriptor?

Each frame type has immutable metadata (name, header size, capabilities). Rather
than storing this per-frame, we use a single shared `FrameDescriptor` per type:

```cpp
// O(1) array lookup, no allocation
const FrameDescriptor& desc = getDescriptor(FrameType::REQUEST_STREAM);
// desc.name == "REQUEST_STREAM"
// desc.headerSize == 10
// desc.hasPayload == true
```

### Why Lazy Views?

Frame-specific fields (e.g., `initialRequestN`, `errorCode`) are often not needed:
- Routing only needs `streamId` and `type`
- Error handling may skip successful frames entirely

Views parse these fields on-demand from the buffer, avoiding unnecessary work.

### Why Separate read/ and write/ Directories?

Clear separation of concerns between incoming and outgoing frame handling:
- **Protocol constants** (sizes, bit positions) are shared in `FrameType.h`
- **Reading code** has its own I/O helpers and parsing logic
- **Writing code** has its own serialization helpers
- Dependencies are explicit via BUCK targets

### Why No Frame Length Prefix?

Our format omits the 3-byte RSocket frame length prefix because:
1. `FrameLengthParser` (separate layer) handles length-prefixed framing
2. Pipeline handlers receive frames with length already stripped
3. Cleaner separation: frame layer vs frame parsing layer

## Performance

Benchmarks vs existing Rocket framing (opt-clang-lto):

### Parsing

| Frame Type | Rocket | FastThrift | Improvement |
|------------|--------|------------|-------------|
| REQUEST_RESPONSE | 77ns | 53ns | **+45%** |
| PAYLOAD (medium) | 103ns | 56ns | **+83%** |
| REQUEST_N | 18ns | 25ns | -27% |

### Writing

| Frame Type | Rocket | FastThrift | Improvement |
|------------|--------|------------|-------------|
| REQUEST_RESPONSE | 137ns | 113ns | **+21%** |
| PAYLOAD (medium) | 176ns | 128ns | **+38%** |
| REQUEST_N | 50ns | 44ns | **+14%** |

Run benchmarks:

```bash
buck2 run @//mode/opt-clang-lto fbcode//thrift/lib/cpp2/fast_thrift/frame/bench:frame_parsing_bench
buck2 run @//mode/opt-clang-lto fbcode//thrift/lib/cpp2/fast_thrift/frame/bench:frame_writing_bench
```

## Building

```bash
# Build all frame libraries
buck2 build fbcode//thrift/lib/cpp2/fast_thrift/frame:...

# Build just reading libraries
buck2 build fbcode//thrift/lib/cpp2/fast_thrift/frame/read:...

# Build just writing libraries
buck2 build fbcode//thrift/lib/cpp2/fast_thrift/frame/write:...

# Run all tests
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/frame/...

# Run reading tests
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/frame/read/test:...

# Run writing tests
buck2 test fbcode//thrift/lib/cpp2/fast_thrift/frame/write/test:...
```

## See Also

- [Channel Pipeline README](../../fast_thrift/channel_pipeline/README.md) - Pipeline architecture
- [FrameLengthParser](../frame/read/FrameParser.h) - Length-prefixed frame extraction
- [RSocket Protocol Spec](https://rsocket.io/about/protocol) - Wire format reference
