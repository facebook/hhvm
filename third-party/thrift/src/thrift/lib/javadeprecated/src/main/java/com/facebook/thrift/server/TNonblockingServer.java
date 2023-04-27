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

import com.facebook.thrift.TByteArrayOutputStream;
import com.facebook.thrift.TException;
import com.facebook.thrift.TProcessor;
import com.facebook.thrift.TProcessorFactory;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.THeaderProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TFramedTransport;
import com.facebook.thrift.transport.TIOStreamTransport;
import com.facebook.thrift.transport.TNonblockingServerTransport;
import com.facebook.thrift.transport.TNonblockingTransport;
import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.transport.TTransportException;
import com.facebook.thrift.utils.Logger;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

/**
 * A nonblocking TServer implementation. This allows for fairness amongst all connected clients in
 * terms of invocations.
 *
 * <p>This server is inherently single-threaded. If you want a limited thread pool coupled with
 * invocation-fairness, see THsHaServer.
 *
 * <p>To use this server, you MUST use a TFramedTransport at the outermost transport, otherwise this
 * server will be unable to determine when a whole method call has been read off the wire. Clients
 * must also use TFramedTransport.
 */
public class TNonblockingServer extends TServer {
  private static final Logger LOGGER = Logger.getLogger(TNonblockingServer.class.getName());
  private static final int selectWaitTime_ = 500;

  private final ExecutorService tcpWorkerPool_;
  // Flag for stopping the server
  private volatile boolean stopped_ = true;

  private SelectThread selectThread_;

  /**
   * The maximum amount of memory we will allocate to client IO buffers at a time. Without this
   * limit, the server will gladly allocate client buffers right into an out of memory exception,
   * rather than waiting.
   */
  private final long MAX_READ_BUFFER_BYTES;

  protected final Options options_;

  /** How many bytes are currently allocated to read buffers. */
  private final AtomicLong readBufferBytesAllocated = new AtomicLong(0);

  /**
   * Create server with given processor and server transport, using TBinaryProtocol for the
   * protocol, TFramedTransport.Factory on both input and output transports. A TProcessorFactory
   * will be created that always returns the specified processor.
   */
  public TNonblockingServer(TProcessor processor, TNonblockingServerTransport serverTransport) {
    this(new TProcessorFactory(processor), serverTransport);
  }

  /**
   * Create server with specified processor factory and server transport. TBinaryProtocol is
   * assumed. TFramedTransport.Factory is used on both input and output transports.
   */
  public TNonblockingServer(
      TProcessorFactory processorFactory, TNonblockingServerTransport serverTransport) {
    this(
        processorFactory,
        serverTransport,
        new TFramedTransport.Factory(),
        new TFramedTransport.Factory(),
        new TBinaryProtocol.Factory(),
        new TBinaryProtocol.Factory());
  }

