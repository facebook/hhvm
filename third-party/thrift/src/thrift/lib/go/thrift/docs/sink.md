# Thrift Sink in Go

This document explains how to use Thrift Sink in Go, both from the client side
and server side.

For general information about Thrift Sink concepts and how to define sink
services in Thrift IDL, please refer to the
[Thrift Streaming Wiki](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/).

## Table of Contents

- [Overview](#overview)
- [Defining Sink Services](#defining-sink-services)
- [Client-Side Usage](#client-side-usage)
- [Server-Side Implementation](#server-side-implementation)

## Overview

Thrift Sink allows the client to stream data to the server (client-to-server
streaming), enabling efficient handling of large uploads or continuous data
feeds. The client produces a stream of elements that the server consumes, and
the server returns a final response after processing all elements. Go's
implementation uses `iter.Seq2` iterators to provide an idiomatic API for sink
functionality.

**Key differences from Streaming:**

| Feature           | Streaming             | Sink                  |
| ----------------- | --------------------- | --------------------- |
| Data direction    | Server → Client       | Client → Server       |
| Producer          | Server                | Client                |
| Consumer          | Client                | Server                |
| Final response    | None                  | Server sends after consuming all elements |

## Defining Sink Services

Define sink methods in your Thrift IDL file using the `sink<ElemType, FinalResponseType>`
return type:

```thrift
// Sink-only (no initial response)
service DataService {
  sink<i32, i32> SinkOnly();
}

// Response and sink (initial response followed by sink)
service FileService {
  i32 /* upload id */, sink<binary /* chunk */, i64 /* checksum */> UploadFileChunks(1: string fileName);
}

// Sink with declared exception on sink elements
service ProcessingService {
  sink<Data throws (1: ProcessingException ex), Result> ProcessData();
}

// Sink with declared exception on final response
service ValidationService {
  sink<Data, Result throws (1: ValidationException ex)> ValidateData();
}
```

## Client-Side Usage

### API Signature

The generated client API returns a callback function that accepts an `iter.Seq2[ElemType, error]`
iterator (the sink producer) and returns the final response. The values returned
depend on whether the sink has an initial response:

**Sink-only (no initial response):**

```go
func (c *Client) SinkMethod(ctx context.Context, args...) (
    func(iter.Seq2[ElemType, error]) (FinalResponse, error), // Sink callback
    error,                                                    // Initial error
)
```

**Response and sink (with initial response):**

```go
func (c *Client) SinkMethod(ctx context.Context, args...) (
    *InitialResponse,                                         // First response
    func(iter.Seq2[ElemType, error]) (FinalResponse, error),  // Sink callback
    error,                                                    // Initial error
)
```

### Usage Guidelines

1. **Context is Required**: The API REQUIRES a context with a timeout, deadline,
   or manual cancel to ensure background goroutines are terminated (avoid
   leaks).

2. **Check Initial Error**: Always check the initial error first. If non-nil, no
   sink follows.

3. **Create a Producer Function**: Define an `iter.Seq2[ElemType, error]`
   function that yields elements to send to the server.

4. **Call the Sink Callback**: Pass your producer function to the sink callback
   to start streaming elements to the server.

5. **Handle Final Response**: The sink callback returns the final response from
   the server after all elements have been consumed.

6. **Cleanup**: You should NOT worry about cleaning up resources besides
   providing a reasonable context timeout/cancellation.

### Example: Sink-Only

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    sinkCallback, err := client.SinkOnly(ctx)
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    // Define the producer function that yields elements
    sinkSeq := func(yield func(int32, error) bool) {
        for i := int32(1); i <= 5; i++ {
            if !yield(i, nil) {
                return // Stop if consumer signals to stop
            }
        }
    }

    // Call the sink callback with the producer
    finalResponse, err := sinkCallback(sinkSeq)
    if err != nil {
        log.Fatalf("sink failed: %v", err)
    }

    fmt.Printf("Final response: %d\n", finalResponse)
}
```

### Example: Response and Sink

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    uploadID, sinkCallback, err := client.UploadFileChunks(ctx, "myfile.txt")
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    fmt.Printf("Upload ID: %d\n", uploadID)

    // Open file and create producer
    file, err := os.Open("myfile.txt")
    if err != nil {
        log.Fatalf("failed to open file: %v", err)
    }
    defer file.Close()

    sinkSeq := func(yield func([]byte, error) bool) {
        buffer := make([]byte, 4096)
        for {
            n, err := file.Read(buffer)
            if err == io.EOF {
                return // Done reading
            }
            if err != nil {
                yield(nil, err) // Signal error to server
                return
            }
            if !yield(buffer[:n], nil) {
                return // Stop if consumer signals to stop
            }
        }
    }

    checksum, err := sinkCallback(sinkSeq)
    if err != nil {
        log.Fatalf("upload failed: %v", err)
    }

    fmt.Printf("Upload complete. Checksum: %d\n", checksum)
}
```

### Example: Handling Errors in Producer

```go
sinkSeq := func(yield func(Data, error) bool) {
    for _, item := range items {
        data, err := processItem(item)
        if err != nil {
            // Signal error to server - this will be received
            // as the error in the server's iter.Seq2 loop
            yield(Data{}, err)
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

Server-side sink methods have a different signature that returns a consumer
function:

**Sink-only (no initial response):**

```go
func (h *Handler) SinkMethod(ctx context.Context, args...) (
    func(context.Context, iter.Seq2[ElemType, error]) (FinalResponse, error), // Consumer function
    error, // Initial error
)
```

**Response and sink (with initial response):**

```go
func (h *Handler) SinkMethod(ctx context.Context, args...) (
    *InitialResponse,                                                          // First response
    func(context.Context, iter.Seq2[ElemType, error]) (FinalResponse, error),  // Consumer function
    error,                                                                     // Initial error
)
```

### Implementation Guidelines

1. **Return Initial Response/Error First**: Return the initial response and/or
   error before sink processing begins.

2. **Consumer Function**: Return a consumer function that will be invoked by the
   Thrift library to process sink elements.

3. **Use Context for Cancellation**: The context passed to the consumer function
   should be used to detect stream interruption (e.g. client disconnected).

4. **Iterate Over Elements**: Use Go's `for elem, err := range seq` syntax to
   consume elements from the client.

5. **Handle Client Errors**: Check the error value in each iteration. If non-nil,
   the client encountered an error and stopped sending.

6. **Return Final Response**: After consuming all elements, return the final
   response to the client.

7. **Return Error for Sink Errors**: Return an error from the consumer function
   to signal a final response error to the client.

8. **Cleanup Resources**: The consumer function should clean up all resources
   before returning.

### Example: Sink-Only

```go
type DataService struct{}

func (s *DataService) SinkOnly(ctx context.Context) (func(context.Context, iter.Seq2[int32, error]) (int32, error), error) {
    // Return consumer function
    elemConsumerFunc := func(ctx context.Context, seq iter.Seq2[int32, error]) (int32, error) {
        var sum int32
        for elem, err := range seq {
            // Check for client-side error
            if err != nil {
                return sum, err
            }
            // Check for cancellation
            select {
            case <-ctx.Done():
                return sum, ctx.Err()
            default:
            }
            // Process element
            sum += elem
        }
        return sum, nil // Return final response
    }

    return elemConsumerFunc, nil
}
```

### Example: Response and Sink

```go
type FileService struct{}

func (h *FileService) UploadFileChunks(ctx context.Context, fileName string) (
    int32,
    func(context.Context, iter.Seq2[[]byte, error]) (int64, error),
    error,
) {
    // Create upload and get ID for initial response
    uploadID, err := createUpload(fileName)
    if err != nil {
        return 0, nil, fmt.Errorf("failed to create upload: %w", err)
    }

    // Create consumer function
    elemConsumerFunc := func(ctx context.Context, seq iter.Seq2[[]byte, error]) (int64, error) {
        var checksum int64

        for chunk, err := range seq {
            // Check for client-side error
            if err != nil {
                cancelUpload(uploadID)
                return 0, err
            }

            // Check for cancellation
            select {
            case <-ctx.Done():
                cancelUpload(uploadID)
                return 0, ctx.Err()
            default:
            }

            // Process chunk
            if err := writeChunk(uploadID, chunk); err != nil {
                cancelUpload(uploadID)
                return 0, fmt.Errorf("failed to write chunk: %w", err)
            }
            checksum = computeChecksum(checksum, chunk)
        }

        // Finalize upload
        if err := finalizeUpload(uploadID); err != nil {
            return 0, fmt.Errorf("failed to finalize upload: %w", err)
        }

        return checksum, nil
    }

    return uploadID, elemConsumerFunc, nil
}
```

### Example: Returning a Final Response Exception

```go
func (h *Handler) ValidateData(ctx context.Context) (func(context.Context, iter.Seq2[*Data, error]) (*Result, error), error) {
    elemConsumerFunc := func(ctx context.Context, seq iter.Seq2[*Data, error]) (*Result, error) {
        for data, err := range seq {
            if err != nil {
                return nil, err
            }
            if !isValid(data) {
                // Return declared exception as final response error
                return nil, NewValidationException().SetMessage("invalid data")
            }
        }
        return &Result{Success: true}, nil
    }

    return elemConsumerFunc, nil
}
```

### Example: Handling Initial Exception

```go
func (h *Handler) ProcessData(ctx context.Context) (func(context.Context, iter.Seq2[*Data, error]) (*Result, error), error) {
    if !h.isReady() {
        // Return initial error - sink will not proceed
        return nil, NewServiceException().SetMessage("service not ready")
    }

    elemConsumerFunc := func(ctx context.Context, seq iter.Seq2[*Data, error]) (*Result, error) {
        // Process elements...
        for data, err := range seq {
            if err != nil {
                return nil, err
            }
            process(data)
        }
        return &Result{Success: true}, nil
    }

    return elemConsumerFunc, nil
}
```

## Additional Resources

- [General Thrift Streaming Documentation](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/)
- [Thrift Streaming in Go](streaming.md)
