# Wangle Tutorial

## Introduction

The tutorial assumes that you have installed Wangle and its dependencies. The tutorial will demonstrate how to build an echo server - the "hello world" of distributed systems.

## What is Wangle?

Wangle is a client/server application framework to build asynchronous, event-driven modern C++ services. The fundamental abstraction of Wangle is the Pipeline. Once you have fully understood this abstraction, you will be able to write all sorts of sophisticated modern C++ services. Another important abstraction is Service, which is an advanced version of a pipeline but it’s out of scope for this post.

## What is a Pipeline?

The pipeline is the most important and powerful abstraction of Wangle. It offers immense flexibility to customize how requests and responses are handled by your service.

A pipeline is a chain of request/response handlers that handle upstream (handling request) and downstream (handling response). Once you chain handlers together, it provides an agile way to convert a raw data stream into the desired message type (class) and the inverse -- desired message type to raw data stream.

A handler should do one and only one function - just like the UNIX philosophy. If you have a handler that is doing more than one function than you should split it into individual handlers. This is really important for maintainability and flexibility as its common to change your protocol for one reason or the other.

All shared state within handlers are not thread-safe. Only use shared state that is guarded by a mutex, atomic lock, etc. If you want to use a thread-safe container then it is recommended to use Folly's lock-free data structures, which can be easily imported because they are a dependency of Wangle and are blazing fast.

## Echo Server

Now onto writing our first service with Wangle: the Echo Server. 

Here's the main piece of code in our echo server; it receives a string, prints it to stdout and sends it back downstream in the pipeline. It's really important to add the line delimiter because our pipeline will use a line decoder.

``` cpp
// the main logic of our echo server; receives a string and writes it straight
// back
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  virtual void read(Context* ctx, std::string msg) override {
    std::cout << "handling " << msg << std::endl;
    write(ctx, msg + "\r\n");
  }
};
```

This needs to be the final handler in the pipeline. Now the definition of the pipeline is needed to handle the requests and responses.

```cpp
// where we define the chain of handlers for each messeage received
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LineBasedFrameDecoder(8192));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
 };
```

It is **very** important to be strict in the order of insertion as they are ordered by insertion. The pipeline has 4 handlers:

  - **AsyncSocketHandler**
    - Upstream: Reads a raw data stream from the socket and converts it into a zero-copy byte buffer.
    - Downstream: Writes the contents of a zero-copy byte buffer to the underlying socket.
  - **LineBasedFrameDecoder**
    - Upstream: receives a zero-copy byte buffer and splits on line-endings
    - Downstream: just passes the byte buffer to AsyncSocketHandler
  - **StringCodec**
    - Upstream: receives a byte buffer and decodes it into a std::string and pass up to the EchoHandler.
    - Downstream: receives a std::string and encodes it into a byte buffer and pass down to the LineBasedFrameDecoder.
  - **EchoHandler**
    - Upstream: receives a std::string and writes it to the pipeline — which will send the message downstream.
    - Downstream: receives a std::string and forwards it to StringCodec.

Now that all needs to be done is plug the pipeline factory into a ServerBootstrap and that’s pretty much it. Bind a port and wait for it to stop.

```cpp
#include <folly/portability/GFlags.h>

#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 8080, "echo server port");

typedef Pipeline<IOBufQueue&, std::string> EchoPipeline;

// the main logic of our echo server; receives a string and writes it straight
// back
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  virtual void read(Context* ctx, std::string msg) override {
    std::cout << "handling " << msg << std::endl;
    write(ctx, msg + "\r\n");
  }
};

// where we define the chain of handlers for each message received
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(LineBasedFrameDecoder(8192));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  ServerBootstrap<EchoPipeline> server;
  server.childPipeline(std::make_shared<EchoPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
```

We've written an asynchronous C++ server in under 48 LOC.

## Echo Client

The code for the echo client is very similar to the Echo Server. Here is the main echo handler.

```cpp
// the handler for receiving messages back from the server
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  virtual void read(Context* ctx, std::string msg) override {
    std::cout << "received back: " << msg;
  }
  virtual void readException(Context* ctx, exception_wrapper e) override {
    std::cout << exceptionStr(e) << std::endl;
    close(ctx);
  }
  virtual void readEOF(Context* ctx) override {
    std::cout << "EOF received :(" << std::endl;
    close(ctx);
  }
};
```

Notice that we override other methods — readException and readEOF. There are few other methods that can be overriden. If you need to handle a particular event, just override the corresponding virtual method.

Now onto the client’s pipeline factory. It is identical the server’s pipeline factory apart from _EventBaseHandler_ — which handles writing data from an event loop thread.

```cpp
// chains the handlers together to define the response pipeline
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(
       EventBaseHandler()); // ensure we can write from any thread
    pipeline->addBack(LineBasedFrameDecoder(8192, false));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
};
```

What does it looks like when it is all put together for the client?

```cpp
#include <folly/portability/GFlags.h>
#include <iostream>

#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LineBasedFrameDecoder.h>
#include <wangle/codec/StringCodec.h>

using namespace folly;
using namespace wangle;

DEFINE_int32(port, 8080, "echo server port");
DEFINE_string(host, "::1", "echo server address");

typedef Pipeline<folly::IOBufQueue&, std::string> EchoPipeline;

// the handler for receiving messages back from the server
class EchoHandler : public HandlerAdapter<std::string> {
 public:
  virtual void read(Context* ctx, std::string msg) override {
    std::cout << "received back: " << msg;
  }
  virtual void readException(Context* ctx, exception_wrapper e) override {
    std::cout << exceptionStr(e) << std::endl;
    close(ctx);
  }
  virtual void readEOF(Context* ctx) override {
    std::cout << "EOF received :(" << std::endl;
    close(ctx);
  }
};

// chains the handlers together to define the response pipeline
class EchoPipelineFactory : public PipelineFactory<EchoPipeline> {
 public:
  EchoPipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport> sock) {
    auto pipeline = EchoPipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    pipeline->addBack(
        EventBaseHandler()); // ensure we can write from any thread
    pipeline->addBack(LineBasedFrameDecoder(8192, false));
    pipeline->addBack(StringCodec());
    pipeline->addBack(EchoHandler());
    pipeline->finalize();
    return pipeline;
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  ClientBootstrap<EchoPipeline> client;
  client.group(std::make_shared<folly::IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<EchoPipelineFactory>());
  auto pipeline = client.connect(SocketAddress(FLAGS_host, FLAGS_port)).get();

  try {
    while (true) {
      std::string line;
      std::getline(std::cin, line);
      if (line == "") {
        break;
      }

      pipeline->write(line + "\r\n").get();
      if (line == "bye") {
        pipeline->close();
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cout << exceptionStr(e) << std::endl;
  }

  return 0;
}
```

It reads input from stdin in a loop and writes it to the pipeline and it blocks until the response is processed. It blocks by calling .get() from the returned future.

## Summary

This quick tutorial has shown how to quickly write a basic service in modern C++ using Wangle. You should now know the fundamentals of Wangle and it should give you confidence to write your own service in C++. It is strongly recommend to understand the Service abstraction once you are comfortable with using the Pipeline as you can build sophisticated servers with it.
