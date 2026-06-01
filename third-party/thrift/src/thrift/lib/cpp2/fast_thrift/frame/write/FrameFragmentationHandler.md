# FrameFragmentationHandler Design

A composable, injectable pipeline handler that mitigates Head-of-Line (HOL)
blocking by fragmenting large RSocket frames and interleaving them across
streams using round-robin scheduling.

## Quick Reference

```
Handler Type:     OutboundHandler (channel_pipeline)
Input:            Frames from upstream handlers
Output:           Fragments to downstream handlers (via fireWrite)
Composable:       Yes - just add to pipeline, no special integration
Thread Model:     Single-threaded EventBase
```

---

## The Problem

In a single-threaded EventBase, even fragmented payloads block other streams:

```
WITHOUT this handler (HOL blocked):

Timeline ──────────────────────────────────────────────────────────────►

Stream A (8MB):  [████████████████████████████████████████████████████]
Stream B (1KB):                                                         [█]
Stream C (2KB):                                                           [██]

Stream B waits for ALL of Stream A to complete.
```

## The Solution

Batch frames during EventBase tick, round-robin at flush:

```
WITH this handler (HOL mitigated):

Timeline ──────────────────────────────────────────────────────────────►

Stream B (1KB):  [█]                          (small - sent first)
Stream A (8MB):     [A1][A2]    [A3][A4]    [A5][A6]...
Stream C (2KB):         [C1][C2]    [C3]

Streams interleave - small payloads get through quickly.
```

---

## Handler Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                    FrameFragmentationHandler                        │
│                                                                     │
│  Implements: OutboundHandler concept                                │
│  Extends:    folly::EventBase::LoopCallback                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────┐    ┌─────────────────────────────────────┐    │
│  │  Configuration  │    │            State                    │    │
│  ├─────────────────┤    ├─────────────────────────────────────┤    │
│  │ maxFragmentSize │    │ isScheduled_      : bool            │    │
│  │ maxPendingBytes │    │ pendingBytes_     : size_t          │    │
│  │ maxPendingFrames│    │ pendingFrames_    : size_t          │    │
│  └─────────────────┘    │ eventBase_        : EventBase*      │    │
│                         └─────────────────────────────────────┘    │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     Data Structures                          │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │                                                               │   │
│  │  immediateQueue_: std::deque<Frame>                          │   │
│  │  ┌─────────────────────────────────────────────────────────┐ │   │
│  │  │ [Frame B: 1KB] [Frame D: 500B] [Frame E: 2KB] ...      │ │   │
│  │  └─────────────────────────────────────────────────────────┘ │   │
│  │  Small frames (≤64KB) - sent first, no fragmentation        │   │
│  │                                                               │   │
│  │  streams_: F14NodeMap<uint32_t, PerStreamState>              │   │
│  │  ┌─────────────────────────────────────────────────────────┐ │   │
│  │  │ StreamId │ PerStreamState                               │ │   │
│  │  │──────────┼─────────────────────────────────────────────│ │   │
│  │  │    5     │ payload: [████████], offset: 0, total: 200KB │ │   │
│  │  │    7     │ payload: [██████], offset: 64KB, total: 150KB│ │   │
│  │  │    9     │ payload: [████], offset: 128KB, total: 180KB │ │   │
│  │  └─────────────────────────────────────────────────────────┘ │   │
│  │  Large frames (>64KB) - lazy fragmentation during flush     │   │
│  │                                                               │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Core Algorithm

### 1. onWrite() - Frame Arrival

