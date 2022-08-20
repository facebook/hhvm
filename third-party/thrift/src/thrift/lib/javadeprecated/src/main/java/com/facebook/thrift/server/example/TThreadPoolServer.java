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

package com.facebook.thrift.server.example;

import com.facebook.thrift.TException;
import com.facebook.thrift.TProcessor;
import com.facebook.thrift.TProcessorFactory;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.THeaderProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.server.TRpcConnectionContext;
import com.facebook.thrift.server.TServer;
import com.facebook.thrift.server.TThreadFactoryImpl;
import com.facebook.thrift.transport.TServerTransport;
import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.transport.TTransportException;
import com.facebook.thrift.transport.TTransportFactory;
import com.facebook.thrift.utils.Logger;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @deprecated "Please use Netty server instead. See https://github.com/facebook/nifty for details."
 *     <p>Server which uses Java's built in ThreadPool management to spawn off a worker pool that
 */
@Deprecated
public class TThreadPoolServer extends TServer {

  private static final Logger LOGGER = Logger.getLogger(TThreadPoolServer.class.getName());

  // Executor service for handling client connections
  private ExecutorService executorService_;

  // Flag for stopping the server
  private volatile boolean stopped_;

  // Server options
  private Options options_;

  // Customizable server options
  public static class Options {
    public int minWorkerThreads = 5;
    public int maxWorkerThreads = Integer.MAX_VALUE;
    public int stopTimeoutVal = 60;
    public TimeUnit stopTimeoutUnit = TimeUnit.SECONDS;
    public BlockingQueue<Runnable> executorQueue = null;
    public RejectedExecutionHandler rejectedExecutionHandler = null;
    public ExecutorService executorService = null;
  }

  public TThreadPoolServer(TProcessor processor, TServerTransport serverTransport) {
    this(
        processor,
        serverTransport,
        new TTransportFactory(),
        new TTransportFactory(),
        new TBinaryProtocol.Factory(),
        new TBinaryProtocol.Factory());
  }

  public TThreadPoolServer(TProcessorFactory processorFactory, TServerTransport serverTransport) {
    this(
        processorFactory,
        serverTransport,
        new TTransportFactory(),
        new TTransportFactory(),
        new TBinaryProtocol.Factory(),
        new TBinaryProtocol.Factory());
  }

  public TThreadPoolServer(
      TProcessor processor, TServerTransport serverTransport, TProtocolFactory protocolFactory) {
    this(
        processor,
        serverTransport,
        new TTransportFactory(),
        new TTransportFactory(),
        protocolFactory,
        protocolFactory);
  }

  public TThreadPoolServer(
      TProcessor processor,
      TServerTransport serverTransport,
      TTransportFactory transportFactory,
      TProtocolFactory protocolFactory) {
    this(
        processor,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory);
  }

