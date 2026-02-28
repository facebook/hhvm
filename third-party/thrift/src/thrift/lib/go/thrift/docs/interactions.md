# Thrift Interactions in Go

This document explains how to use Thrift Interactions in Go, both from the
client side and server side.

For general information about Thrift Interactions concepts and how to define
interactions in Thrift IDL, please refer to the
[Thrift Interactions Documentation](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/interactions/).

## Table of Contents

- [Overview](#overview)
- [Defining Interactions](#defining-interactions)
- [Client-Side Usage](#client-side-usage)
- [Server-Side Implementation](#server-side-implementation)

## Overview

Thrift Interactions provide a way to create stateful RPC sessions between
clients and servers. Unlike regular RPC methods that are stateless, interactions
allow you to maintain state across multiple method calls within a session. The
client creates an interaction "channel" or "session" by calling an interaction
factory method, and then uses that channel to make subsequent calls that can
access and modify shared state.

**Key Routing Behavior:** All RPC calls within an interaction are automatically
routed to the same host over the same connection. This ensures that all methods
in an interaction session access the same server-side state, making interactions
ideal for use cases that require maintaining context across multiple related
requests.

## Defining Interactions

Define interactions in your Thrift IDL file using the `interaction` keyword. An
interaction is created via a factory method that returns the interaction type.

Here's an example showing both interaction creation patterns:

```thrift
struct FileInfo {
  1: i64 size;
  2: i64 modifiedTime;
}

// Define an interaction with methods
interaction FileAccess {
  i64 /*offset*/ seek(1: i32 delta);
  stream<binary> /*chunk stream*/ read(1: i32 bytes);
}

service FileServer {
  // Creates a FileAccess interaction (no initial response)
  FileAccess open(1: string path);

  // Creates a FileAccess interaction (with initial response - FileInfo)
  // Note: Interaction is always the leftmost return value
  FileAccess, FileInfo openAndStat(1: string path);

  // Regular non-interaction method
  FileInfo stat(1: string filePath);
}
```

Key points:

- The `interaction` keyword defines a stateful session with multiple methods
- Interaction factory methods return the interaction type
- Interactions can be returned with or without an initial response value
- When an initial response is included, the interaction is always the leftmost
  return value

## Client-Side Usage

### API Signature

The generated client API for interaction factory methods returns an interaction
channel that you use to make method calls:

**Interaction without initial response:**

```go
func (c *Client) Open(ctx context.Context, path string) (
    FileAccessClient, // Interaction client/session
    error,            // Error
)
```

**Interaction with initial response:**

```go
func (c *Client) OpenAndStat(ctx context.Context, path string) (
    FileAccessClient, // Interaction client/session
    *FileInfo,        // Initial response value
    error,            // Error
)
```

The returned interaction client has its own methods that you can call to
interact with the stateful session on the server.

### Usage Guidelines

1. **Create Interaction Client**: Call the interaction factory method to create
   a new interaction session.

2. **Check Error**: Always check the error returned from the factory method. If
   non-nil, no interaction was created.

3. **Use Interaction Methods**: Call methods on the returned interaction client.
   Each call operates on the same server-side state.

4. **State Persistence**: State is maintained across calls for the lifetime of
   the interaction client.

5. **Cleanup**: Close the client when done to clean up resources.

### Example: Client-Side Interaction Usage

```go
func main() {
    ctx := context.Background()

    channel, err := thrift.NewClient(
        thrift.WithRocket(),
        thrift.WithDialer(func() (net.Conn, error) {
            return net.Dial("tcp", "localhost:9090")
        }),
    )
    if err != nil {
        log.Fatalf("failed to create channel: %v", err)
    }
    defer channel.Close()

    client := fileserver.NewFileServerChannelClient(channel)

    // Create an interaction with initial response
    // Note: Interaction client comes first (leftmost), then initial response
    fileAccessClient, fileInfo, err := client.OpenAndStat(ctx, "/tmp/myfile.txt")
    if err != nil {
        log.Fatalf("failed to open and stat file: %v", err)
    }
    defer fileAccessClient.Close()

    fmt.Printf("File size: %d bytes\n", fileInfo.GetSize())
    fmt.Printf("Modified time: %d\n", fileInfo.GetModifiedTime())

    // Now use the interaction to seek and read
    offset, err := fileAccessClient.Seek(ctx, 0)
    if err != nil {
        log.Fatalf("seek failed: %v", err)
    }
    fmt.Printf("Seeked to offset: %d\n", offset)
}
```

## Server-Side Implementation

### API Signature

Server-side interaction factory methods have a signature that returns a
processor for the interaction:

**Interaction without initial response:**

```go
func (h *Handler) Open(ctx context.Context, path string) (
    *FileAccessProcessor, // Interaction processor
    error,                // Error
)
```

**Interaction with initial response:**

```go
func (h *Handler) OpenAndStat(ctx context.Context, path string) (
    *FileAccessProcessor, // Interaction processor
    *FileInfo,            // Initial response
    error,                // Error
)
```

The interaction processor is created by passing an implementation of the
interaction interface to the generated processor constructor.

### Implementation Guidelines

1. **Create Interaction Handler**: Implement the interaction interface with
   methods that maintain state.

2. **Store State**: Use struct fields to maintain state across method calls
   within the interaction.

3. **Create Processor**: Return a new interaction processor wrapping your
   handler implementation.

4. **Return Initial Response (Optional)**: If the interaction factory method has
   a return value, return it along with the processor.

5. **Implement Termination (Optional)**: Implement the `OnTermination()` method
   from the `types.Terminable` interface to clean up resources when the
   interaction ends.

6. **Error Handling**: Return errors from the factory method if initialization
   fails.

### Example: Server-Side Interaction Usage

```go
// Implement the main service handler
type FileServerHandler struct{}

// Open creates a FileAccess interaction (no initial response)
func (h *FileServerHandler) Open(ctx context.Context, path string) (*fileserver.FileAccessProcessor, error) {
    // Open the file
    file, err := os.Open(path)
    if err != nil {
        return nil, fmt.Errorf("failed to open file: %w", err)
    }

    // Create the interaction handler
    fileAccessHandler := &FileAccessHandler{
        file: file,
    }

    // Return a processor that wraps the interaction handler
    return fileserver.NewFileAccessProcessor(fileAccessHandler), nil
}

// OpenAndStat creates a FileAccess interaction with initial response (FileInfo)
func (h *FileServerHandler) OpenAndStat(ctx context.Context, path string) (
    *fileserver.FileAccessProcessor,
    *fileserver.FileInfo,
    error,
) {
    // Stat the file first
    fileInfo, err := os.Stat(path)
    if err != nil {
        return nil, nil, fmt.Errorf("failed to stat file: %w", err)
    }

    // Open the file
    file, err := os.Open(path)
    if err != nil {
        return nil, nil, fmt.Errorf("failed to open file: %w", err)
    }

    // Create initial response
    initialResponse := &fileserver.FileInfo{
        Size:         fileInfo.Size(),
        ModifiedTime: fileInfo.ModTime().Unix(),
    }

    // Create the interaction handler
    fileAccessHandler := &FileAccessHandler{
        file: file,
    }

    // Return processor, initial response, and nil error
    return fileserver.NewFileAccessProcessor(fileAccessHandler), initialResponse, nil
}

// Implement the interaction handler
type FileAccessHandler struct {
    file *os.File // State persisted across calls
}

// Compile time check that FileAccessHandler implements the FileAccess interface
var _ fileserver.FileAccess = (*FileAccessHandler)(nil)

func (h *FileAccessHandler) Seek(ctx context.Context, delta int32) (int64, error) {
    // Seek relative to current position
    offset, err := h.file.Seek(int64(delta), io.SeekCurrent)
    if err != nil {
        return 0, fmt.Errorf("seek failed: %w", err)
    }
    return offset, nil
}

func (h *FileAccessHandler) Read(ctx context.Context, bytes int32) (func(context.Context, chan<- []byte) error, error) {
    // Return producer function for streaming
    producerFunc := func(ctx context.Context, dataChan chan<- []byte) error {
        buffer := make([]byte, bytes)
        n, err := h.file.Read(buffer)
        if err != nil && err != io.EOF {
            return fmt.Errorf("read failed: %w", err)
        }

        select {
        case <-ctx.Done():
            return ctx.Err()
        case dataChan <- buffer[:n]:
            // Data sent successfully
        }

        return nil
    }

    return producerFunc, nil
}

// OnTermination is called when the interaction is terminated
func (h *FileAccessHandler) OnTermination() {
    // Clean up resources when the interaction ends
    if h.file != nil {
        h.file.Close()
    }
}
```

## Additional Resources

- [General Thrift Interactions Documentation](https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/features/interactions/)