```
onWrite(frame)
    │
    ▼
┌─────────────────────┐
│ Get frame size      │
└─────────────────────┘
    │
    ▼
┌─────────────────────┐     YES     ┌─────────────────────┐
│ size ≤ 64KB?        │────────────►│ Add to              │
└─────────────────────┘             │ immediateQueue_     │
    │ NO                            └─────────────────────┘
    ▼                                         │
┌─────────────────────┐                       │
│ Create PerStreamState                       │
│ Add to streams_ map │                       │
└─────────────────────┘                       │
    │                                         │
    ▼◄────────────────────────────────────────┘
┌─────────────────────┐
│ pendingBytes_ +=    │
│ pendingFrames_++    │
└─────────────────────┘
    │
    ▼
┌─────────────────────┐     YES     ┌─────────────────────┐
│ Over threshold?     │────────────►│ flushNow()          │
│ (bytes OR frames)   │             │ (forced mid-tick)   │
└─────────────────────┘             └─────────────────────┘
    │ NO                                      │
    ▼                                         │
┌─────────────────────┐                       │
│ !isScheduled_?      │                       │
└─────────────────────┘                       │
    │ YES                                     │
    ▼                                         │
┌─────────────────────┐                       │
│ runInLoop(this)     │                       │
│ isScheduled_ = true │                       │
└─────────────────────┘                       │
    │                                         │
    ▼◄────────────────────────────────────────┘
┌─────────────────────┐
│ return Success      │
└─────────────────────┘
```

### 2. runLoopCallback() - Flush at Tick End

```
runLoopCallback()
    │
    ▼
┌─────────────────────────────────────────────────────────────┐
│ STEP 1: Drain immediateQueue_ (small frames first)          │
│                                                              │
│   while (!immediateQueue_.empty()):                         │
│       frame = immediateQueue_.front()                       │
│       result = ctx.fireWrite(frame)                         │
│       if (result == Backpressure):                          │
│           awaitWriteReady()                                 │
│           return  ◄─── PAUSE                                │
│       immediateQueue_.pop_front()                           │
└─────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────┐
│ STEP 2: Round-robin through streams_ (large frame fragments)│
│                                                              │
│   while (streams_ not empty):                               │
│       for each (streamId, state) in streams_:               │
│           if (!state.hasMore()):                            │
│               erase(streamId)                               │
│               continue                                       │
│                                                              │
│           fragment = state.nextFragment(64KB)               │
│           setFollowsFlag(fragment, state.hasMore())         │
│           result = ctx.fireWrite(fragment)                  │
│                                                              │
│           if (result == Backpressure):                      │
│               saveIteratorPosition()                        │
│               awaitWriteReady()                             │
│               return  ◄─── PAUSE                            │
└─────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────┐
│ STEP 3: Reset state                                          │
│                                                              │
│   pendingBytes_ = 0                                         │
│   pendingFrames_ = 0                                        │
│   isScheduled_ = false                                      │
└─────────────────────────────────────────────────────────────┘
```

### 3. Round-Robin Detail

```
Initial State:
┌────────────────────────────────────────────────────────────┐
│ Stream 5: [████████████] 200KB  (4 fragments)              │
│ Stream 7: [████████]     128KB  (2 fragments)              │
│ Stream 9: [██████████]   160KB  (3 fragments)              │
└────────────────────────────────────────────────────────────┘

Round 1:
  Send S5:frag1 ──► [████........] offset=64KB
  Send S7:frag1 ──► [████.......]  offset=64KB
  Send S9:frag1 ──► [████......]   offset=64KB

Round 2:
  Send S5:frag2 ──► [████████....] offset=128KB
  Send S7:frag2 ──► [████████]     offset=128KB ✓ DONE, erase
  Send S9:frag2 ──► [████████..]   offset=128KB

Round 3:
  Send S5:frag3 ──► [████████████] offset=192KB
  (S7 gone)
  Send S9:frag3 ──► [██████████]   offset=160KB ✓ DONE, erase

Round 4:
  Send S5:frag4 ──► [████████████] offset=200KB ✓ DONE, erase
  (S7 gone)
  (S9 gone)

All streams drained.

Wire order: [S5:f1][S7:f1][S9:f1][S5:f2][S7:f2][S9:f2][S5:f3][S9:f3][S5:f4]
            ─────────────────────────────────────────────────────────────►
            Interleaved!
```

---

## Backpressure Handling

