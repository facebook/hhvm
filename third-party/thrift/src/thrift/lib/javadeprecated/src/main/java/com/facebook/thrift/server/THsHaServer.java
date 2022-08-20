/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.facebook.thrift.server;

import com.facebook.thrift.TProcessor;
import com.facebook.thrift.TProcessorFactory;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TFramedTransport;
import com.facebook.thrift.transport.TNonblockingServerTransport;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * An extension of the TNonblockingServer to a Half-Sync/Half-Async server. Like TNonblockingServer,
 * it relies on the use of TFramedTransport.
 */
public class THsHaServer extends TNonblockingServer {

  // This wraps all the functionality of queueing and thread pool management
  // for the passing of Invocations from the Selector to workers.
  private ExecutorService invoker;

  protected final Options options_;

  /**
   * Create server with given processor, and server transport. Default server options,
   * TBinaryProtocol for the protocol, and TFramedTransport.Factory on both input and output
   * transports. A TProcessorFactory will be created that always returns the specified processor.
   */
  public THsHaServer(TProcessor processor, TNonblockingServerTransport serverTransport) {
    this(processor, serverTransport, new Options());
  }

  /**
   * Create server with given processor, server transport, and server options using TBinaryProtocol
   * for the protocol, and TFramedTransport.Factory on both input and output transports. A
   * TProcessorFactory will be created that always returns the specified processor.
   */
  public THsHaServer(
      TProcessor processor, TNonblockingServerTransport serverTransport, Options options) {
    this(new TProcessorFactory(processor), serverTransport, options);
  }

  /**
   * Create server with specified processor factory and server transport. Uses default options.
   * TBinaryProtocol is assumed. TFramedTransport.Factory is used on both input and output
   * transports.
   */
  public THsHaServer(
      TProcessorFactory processorFactory, TNonblockingServerTransport serverTransport) {
    this(processorFactory, serverTransport, new Options());
  }

  /**
   * Create server with specified processor factory, server transport, and server options.
   * TBinaryProtocol is assumed. TFramedTransport.Factory is used on both input and output
   * transports.
   */
  public THsHaServer(
      TProcessorFactory processorFactory,
      TNonblockingServerTransport serverTransport,
      Options options) {
    this(
        processorFactory,
        serverTransport,
        new TFramedTransport.Factory(),
        new TBinaryProtocol.Factory(),
        options);
  }