  public TThreadPoolServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory transportFactory,
      TProtocolFactory protocolFactory) {
    this(
        processorFactory,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory);
  }

  public TThreadPoolServer(
      TProcessor processor,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
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

  public TThreadPoolServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    super(
        processorFactory,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory);
    options_ = new Options();
    executorService_ = Executors.newCachedThreadPool(new TThreadFactoryImpl("thrift"));
  }

  public TThreadPoolServer(
      TProcessor processor,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory,
      Options options) {
    this(
        new TProcessorFactory(processor),
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory,
        options);
  }

  public TThreadPoolServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory,
      Options options) {
    super(
        processorFactory,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory);

    if (options.executorService != null) {
      executorService_ = options.executorService;
    } else {
      RejectedExecutionHandler rejectedExecutionHandler = null;
      if (options.rejectedExecutionHandler != null) {
        rejectedExecutionHandler = options.rejectedExecutionHandler;
      } else {
        rejectedExecutionHandler = new ThreadPoolExecutor.AbortPolicy();
      }

      BlockingQueue<Runnable> executorQueue = null;
      if (options.executorQueue != null) {
        executorQueue = options.executorQueue;
      } else {
        executorQueue = new SynchronousQueue<Runnable>();
      }
      executorService_ =
          new ThreadPoolExecutor(
              options.minWorkerThreads,
              options.maxWorkerThreads,
              60,
              TimeUnit.SECONDS,
              executorQueue,
              new TThreadFactoryImpl("thrift"),
              rejectedExecutionHandler);
    }

    options_ = options;
  }

  public void serve() {
    try {
      serverTransport_.listen();
    } catch (TTransportException ttx) {
      LOGGER.error("Error occurred during listening.", ttx);
      return;
    }

    stopped_ = false;
    while (!stopped_) {
      try {
        TTransport client = serverTransport_.accept();
        WorkerProcess wp = new WorkerProcess(client);
        try {
          executorService_.execute(wp);
        } catch (RejectedExecutionException ree) {
          LOGGER.warn("Client request was rejected by executor", ree);
          // close the connection, we don't need it anymore
          // if clause should always be true, but just to be on the safe side
          if (client != null && client.isOpen()) {
            client.close();
          }
        }
      } catch (TTransportException ttx) {
        if (!stopped_) {
          LOGGER.warn("Transport error occurred during acceptance of message.", ttx);
        }
      }
    }

    executorService_.shutdown();

    // Loop until awaitTermination finally does return without a interrupted
    // exception. If we don't do this, then we'll shut down prematurely. We want
    // to let the executorService clear it's task queue, closing client sockets
    // appropriately.
    long timeoutMS = options_.stopTimeoutUnit.toMillis(options_.stopTimeoutVal);
    long now = System.currentTimeMillis();
    while (timeoutMS >= 0) {
      try {
        executorService_.awaitTermination(timeoutMS, TimeUnit.MILLISECONDS);
        break;
      } catch (InterruptedException ix) {
        long newnow = System.currentTimeMillis();
        timeoutMS -= (newnow - now);
        now = newnow;
      }
    }
  }

  public void stop() {
    stopped_ = true;
    serverTransport_.interrupt();
  }

  private class WorkerProcess implements Runnable {

    /** Client that this services. */
    private TTransport client_;

    /**
     * Default constructor.
     *
     * @param client Transport to process
     */
    private WorkerProcess(TTransport client) {
      client_ = client;
    }

    /** Loops on processing a client forever */
    public void run() {
      TProcessor processor = null;
      TTransport inputTransport = null;
      TTransport outputTransport = null;
      TProtocol inputProtocol = null;
      TProtocol outputProtocol = null;
      try {
        processor = processorFactory_.getProcessor(client_);
        inputTransport = inputTransportFactory_.getTransport(client_);
        inputProtocol = inputProtocolFactory_.getProtocol(inputTransport);
        // THeaderProtocol must be the same instance for both input and output
        if (inputProtocol instanceof THeaderProtocol) {
          outputProtocol = inputProtocol;
        } else {
          outputTransport = outputTransportFactory_.getTransport(client_);
          outputProtocol = outputProtocolFactory_.getProtocol(outputTransport);
        }
        // we check stopped_ first to make sure we're not supposed to be shutting
        // down. this is necessary for graceful shutdown.
        TRpcConnectionContext server_ctx =
            new TRpcConnectionContext(client_, inputProtocol, outputProtocol);
        while (!stopped_ && processor.process(inputProtocol, outputProtocol, server_ctx)) {}
      } catch (TTransportException ttx) {
        // Assume the client died and continue silently
      } catch (TException tx) {
        LOGGER.error("Thrift error occurred during processing of message.", tx);
      } catch (Exception x) {
        LOGGER.error("Error occurred during processing of message.", x);
      }

      if (inputTransport != null) {
        inputTransport.close();
      }

      if (outputTransport != null) {
        outputTransport.close();
      }
    }
  }
}