```
Normal Flow:

  onWrite ──► Queue ──► LoopCallback ──► fireWrite ──► Success
                                              │
                                              ▼
                                         Next fragment


Backpressure Flow:

  onWrite ──► Queue ──► LoopCallback ──► fireWrite ──► Backpressure!
                                              │
                              ┌───────────────┘
                              ▼
                    ┌─────────────────────┐
                    │ Save iterator pos   │
                    │ awaitWriteReady()   │
                    │ return (PAUSE)      │
                    └─────────────────────┘

                        ... time passes ...
                        (TCP buffer drains)

                    ┌─────────────────────┐
                    │ onWriteReady()      │◄─── Transport ready
                    └─────────────────────┘
                              │
                              ▼
                    ┌─────────────────────┐
                    │ cancelAwaitWriteReady()
                    │ Resume from saved   │
                    │ iterator position   │
                    └─────────────────────┘
                              │
                              ▼
                         Continue round-robin
```

---

## LoopCallback Self-Scheduling

```
State Diagram:

                 onWrite()
                 (first frame)
    ┌──────────────────────────────────┐
    │                                  ▼
    │                         ┌───────────────┐
    │   ┌─────────────────────│  SCHEDULED    │◄────────┐
    │   │                     └───────────────┘         │
    │   │                            │                  │
    │   │ runLoopCallback()          │ onWrite()       │
    │   │ (all drained)              │ (more frames)   │
    │   │                            │                  │
    │   ▼                            └──────────────────┘
┌───────────────┐
│     IDLE      │     isScheduled_ = false
└───────────────┘     (no callback registered)
        ▲
        │
        │ runLoopCallback()
        │ (all drained)
        │
┌───────────────┐
│   FLUSHING    │────────────────────────────┐
└───────────────┘                            │
        ▲                                    │
        │ onWriteReady()                     │ fireWrite()
        │                                    │ returns Backpressure
        │                                    ▼
        │                          ┌───────────────┐
        └──────────────────────────│ BACKPRESSURED │
                                   └───────────────┘
```

Key invariant: Only ONE LoopCallback scheduled at a time.

```
onWrite():
  if (!isScheduled_) {
    eventBase_->runInLoop(this);
    isScheduled_ = true;
  }
  // else: already scheduled, just queue the frame
```

---

## Configuration

```
┌─────────────────────────────────────────────────────────────────┐
│                    FragmentationConfig                          │
├────────────────────┬──────────────┬────────────────────────────┤
│ Parameter          │ Default      │ Purpose                    │
├────────────────────┼──────────────┼────────────────────────────┤
│ maxFragmentSize    │ 64 KB        │ Fragment size for large    │
│                    │              │ payloads; bypass threshold │
├────────────────────┼──────────────┼────────────────────────────┤
│ maxPendingBytes    │ 512 KB       │ Force flush if exceeded    │
│                    │              │ (memory pressure limit)    │
├────────────────────┼──────────────┼────────────────────────────┤
│ maxPendingFrames   │ 128          │ Force flush if exceeded    │
│                    │              │ (backstop for tiny frames) │
├────────────────────┼──────────────┼────────────────────────────┤
│ minSizeToFragment  │ 1 KB         │ Don't fragment payloads    │
│                    │              │ smaller than this          │
└────────────────────┴──────────────┴────────────────────────────┘

Tuning:
  High latency network  → Larger maxFragmentSize (128KB+)
  Low latency needs     → Smaller maxFragmentSize (32KB)
  Memory constrained    → Lower maxPendingBytes (256KB)
  Bulk transfers        → Higher maxPendingBytes (1MB+)
```

---

## Pipeline Integration

This handler is **composable** - just inject it into the pipeline:

```
┌─────────────────────────────────────────────────────────────────┐
│                        PIPELINE                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Application                                                    │
│       │                                                         │
│       │ write(payload)                                          │
│       ▼                                                         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              [Any Upstream Handlers]                     │   │
│  └─────────────────────────────────────────────────────────┘   │
│       │                                                         │
│       │ onWrite(frame)                                          │
│       ▼                                                         │
│  ╔═════════════════════════════════════════════════════════╗   │
│  ║         FrameFragmentationHandler  ◄── INJECT HERE      ║   │
│  ╚═════════════════════════════════════════════════════════╝   │
│       │                                                         │
│       │ fireWrite(fragment)                                     │
│       ▼                                                         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                     FrameWriter                          │   │
│  │            (serializes to wire format)                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│       │                                                         │
│       │ fireWrite(IOBuf)                                        │
│       ▼                                                         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                   TransportAdapter                       │   │
│  │                  (TCP socket write)                      │   │
│  └─────────────────────────────────────────────────────────┘   │
│       │                                                         │
│       ▼                                                         │
│  Network                                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

Usage:
```cpp
auto pipeline = PipelineBuilder()
    .addHandler<FrameFragmentationHandler>(config)  // Just add it
    .addHandler<FrameWriter>()
    .addHandler<TransportAdapter>(socket)
    .build();
