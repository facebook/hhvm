# Thrift Streaming in Go

This document explains how to use Thrift Streaming in Go, both from the client side and server side.

For general information about Thrift Streaming concepts and how to define streaming services in Thrift IDL, please refer to the [Thrift Streaming Wiki](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/).

## Table of Contents

- [Overview](#overview)
- [Defining Streaming Services](#defining-streaming-services)
- [Client-Side Usage](#client-side-usage)
- [Server-Side Implementation](#server-side-implementation)

## Overview

Thrift Streaming allows the server to stream data to the client (server-to-client streaming), enabling efficient handling of large datasets or real-time data feeds. The server produces a stream of elements that the client consumes. Go's implementation uses channels to provide an idiomatic API for streaming functionality.

## Defining Streaming Services

Define streaming methods in your Thrift IDL file using the `stream<T>` return type:

```thrift
// Stream-only response (no initial response)
service NumberService {
  stream<i32> numberRange(1: i32 from_val, 2: i32 to_val);
}

// Response and stream (initial response followed by stream)
struct GetFileResult {
  1: i64 fileSize;
}

struct FileChunk {
  1: binary data;
}

service FileService {
  GetFileResult, stream<FileChunk> getFile(1: string filePath);
}
```

## Client-Side Usage

### API Signature

The generated client API returns different values depending on whether the stream has an initial response:

**Stream-only (no initial response):**
```go
func (c *Client) StreamMethod(ctx context.Context, args...) (
    <-chan ElemType,  // Element channel
    <-chan error,     // Error channel
    error,            // Initial error
)
```

**Response and stream (with initial response):**
```go
func (c *Client) StreamMethod(ctx context.Context, args...) (
    *InitialResponse, // First response
    <-chan ElemType,  // Element channel
    <-chan error,     // Error channel
    error,            // Initial error
)
```

### Usage Guidelines

1. **Context is Required**: The API REQUIRES a context with a timeout, deadline, or manual cancel to ensure background goroutines are terminated (avoid leaks).

2. **Check Initial Error**: Always check the initial error first. If non-nil, no stream follows.

3. **Consume Element Channel**: Consume elements from the element channel until it's exhausted (closed).

4. **Check Stream Error**: After consuming all elements, check the error channel once to determine if the stream terminated gracefully or encountered an error.

5. **Cleanup**: You should NOT worry about cleaning up channels/resources besides providing a reasonable context timeout/cancellation.

### Example: Stream-Only Response

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    elemChan, errChan, err := client.NumberRange(ctx, 1, 100)
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    for elem := range elemChan {
        // Process each element
        fmt.Printf("Received: %d\n", elem)
    }

    // Check if streaming encountered an error
    streamErr := <-errChan
    if streamErr != nil {
        log.Fatalf("error during stream: %v", streamErr)
    }
}
```

### Example: Response and Stream

```go
func main() {
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()

    firstResponse, elemChan, errChan, err := client.GetFile(ctx, "/path/to/file.txt")
    if err != nil {
        log.Fatalf("request failed: %v", err)
    }

    fmt.Printf("File size: %d bytes\n", firstResponse.GetFileSize())

    for fileChunk := range elemChan {
        // Process each file chunk
        fmt.Printf("Received chunk: %d bytes\n", len(fileChunk.GetData()))
    }

    // Check if streaming encountered an error
    streamErr := <-errChan
    if streamErr != nil {
        log.Fatalf("error during stream: %v", streamErr)
    }
}
```

## Server-Side Implementation

### API Signature

Server-side streaming methods have a different signature that returns a producer function:

**Stream-only (no initial response):**
```go
func (h *Handler) StreamMethod(ctx context.Context, args...) (
    func(context.Context, chan<- ElemType) error, // Producer function
    error, // Initial error
)
```

**Response and stream (with initial response):**
```go
func (h *Handler) StreamMethod(ctx context.Context, args...) (
    *InitialResponse,                              // First response
    func(context.Context, chan<- ElemType) error,  // Producer function
    error,                                         // Initial error
)
```

### Implementation Guidelines

1. **Return Initial Response/Error First**: Return the initial response and/or error before streaming begins.

2. **Producer Function**: Return a producer function that will be invoked by the Thrift library to generate stream elements.

3. **Use Context for Cancellation**: The context passed to the producer function should be used to detect stream interruption (e.g. client disconnected).

4. **Send Elements via Channel**: Use the provided channel to send stream elements.

5. **Return Error for Stream Errors**: Return an error from the producer function to signal a streaming error.

6. **Return nil for Successful Completion**: Return `nil` to indicate successful stream completion.

7. **Cleanup Resources**: The producer function should clean up all resources before returning.

### Example: Stream-Only Response

```go
type NumberService struct{}

func (s *NumberService) NumberRange(ctx context.Context, from int32, to int32) (func(context.Context, chan<- int32) error, error) {
    // Validate input
    if from > to {
        return nil, fmt.Errorf("invalid range: from (%d) > to (%d)", from, to)
    }

    // Return producer function
    producerFunc := func(ctx context.Context, elemChan chan<- int32) error {
        for i := from; i <= to; i++ {
            // Check for cancellation
            select {
            case <-ctx.Done():
                return ctx.Err()
            case elemChan <- i:
                // Element sent successfully
            }
        }
        return nil // Successful completion
    }

    return producerFunc, nil
}
```

### Example: Response and Stream

```go
type FileService struct{}

func (h *FileService) GetFile(ctx context.Context, filePath string) (
    *GetFileResult,
    func(context.Context, chan<- *FileChunk) error,
    error,
) {
    // Get file info for initial response
    fileInfo, err := os.Stat(filePath)
    if err != nil {
        return nil, nil, fmt.Errorf("error statting file: %w", err)
    }

    // Open file
    file, err := os.Open(filePath)
    if err != nil {
        return nil, nil, fmt.Errorf("error opening file: %w", err)
    }

    // Create initial response
    initialResponse := &GetFileResult{
        FileSize: fileInfo.Size(),
    }

    // Create producer function
    producerFunc := func(ctx context.Context, elemChan chan<- *FileChunk) error {
        defer file.Close()

        buffer := make([]byte, 4096) // 4KB chunks

        for {
            n, err := file.Read(buffer)
            if err != nil && err != io.EOF {
                return fmt.Errorf("error reading file: %w", err)
            }

            if n == 0 {
                break // End of file
            }

            chunk := &FileChunk{
                Data: buffer[:n],
            }

            // Send chunk to client
            select {
            case <-ctx.Done():
                return ctx.Err()
            case elemChan <- chunk:
                // Chunk sent successfully
            }

            if err == io.EOF {
                break
            }
        }

        return nil // Successful completion
    }

    return initialResponse, producerFunc, nil
}
```

## Additional Resources

- [General Thrift Streaming Documentation](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/streaming/)