  /**
   * Server with specified processor, server transport, and in/out protocol factory. Defaults will
   * be used for in/out transport factory and server options.
   */
  public THsHaServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TProtocolFactory protocolFactory) {
    this(processor, serverTransport, protocolFactory, new Options());
  }

  /**
   * Server with specified processor, server transport, and in/out protocol factory. Defaults will
   * be used for in/out transport factory and server options.
   */
  public THsHaServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TProtocolFactory protocolFactory,
      Options options) {
    this(
        new TProcessorFactory(processor),
        serverTransport,
        new TFramedTransport.Factory(),
        protocolFactory,
        options);
  }

  /**
   * Create server with specified processor, server transport, in/out transport factory, in/out
   * protocol factory, and default server options. A processor factory will be created that always
   * returns the specified processor.
   */
  public THsHaServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory transportFactory,
      TProtocolFactory protocolFactory) {
    this(new TProcessorFactory(processor), serverTransport, transportFactory, protocolFactory);
  }

  /**
   * Create server with specified processor factory, server transport, in/out transport factory,
   * in/out protocol factory, and default server options.
   */
  public THsHaServer(
      TProcessorFactory processorFactory,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory transportFactory,
      TProtocolFactory protocolFactory) {
    this(
        processorFactory,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory,
        new Options());
  }

  /**
   * Create server with specified processor factory, server transport, in/out transport factory,
   * in/out protocol factory, and server options.
   */
  public THsHaServer(
      TProcessorFactory processorFactory,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory transportFactory,
      TProtocolFactory protocolFactory,
      Options options) {
    this(
        processorFactory,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory,
        options);
  }

  /** Create server with everything specified, except use default server options. */
  public THsHaServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory inputTransportFactory,
      TFramedTransport.Factory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    this(
        new TProcessorFactory(processor),
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory);
  }

  /** Create server with everything specified, except use default server options. */
  public THsHaServer(
      TProcessorFactory processorFactory,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory inputTransportFactory,
      TFramedTransport.Factory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    this(
        processorFactory,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory,
        new Options());
  }

  /** Create server with every option fully specified. */
  public THsHaServer(
      TProcessorFactory processorFactory,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory inputTransportFactory,
      TFramedTransport.Factory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory,
      Options options) {
    super(
        processorFactory,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory,
        options);
    options_ = options;
  }

  /** @inheritDoc */
  @Override
  public void serve() {
    if (!startInvokerPool()) {
      return;
    }

    // start listening, or exit
    if (!startListening()) {
      return;
    }

    // start the selector, or exit
    if (!startSelectorThread()) {
      return;
    }

    // this will block while we serve
    joinSelector();

    gracefullyShutdownInvokerPool();

    // do a little cleanup
    stopListening();

    // ungracefully shut down the invoker pool?
  }

  protected boolean startInvokerPool() {
    // start the invoker pool
    BlockingQueue<Runnable> queue =
        (options_.hsHaQueueSize == 0)
            ? new SynchronousQueue<Runnable>()
            : new LinkedBlockingQueue<Runnable>(options_.hsHaQueueSize);
    invoker =
        new ThreadPoolExecutor(
            options_.minHsHaWorkerThreads,
            options_.maxHsHaWorkerThreads,
            options_.stopTimeoutVal,
            options_.stopTimeoutUnit,
            queue,
            new TThreadFactoryImpl("THsHaServer-Invoker"));

    return true;
  }

  protected void gracefullyShutdownInvokerPool() {
    // try to gracefully shut down the executor service
    invoker.shutdown();

    // Loop until awaitTermination finally does return without a interrupted
    // exception. If we don't do this, then we'll shut down prematurely. We want
    // to let the executorService clear it's task queue, closing client sockets
    // appropriately.
    long timeoutMS = 10000;
    long now = System.currentTimeMillis();
    while (timeoutMS >= 0) {
      try {
        invoker.awaitTermination(timeoutMS, TimeUnit.MILLISECONDS);
        break;
      } catch (InterruptedException ix) {
        long newnow = System.currentTimeMillis();
        timeoutMS -= (newnow - now);
        now = newnow;
      }
    }
  }

  /**
   * We override the standard invoke method here to queue the invocation for invoker service instead
   * of immediately invoking. The thread pool takes care of the rest.
   */
  @Override
  protected void requestInvoke(FrameBuffer frameBuffer) throws ServerOverloadedException {
    try {
      invoker.execute(new Invocation(frameBuffer));
    } catch (RejectedExecutionException e) {
      throw new ServerOverloadedException(e);
    }
  }

  /**
   * An Invocation represents a method call that is prepared to execute, given an idle worker
   * thread. It contains the input and output protocols the thread's processor should use to perform
   * the usual Thrift invocation.
   */
  private static class Invocation implements Runnable {

    private final FrameBuffer frameBuffer;

    public Invocation(final FrameBuffer frameBuffer) {
      this.frameBuffer = frameBuffer;
    }

    public void run() {
      frameBuffer.invoke();
    }
  }

  public static class Options extends TNonblockingServer.Options {
    public int minHsHaWorkerThreads = 8;
    public int maxHsHaWorkerThreads = Integer.MAX_VALUE;
    public int hsHaQueueSize = Integer.MAX_VALUE;
    public int stopTimeoutVal = 60;
    public TimeUnit stopTimeoutUnit = TimeUnit.SECONDS;
  }
}
