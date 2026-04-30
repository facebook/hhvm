# Thrift BiDi Streaming in Go

This document explains how to use Thrift BiDi (bidirectional) streaming in Go,
both from the client side and server side.

For general information about Thrift Streaming concepts and how to define
streaming services in Thrift IDL, please refer to the
[Thrift Streaming Wiki](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/).

## Table of Contents

- [Overview](#overview)
- [Defining BiDi Services](#defining-bidi-services)
- [Client-Side Usage](#client-side-usage)
- [Server-Side Implementation](#server-side-implementation)

## Overview

Thrift BiDi streaming allows the client and server to stream data to each other
simultaneously (bidirectional streaming). The client produces a stream of sink
elements that the server consumes, while the server concurrently produces a
stream of elements that the client consumes. Go's implementation uses
`iter.Seq2` iterators and channels to provide an idiomatic API for
bidirectional streaming.

**Key differences from Streaming and Sink:**

| Feature           | Streaming             | Sink                  | BiDi                  |
| ----------------- | --------------------- | --------------------- | --------------------- |
| Data direction    | Server → Client       | Client → Server       | Both directions simultaneously |
| Client produces   | Nothing               | Sink elements         | Sink elements         |
| Server produces   | Stream elements       | Final response        | Stream elements       |
| Final response    | None                  | Server sends after consuming all elements | None |

## Defining BiDi Services

Define BiDi methods in your Thrift IDL file using the `sink<SinkType>, stream<StreamType>`
return type:

```thrift
// BiDi-only (no initial response)
service ChatService {
  sink<i32>, stream<i16> simple();
}

// Response and BiDi (initial response followed by bidirectional streaming)
service SessionService {
  string /* session id */, sink<i32>, stream<i16> startSession();
}

// BiDi with declared exceptions on sink and stream elements
exception SinkException {
  1: string message;
}

exception StreamException {
  1: string message;
}

service ProcessingService {
  sink<i64 throws (1: SinkException ex)>, stream<
    i64 throws (1: StreamException ex)
  > process();
}
```

## Client-Side Usage

### API Signature

The generated client API returns a sink callback function and a stream
iterator. The sink callback accepts an `iter.Seq2[SinkElemType, error]`
iterator (the sink producer) and sends elements to the server. The stream
iterator is an `iter.Seq2[StreamElemType, error]` that yields elements from
the server. The values returned depend on whether the BiDi method has an
initial response:

**BiDi-only (no initial response):**

```go
func (c *Client) BiDiMethod(ctx context.Context, args...) (
    func(iter.Seq2[SinkElemType, error]), // Sink callback
    iter.Seq2[StreamElemType, error],     // Stream iterator
    error,                                // Initial error
)
```

**Response and BiDi (with initial response):**

```go
func (c *Client) BiDiMethod(ctx context.Context, args...) (
    *InitialResponse,                     // First response
    func(iter.Seq2[SinkElemType, error]), // Sink callback
    iter.Seq2[StreamElemType, error],     // Stream iterator
    error,                                // Initial error
)
```

### Usage Guidelines

1. **Context is Required**: The API REQUIRES a context with a timeout, deadline,
   or manual cancel to ensure background goroutines are terminated (avoid
   leaks).

2. **Check Initial Error**: Always check the initial error first. If non-nil, no
   BiDi session follows.

3. **Create a Sink Producer Function**: Define an `iter.Seq2[SinkElemType, error]`
   function that yields elements to send to the server.

4. **Call the Sink Callback**: Pass your producer function to the sink callback
   to start streaming elements to the server. The sink callback runs
   asynchronously in a background goroutine.

5. **Iterate Over Stream**: Use Go's `for elem, err := range streamSeq` syntax
   to consume stream elements from the server concurrently.

6. **Cleanup**: You should NOT worry about cleaning up resources besides
   providing a reasonable context timeout/cancellation.

### Example: BiDi-Only

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    sinkCallback, streamSeq, err := client.Simple(ctx)
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    // Define the sink producer function that yields elements to the server
    sinkSeq := func(yield func(int32, error) bool) {
        for i := int32(1); i <= 10; i++ {
            if !yield(i, nil) {
                return // Stop if consumer signals to stop
            }
        }
    }

    // Start sending sink elements (runs asynchronously)
    sinkCallback(sinkSeq)

    // Concurrently consume stream elements from the server
    for elem, err := range streamSeq {
        if err != nil {
            log.Fatalf("error during stream: %v", err)
        }
        fmt.Printf("Received from server: %d\n", elem)
    }
}
```

### Example: Response and BiDi

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    sessionID, sinkCallback, streamSeq, err := client.StartSession(ctx)
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    fmt.Printf("Session ID: %s\n", sessionID)

    // Start sending sink elements
    sinkSeq := func(yield func(int32, error) bool) {
        for i := int32(1); i <= 5; i++ {
            if !yield(i, nil) {
                return
            }
        }
    }
    sinkCallback(sinkSeq)

    // Consume stream elements from the server
    for elem, err := range streamSeq {
        if err != nil {
            log.Fatalf("error during stream: %v", err)
        }
        fmt.Printf("Received: %d\n", elem)
    }
}
```

### Example: Handling Errors in Sink Producer

```go
sinkSeq := func(yield func(int64, error) bool) {
    for _, item := range items {
        data, err := processItem(item)
        if err != nil {
            // Signal error to server - this will be received
            // as the error in the server's iter.Seq2 loop
            yield(0, err)
            return
        }
        if !yield(data, nil) {
            return
        }
    }
}
```

## Server-Side Implementation

### API Signature

Server-side BiDi methods return a sink consumer function and a stream producer
function:

**BiDi-only (no initial response):**

```go
func (h *Handler) BiDiMethod(ctx context.Context, args...) (
    func(context.Context, iter.Seq2[SinkElemType, error]) error, // Sink consumer function
    func(context.Context, chan<- StreamElemType) error,           // Stream producer function
    error,                                                       // Initial error
)
```

**Response and BiDi (with initial response):**

```go
func (h *Handler) BiDiMethod(ctx context.Context, args...) (
    *InitialResponse,                                            // First response
    func(context.Context, iter.Seq2[SinkElemType, error]) error, // Sink consumer function
    func(context.Context, chan<- StreamElemType) error,           // Stream producer function
    error,                                                       // Initial error
)
```

### Implementation Guidelines

1. **Return Initial Response/Error First**: Return the initial response and/or
   error before BiDi processing begins.

2. **Sink Consumer Function**: Return a function that consumes sink elements
   from the client via an `iter.Seq2` iterator. Use Go's
   `for elem, err := range seq` syntax.

3. **Stream Producer Function**: Return a function that produces stream elements
   to send to the client via a `chan<-` channel.

4. **Concurrent Execution**: The Thrift library runs the sink consumer and
   stream producer **concurrently** in separate goroutines. Design your handler
   accordingly.

5. **Use Context for Cancellation**: The context passed to both functions should
   be used to detect stream interruption (e.g. client disconnected).

6. **Handle Client Errors**: Check the error value in each sink iteration. If
   non-nil, the client encountered an error and stopped sending.

7. **Return Error for Stream Errors**: Return an error from the stream producer
   function to signal a streaming error to the client.

8. **Cleanup Resources**: Both functions should clean up all resources before
   returning.

### Example: BiDi-Only

```go
type ChatService struct{}

func (s *ChatService) Simple(ctx context.Context) (
    func(context.Context, iter.Seq2[int32, error]) error,
    func(context.Context, chan<- int16) error,
    error,
) {
    // Sink consumer: receives elements from the client
    sinkConsumerFunc := func(ctx context.Context, seq iter.Seq2[int32, error]) error {
        for elem, err := range seq {
            if err != nil {
                return err
            }
            select {
            case <-ctx.Done():
                return ctx.Err()
            default:
            }
            // Process element from client
            fmt.Printf("Received from client: %d\n", elem)
        }
        return nil
    }

    // Stream producer: sends elements to the client
    streamProducerFunc := func(ctx context.Context, elemChan chan<- int16) error {
        for i := int16(1); i <= 10; i++ {
            select {
            case <-ctx.Done():
                return ctx.Err()
            case elemChan <- i:
                // Element sent successfully
            }
        }
        return nil // Successful completion
    }

    return sinkConsumerFunc, streamProducerFunc, nil
}
```

### Example: Response and BiDi

```go
type SessionService struct{}

func (h *SessionService) StartSession(ctx context.Context) (
    string,
    func(context.Context, iter.Seq2[int32, error]) error,
    func(context.Context, chan<- int16) error,
    error,
) {
    sessionID := "session-123"

    sinkConsumerFunc := func(ctx context.Context, seq iter.Seq2[int32, error]) error {
        for elem, err := range seq {
            if err != nil {
                return err
            }
            select {
            case <-ctx.Done():
                return ctx.Err()
            default:
            }
            processClientData(sessionID, elem)
        }
        return nil
    }

    streamProducerFunc := func(ctx context.Context, elemChan chan<- int16) error {
        for i := int16(1); i <= 5; i++ {
            select {
            case <-ctx.Done():
                return ctx.Err()
            case elemChan <- i:
            }
        }
        return nil
    }

    return sessionID, sinkConsumerFunc, streamProducerFunc, nil
}
```

### Example: Handling Initial Exception

```go
func (h *Handler) Process(ctx context.Context) (
    func(context.Context, iter.Seq2[int64, error]) error,
    func(context.Context, chan<- int64) error,
    error,
) {
    if !h.isReady() {
        // Return initial error - BiDi session will not proceed
        return nil, nil, NewBiDiMethodException().SetMessage("service not ready")
    }

    sinkConsumerFunc := func(ctx context.Context, seq iter.Seq2[int64, error]) error {
        for data, err := range seq {
            if err != nil {
                return err
            }
            process(data)
        }
        return nil
    }

    streamProducerFunc := func(ctx context.Context, elemChan chan<- int64) error {
        for result := range results() {
            select {
            case <-ctx.Done():
                return ctx.Err()
            case elemChan <- result:
            }
        }
        return nil
    }

    return sinkConsumerFunc, streamProducerFunc, nil
}
```

## Additional Resources

- [General Thrift Streaming Documentation](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/)
- [Thrift Streaming in Go](streaming.md)
- [Thrift Sink in Go](sink.md)