  public TNonblockingServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TProtocolFactory protocolFactory) {
    this(
        processor,
        serverTransport,
        new TFramedTransport.Factory(),
        new TFramedTransport.Factory(),
        protocolFactory,
        protocolFactory);
  }

  public TNonblockingServer(
      TProcessor processor,
      TNonblockingServerTransport serverTransport,
      TFramedTransport.Factory transportFactory,
      TProtocolFactory protocolFactory) {
    this(
        processor,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory);
  }

  public TNonblockingServer(
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
        protocolFactory);
  }

  public TNonblockingServer(
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

  public TNonblockingServer(
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

  public TNonblockingServer(
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
        outputProtocolFactory);
    options_ = options;
    options_.validate();
    MAX_READ_BUFFER_BYTES = options.maxReadBufferBytes;
    BlockingQueue<Runnable> queue =
        (options_.queueSize == 0)
            ? new SynchronousQueue<Runnable>()
            : new LinkedBlockingQueue<Runnable>(options_.queueSize);
    tcpWorkerPool_ =
        new ThreadPoolExecutor(
            options_.minWorkerThreads,
            options_.maxWorkerThreads,
            Integer.MAX_VALUE,
            TimeUnit.MILLISECONDS,
            queue,
            new TThreadFactoryImpl("thrift-tcpworker"));
  }

  protected void submitTask(Runnable r) throws ServerOverloadedException {
    try {
      if (options_.forceSingleThreaded) {
        r.run();
      } else {
        tcpWorkerPool_.execute(r);
      }
    } catch (RejectedExecutionException e) {
      throw new ServerOverloadedException(e);
    }
  }

  /** Begin accepting connections and processing invocations. */
  public void serve() {
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

    // do a little cleanup
    stopListening();
  }

  /**
   * Have the server transport start accepting connections.
   *
   * @return true if we started listening successfully, false if something went wrong.
   */
  protected boolean startListening() {
    try {
      serverTransport_.listen();
      return true;
    } catch (TTransportException ttx) {
      LOGGER.error("Failed to start listening on server socket!", ttx);
      return false;
    }
  }

  /** Stop listening for connections. */
  protected void stopListening() {
    serverTransport_.close();
  }

  /**
   * Start the selector thread running to deal with clients.
   *
   * @return true if everything went ok, false if we couldn't start for some reason.
   */
  protected boolean startSelectorThread() {
    // start the selector
    try {
      selectThread_ = new SelectThread((TNonblockingServerTransport) serverTransport_);
      stopped_ = false;
      selectThread_.start();
      return true;
    } catch (IOException e) {
      LOGGER.error("Failed to start selector thread!", e);
      return false;
    }
  }

  /** Block until the selector exits. */
  protected void joinSelector() {
    // wait until the selector thread exits
    try {
      selectThread_.join();
    } catch (InterruptedException e) {
      // for now, just silently ignore. technically this means we'll have
      // less of
      // a graceful shutdown as a result.
    }
  }

  /** Stop serving and shut everything down. */
  public void stop() {
    stopped_ = true;
    if (selectThread_ != null) {
      selectThread_.wakeupSelector();
    }
  }

  protected long getReadBufferBytesAllocated() {
    return readBufferBytesAllocated.get();
  }

  /**
   * Perform an invocation. This method could behave several different ways - invoke immediately
   * inline, queue for separate execution, etc.
   */
  protected void requestInvoke(FrameBuffer frameBuffer) throws ServerOverloadedException {
    frameBuffer.invoke();
  }

  public boolean isStopped() {
    return selectThread_.isStopped();
  }

  /**
   * Frame buffer requests cancellation of its preferences.
   *
   * @param frameBuffer for which cancellation is requested.
   */
  protected void requestCancellation(FrameBuffer frameBuffer) {
    selectThread_.requestCancellation(frameBuffer);
  }

  /**
   * The thread that will be doing all the selecting, managing new connections and those that still
   * need to be read.
   */
  protected class SelectThread extends Thread {

    private static final String name_ = "Thrift-Selector";

    private abstract class TcpHandler implements Runnable {
      protected final SelectionKey sKey_;

      TcpHandler(SelectionKey sKey) {
        sKey_ = sKey;
      }
    }

    private class TcpReader extends TcpHandler {
      TcpReader(SelectionKey sKey) {
        super(sKey);
      }

      public void run() {
        SelectThread.this.handleRead(sKey_);
      }
    }

    private class TcpWriter extends TcpHandler {
      TcpWriter(SelectionKey sKey) {
        super(sKey);
      }

      public void run() {
        SelectThread.this.handleWrite(sKey_);
      }
    }

    private final TNonblockingServerTransport serverTransport;
    private final Selector selector;

    // List of FrameBuffers that want to cancel their keys.
    private final Set<FrameBuffer> cancellations = new HashSet<FrameBuffer>();

    /** Set up the SelectorThread. */
    public SelectThread(final TNonblockingServerTransport serverTransport) throws IOException {
      super(SelectThread.name_);
      this.serverTransport = serverTransport;
      this.selector = SelectorProvider.provider().openSelector();
      serverTransport.registerSelector(selector);
    }

    public boolean isStopped() {
      return stopped_;
    }

    /**
     * The work loop. Handles both selecting (all IO operations) and managing the selection
     * preferences of all existing connections.
     */
    public void run() {
      while (!stopped_) {
        /* Keeps track of iteration through the selector */
        select();
        processCancellations();
      }
    }

    /** If the selector is blocked, wake it up. */
    public void wakeupSelector() {
      selector.wakeup();
    }

    public void requestCancellation(FrameBuffer frameBuffer) {
      synchronized (cancellations) {
        cancellations.add(frameBuffer);
      }
      // wakeup the selector, if it's currently blocked.
      selector.wakeup();
    }

    /**
     * Select and process IO events appropriately: If there are connections to be accepted, accept
     * them. If there are existing connections with data waiting to be read, read it, buffering
     * until a whole frame has been read. If there are any pending responses, buffer them until
     * their target client is available, and then send the data.
     */
    private void select() {
      try {
        // wait for io events.
        selector.select(TNonblockingServer.selectWaitTime_);

        // process the io events we received
        Iterator<SelectionKey> selectedKeys = selector.selectedKeys().iterator();
        while (!stopped_ && selectedKeys.hasNext()) {
          SelectionKey key = selectedKeys.next();
          selectedKeys.remove();

          // skip if not valid
          if (!key.isValid()) {
            cleanupSelectionkey(key);
            continue;
          }

          // if the key is marked Accept, then it has to be the server
          // transport.
          if (key.isAcceptable()) {
            handleAccept();
          } else if (key.isReadable()) {
            // deal with reads
            key.interestOps(0);
            TcpHandler readHandler = new TcpReader(key);
            try {
              TNonblockingServer.this.submitTask(readHandler);
            } catch (ServerOverloadedException e) {
              cleanupSelectionkey(key);
            }
          } else if (key.isWritable()) {
            // deal with writes
            key.interestOps(0);
            TcpHandler writeHandler = new TcpWriter(key);
            try {
              TNonblockingServer.this.submitTask(writeHandler);
            } catch (ServerOverloadedException e) {
              cleanupSelectionkey(key);
            }
          } else {
            LOGGER.warn("Unexpected state in select! " + key.interestOps());
          }
        }
      } catch (IOException e) {
        LOGGER.warn("Got an IOException while selecting!", e);
      }
    }

    private void processCancellations() {
      synchronized (cancellations) {
        for (FrameBuffer fb : cancellations) {
          fb.cancel();
        }
        cancellations.clear();
      }
    }

    /** Accept a new connection. */
    private void handleAccept() throws IOException {
      TNonblockingTransport client = null;
      try {
        client = (TNonblockingTransport) serverTransport.accept();
      } catch (TTransportException tte) {
        LOGGER.warn("Exception trying to accept!", tte);
        return;
      }

      SelectionKey clientKey = client.registerSelector(selector, SelectionKey.OP_READ);

      // add this key to the map
      FrameBuffer frameBuffer = new FrameBuffer(client, clientKey);
      clientKey.attach(frameBuffer);
    }

    /**
     * Do the work required to read from a readable client. If the frame is fully read, then invoke
     * the method call.
     */
    private void handleRead(SelectionKey key) {
      FrameBuffer buffer = (FrameBuffer) key.attachment();
      if (buffer.read()) {
        // if the buffer's frame read is complete, invoke the method.
        if (buffer.isFrameFullyRead()) {
          try {
            requestInvoke(buffer);
          } catch (ServerOverloadedException e) {
            TNonblockingServer.this.requestCancellation(buffer);
          }
        } else {
          buffer.changeSelectInterests();
        }
      } else {
        TNonblockingServer.this.requestCancellation(buffer);
      }
    }

    /** Let a writable client get written, if there's data to be written. */
    private void handleWrite(SelectionKey key) {
      FrameBuffer buffer = (FrameBuffer) key.attachment();
      if (buffer.write()) {
        buffer.changeSelectInterests();
      } else {
        TNonblockingServer.this.requestCancellation(buffer);
      }
    }

    /** Do connection-close cleanup on a given SelectionKey. */
    private void cleanupSelectionkey(SelectionKey key) {
      // remove the records from the two maps
      FrameBuffer buffer = (FrameBuffer) key.attachment();
      if (buffer != null) {
        // close the buffer
        buffer.close();
      }
      // cancel the selection key
      key.cancel();
    }
  } // SelectorThread

  /**
   * Class that implements a sort of state machine around the interaction with a client and an
   * invoker. It manages reading the frame size and frame data, getting it handed off as wrapped
   * transports, and then the writing of response data back to the client. In the process it manages
   * flipping the read and write bits on the selection key for its client.
   */
  protected class FrameBuffer {
    //
    // Possible states for the FrameBuffer state machine.
    //
    // in the midst of reading the frame size off the wire
    private static final int READING_FRAME_SIZE = 1;
    // reading the actual frame data now, but not all the way done yet
    private static final int READING_FRAME = 2;
    // completely read the frame, so an invocation can now happen
    private static final int READ_FRAME_COMPLETE = 3;
    // waiting to get switched to listening for write events
    private static final int AWAITING_REGISTER_WRITE = 4;
    // started writing response data, not fully complete yet
    private static final int WRITING = 6;
    // another thread wants this framebuffer to go back to reading
    private static final int AWAITING_REGISTER_READ = 7;
    // we want our transport and selection key invalidated in the selector
    // thread
    private static final int AWAITING_CLOSE = 8;

    //
    // Instance variables
    //

    private final long creationTime_;
    // the actual transport hooked up to the client.
    private final TNonblockingTransport trans_;

    // the SelectionKey that corresponds to our transport
    private final SelectionKey selectionKey_;

    // where in the process of reading/writing are we?
    private int state_ = READING_FRAME_SIZE;

    // the ByteBuffer we'll be using to write and read, depending on the state
    private ByteBuffer buffer_;

    private TByteArrayOutputStream response_;

    public FrameBuffer(final TNonblockingTransport trans, final SelectionKey selectionKey) {
      creationTime_ = System.currentTimeMillis();
      trans_ = trans;
      selectionKey_ = selectionKey;
      buffer_ = ByteBuffer.allocate(4);
    }

    public boolean isValid() {
      long currentTime = System.currentTimeMillis();
      return (currentTime - creationTime_) < options_.timeout;
    }

    /**
     * Give this FrameBuffer a chance to read. The selector loop should have received a read event
     * for this FrameBuffer.
     *
     * @return true if the connection should live on, false if it should be closed
     */
    public synchronized boolean read() {
      if (state_ == READING_FRAME_SIZE) {
        // try to read the frame size completely
        if (!internalRead()) {
          return false;
        }

        // if the frame size has been read completely, then prepare to read the
        // actual frame.
        if (buffer_.remaining() == 0) {
          // pull out the frame size as an integer.
          int frameSize = buffer_.getInt(0);
          if (frameSize <= 0) {
            LOGGER.error(
                "Read an invalid frame size of "
                    + frameSize
                    + ". Are you using TFramedTransport on the client side?");
            return false;
          }

          // if this frame will always be too large for this server, log the
          // error and close the connection.
          int newBufferSize = frameSize + 4;
          if (newBufferSize > MAX_READ_BUFFER_BYTES) {
            LOGGER.error(
                "Read a frame size of "
                    + frameSize
                    + ", which is bigger than the maximum allowable buffer size "
                    + "for ALL connections.");
            return false;
          }

          // increment the amount of memory allocated to read buffers
          // if this frame will push us over the memory limit, then return.
          // with luck, more memory will free up the next time around.
          if (readBufferBytesAllocated.addAndGet(newBufferSize) > MAX_READ_BUFFER_BYTES) {
            readBufferBytesAllocated.addAndGet(-newBufferSize);
            return true;
          }
          // reallocate the readbuffer as a frame-sized buffer
          buffer_ = ByteBuffer.allocate(newBufferSize);
          // put the frame size at the head of the buffer
          buffer_.putInt(frameSize);

          state_ = READING_FRAME;
        } else {
          // this skips the check of READING_FRAME state below, since we can't
          // possibly go on to that state if there's data left to be read at
          // this one.
          return true;
        }
      }

      // it is possible to fall through from the READING_FRAME_SIZE section
      // to READING_FRAME if there's already some frame data available once
      // READING_FRAME_SIZE is complete.

      if (state_ == READING_FRAME) {
        if (!internalRead()) {
          return false;
        }

        // since we're already in the select loop here for sure, we can just
        // modify our selection key directly.
        if (buffer_.remaining() == 0) {
          state_ = READ_FRAME_COMPLETE;
        }

        return true;
      }

      // if we fall through to this point, then the state must be invalid.
      LOGGER.error("Read was called but state is invalid (" + state_ + ")");

      return false;
    }

    /** Give this FrameBuffer a chance to write its output to the final client. */
    public synchronized boolean write() {
      if (state_ == WRITING) {
        try {
          if (trans_.write(buffer_) < 0) {
            return false;
          }
        } catch (IOException e) {
          LOGGER.warn("Got an IOException during write!", e);
          return false;
        }

        // we're done writing. now we need to switch back to reading.
        if (buffer_.remaining() == 0) {
          state_ = AWAITING_REGISTER_READ;
        }

        return true;
      }

      LOGGER.error("Write was called, but state is invalid (" + state_ + ")");
      return false;
    }

    /** Give this FrameBuffer a chance to set its interest depending on state */
    public synchronized void changeSelectInterests() {
      if ((state_ == AWAITING_REGISTER_WRITE) || (state_ == WRITING)) {
        state_ = WRITING;
        // set the OP_WRITE interest. Again set it after the assigning of the
        // state in the state machine to avoid inconsistencies between the
        // state machine and the interests.
        selectionKey_.interestOps(SelectionKey.OP_WRITE);
      } else if (state_ == AWAITING_REGISTER_READ) {
        // get ready for another go-around
        buffer_ = ByteBuffer.allocate(4);
        state_ = READING_FRAME_SIZE;
        // Once you set the interest, the selector thread could schedule a read
        // or write on a different thread before you can set the state machine
        // state. That's why the state machine state *MUST* be set before
        // setting the interest
        selectionKey_.interestOps(SelectionKey.OP_READ);
      } else if (state_ == AWAITING_CLOSE) {
        cancel();
      } else if ((state_ == READING_FRAME_SIZE) || (state_ == READING_FRAME)) {
        // Register interest to continue reading.
        selectionKey_.interestOps(SelectionKey.OP_READ);
      } else {
        LOGGER.error("changeSelectInterest was called, but state is invalid (" + state_ + ")");
      }

      // All selector interest ops updates MUST be followed with a call to wake
      // up the selector, unless this thread is currently the selector.
      if (Thread.currentThread() != selectThread_) {
        selectThread_.wakeupSelector();
      }
    }

    public synchronized void cancel() {
      close();
      selectionKey_.cancel();
    }

    /** Shut the connection down. */
    public synchronized void close() {
      // if we're being closed due to an error, we might have allocated a
      // buffer that we need to subtract for our memory accounting.
      if (state_ == READING_FRAME || state_ == READ_FRAME_COMPLETE) {
        readBufferBytesAllocated.addAndGet(-buffer_.capacity());
      }
      trans_.close();
    }

    /** Check if this FrameBuffer has a full frame read. */
    public synchronized boolean isFrameFullyRead() {
      return state_ == READ_FRAME_COMPLETE;
    }

    /**
     * After the processor has processed the invocation, whatever thread is managing invocations
     * should call this method on this FrameBuffer so we know it's time to start trying to write
     * again. Also, if it turns out that there actually isn't any data in the response buffer, we'll
     * skip trying to write and instead go back to reading.
     */
    private void responseReady() {
      // the read buffer is definitely no longer in use, so we will decrement
      // our read buffer count. we do this here as well as in close because
      // we'd like to free this read memory up as quickly as possible for other
      // clients.
      readBufferBytesAllocated.addAndGet(-buffer_.capacity());

      if (response_.len() == 0) {
        // go straight to reading again. this was probably an oneway method
        state_ = AWAITING_REGISTER_READ;
        buffer_ = null;
      } else {
        buffer_ = ByteBuffer.wrap(response_.get(), 0, response_.len());

        // set state that we're waiting to be switched to write. we do this
        // asynchronously through requestSelectInterestChange() because
        // there is a
        // possibility that we're not in the main thread, and thus currently
        // blocked in select(). (this functionality is in place for the sake of
        // the HsHa server.)
        state_ = AWAITING_REGISTER_WRITE;
      }
      changeSelectInterests();
    }

    /** Actually invoke the method signified by this FrameBuffer. */
    public synchronized void invoke() {
      TTransport inTrans;
      TProtocol inProt;
      TProtocol outProt;
      // For THeader inProt and outProt must be the same object
      if (inputProtocolFactory_ instanceof THeaderProtocol.Factory) {
        inTrans = getInputOutputTransport();
        inProt = inputProtocolFactory_.getProtocol(inTrans);
        outProt = inProt;
      } else {
        inTrans = getInputTransport();
        inProt = inputProtocolFactory_.getProtocol(inTrans);
        outProt = outputProtocolFactory_.getProtocol(getOutputTransport());
      }

      try {
        TRpcConnectionContext server_ctx = new TRpcConnectionContext(trans_, inProt, outProt);
        processorFactory_.getProcessor(inTrans).process(inProt, outProt, server_ctx);
        responseReady();
        return;
      } catch (TException te) {
        LOGGER.warn("Exception while invoking!", te);
      } catch (Exception e) {
        LOGGER.error("Unexpected exception while invoking!", e);
      }

      // Exceptions will fall through to here.
      // We can clear out the buffer here because it is no longer being used
      readBufferBytesAllocated.addAndGet(-buffer_.capacity());
      buffer_ = null;
      state_ = AWAITING_CLOSE;
      changeSelectInterests();
    }

    /**
     * Wrap the read buffer in a memory-based transport so a processor can read the data it needs to
     * handle an invocation.
     */
    public synchronized TTransport getInputTransport() {
      return inputTransportFactory_.getTransport(
          new TIOStreamTransport(new ByteArrayInputStream(buffer_.array())));
    }

    /** Get the transport that should be used by the invoker for responding. */
    private TTransport getOutputTransport() {
      response_ = new TByteArrayOutputStream();
      return outputTransportFactory_.getTransport(new TIOStreamTransport(response_));
    }

    /** Get a transport that works for reading and responding */
    private TTransport getInputOutputTransport() {
      response_ = new TByteArrayOutputStream();
      return inputTransportFactory_.getTransport(
          new TIOStreamTransport(new ByteArrayInputStream(buffer_.array()), response_));
    }

    /**
     * Perform a read into buffer.
     *
     * @return true if the read succeeded, false if there was an error or the connection closed.
     */
    private boolean internalRead() {
      try {
        if (trans_.read(buffer_) < 0) {
          return false;
        }
        return true;
      } catch (IOException e) {
        LOGGER.warn("Got an IOException in internalRead!", e);
        return false;
      }
    }
  } // FrameBuffer

  public static class ServerOverloadedException extends Exception {
    private static final long serialVersionUID = 1L;

    public ServerOverloadedException() {}

    public ServerOverloadedException(Throwable th) {
      super(th);
    }
  }

  public static class Options {
    public int minWorkerThreads = 8;
    public int maxWorkerThreads = Integer.MAX_VALUE;
    public boolean forceSingleThreaded = false;
    public int queueSize = Integer.MAX_VALUE;
    public long timeout = 2000;
    public long maxReadBufferBytes = Long.MAX_VALUE;

    public Options() {}

    public void validate() {
      if (maxReadBufferBytes <= 1024) {
        throw new IllegalArgumentException("You must allocate at least 1KB to the read buffer.");
      }
    }
  }
}