```

No fragmentation needed? Don't add it. The pipeline works either way.

---

## PerStreamState

Tracks lazy fragmentation state for a single stream:

```
┌─────────────────────────────────────────────────────────────────┐
│                       PerStreamState                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  streamId:        uint32_t          Stream identifier           │
│  originalPayload: unique_ptr<IOBuf> Original data               │
│  currentOffset:   size_t            Bytes already sent          │
│  totalBytes:      size_t            Total payload size          │
│  frameType:       FrameType         REQUEST_*, PAYLOAD, etc.    │
│  originalFlags:   uint16_t          Flags from original frame   │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  hasMore():                                                     │
│      return currentOffset < totalBytes                          │
│                                                                 │
│  nextFragment(maxSize):                                         │
│      remaining = totalBytes - currentOffset                     │
│      chunkSize = min(maxSize, remaining)                        │
│      fragment = slice(currentOffset, chunkSize)  // zero-copy   │
│      currentOffset += chunkSize                                 │
│      return fragment                                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

Lazy fragmentation: Don't pre-split into N IOBufs.
Just track offset, slice on demand during round-robin.
```

---

## FOLLOWS Flag

RSocket protocol uses the FOLLOWS flag for fragmentation:

```
Fragment 1:  [Header: FOLLOWS=1] [64KB data...]   ──► More coming
Fragment 2:  [Header: FOLLOWS=1] [64KB data...]   ──► More coming
Fragment 3:  [Header: FOLLOWS=1] [64KB data...]   ──► More coming
Fragment 4:  [Header: FOLLOWS=0] [8KB data...]    ──► Final fragment

Receiver reassembles by buffering until FOLLOWS=0.
```

This handler sets FOLLOWS based on `state.hasMore()`:
- `hasMore() == true`  → FOLLOWS=1
- `hasMore() == false` → FOLLOWS=0 (final fragment)

---

## Key Guarantees

```
┌────────────────────────────────────────────────────────────────┐
│ 1. Per-stream ordering preserved                               │
│    Fragments for stream A always sent in order: A1, A2, A3...  │
│    Never out of order within a stream.                         │
├────────────────────────────────────────────────────────────────┤
│ 2. Small frames prioritized                                    │
│    immediateQueue_ drained before round-robin starts.          │
│    Latency-sensitive small payloads go first.                  │
├────────────────────────────────────────────────────────────────┤
│ 3. Memory bounded                                              │
│    maxPendingBytes triggers forced flush.                      │
│    Cannot accumulate unbounded data.                           │
├────────────────────────────────────────────────────────────────┤
│ 4. Backpressure respected                                      │
│    Pauses on Backpressure, resumes on WriteReady.              │
│    No data loss.                                                │
├────────────────────────────────────────────────────────────────┤
│ 5. Composable                                                  │
│    Standard OutboundHandler. No special integration.           │
│    Add to pipeline, remove from pipeline - just works.         │
└────────────────────────────────────────────────────────────────┘
```

---

## Summary

```
┌─────────────────────────────────────────────────────────────────┐
│                    FrameFragmentationHandler                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  WHAT:    Composable outbound handler for HOL mitigation        │
│                                                                 │
│  HOW:     Batch → Fragment → Round-Robin → Interleave           │
│                                                                 │
│  WHEN:    LoopCallback at end of EventBase tick                 │
│                                                                 │
│  WHERE:   Inject into pipeline before FrameWriter               │
│                                                                 │
│  WHY:     Prevents large payloads from blocking small ones      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```
