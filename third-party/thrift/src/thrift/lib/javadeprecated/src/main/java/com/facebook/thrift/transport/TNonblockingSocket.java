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

package com.facebook.thrift.transport;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;

/** Socket implementation of the TTransport interface. To be commented soon! */
public class TNonblockingSocket extends TNonblockingTransport implements TSocketIf {

  /** Host and port if passed in, used for lazy non-blocking connect. */
  private final SocketAddress socketAddress;

  private SocketChannel socketChannel = null;

  /** Wrapped Socket object */
  private Socket socket_ = null;

  /** Socket timeout */
  private int timeout_ = 0;

  public TNonblockingSocket(String host, int port) throws TTransportException, IOException {
    this(host, port, 0);
  }

  /** Create a new nonblocking socket transport that will be connected to host:port. */
  public TNonblockingSocket(String host, int port, int timeout)
      throws TTransportException, IOException {
    this(SocketChannel.open(), timeout, new InetSocketAddress(host, port));
  }

  /**
   * Constructor that takes an already created socket.
   *
   * @param socketChannel Already created SocketChannel object
   * @throws TTransportException if there is an error setting up the streams
   */
  public TNonblockingSocket(SocketChannel socketChannel) throws TTransportException, IOException {
    this(socketChannel, 0, null);
    if (!socketChannel.isConnected()) {
      throw new IOException("Socket must already be connected");
    }
  }

  private TNonblockingSocket(SocketChannel socketChannel, int timeout, SocketAddress socketAddress)
      throws TTransportException, IOException {

    try {
      // make it a nonblocking channel
      socketChannel.configureBlocking(false);
    } catch (IOException e) {
      throw new TTransportException(e);
    }

    this.socketAddress = socketAddress;
    this.socketChannel = socketChannel;
    this.socket_ = socketChannel.socket();
    try {
      socket_.setSoLinger(false, 0);
      socket_.setTcpNoDelay(true);
    } catch (SocketException sx) {
      sx.printStackTrace();
    }
  }

  /**
   * Register this socket with the specified selector for both read and write operations.
   *
   * @return the selection key for this socket.
   */
  public SelectionKey registerSelector(Selector selector, int interests) throws IOException {
    // Register the new SocketChannel with our Selector, indicating
    // we'd like to be notified when there's data waiting to be read
    return socketChannel.register(selector, interests);
  }

  /** Initializes the socket object */
  private void initSocket() {
    socket_ = new Socket();
    try {
      socket_.setSoLinger(false, 0);
      socket_.setTcpNoDelay(true);
      socket_.setSoTimeout(timeout_);
    } catch (SocketException sx) {
      sx.printStackTrace();
    }
  }

  /**
   * Sets the socket timeout
   *
   * @param timeout Milliseconds timeout
   */
  public void setTimeout(int timeout) {
    timeout_ = timeout;
    try {
      socket_.setSoTimeout(timeout);
    } catch (SocketException sx) {
      sx.printStackTrace();
    }
  }

  /** Returns a reference to the underlying socket. */
  public Socket getSocket() {
    if (socket_ == null) {
      initSocket();
    }
    return socket_;
  }

  /** Checks whether the socket is connected. */
  public boolean isOpen() {
    if (socket_ == null) {
      return false;
    }
    return socket_.isConnected();
  }

  /** Connects the socket, creating a new socket object if necessary. */
  public void open() throws TTransportException {
    throw new RuntimeException("Not implemented yet");
  }

  /** Perform a nonblocking read into buffer. */
  public int read(ByteBuffer buffer) throws IOException {
    return socketChannel.read(buffer);
  }

  /** Reads from the underlying input stream if not null. */
  public int read(byte[] buf, int off, int len) throws TTransportException {
    if ((socketChannel.validOps() & SelectionKey.OP_READ) != SelectionKey.OP_READ) {
      throw new TTransportException(
          TTransportException.NOT_OPEN, "Cannot read from write-only socket channel");
    }
    try {
      return socketChannel.read(ByteBuffer.wrap(buf, off, len));
    } catch (IOException iox) {
      throw new TTransportException(TTransportException.UNKNOWN, iox);
    }
  }

  /** Perform a nonblocking write of the data in buffer; */
  public int write(ByteBuffer buffer) throws IOException {
    return socketChannel.write(buffer);
  }

  /** Writes to the underlying output stream if not null. */
  public void write(byte[] buf, int off, int len) throws TTransportException {
    if ((socketChannel.validOps() & SelectionKey.OP_WRITE) != SelectionKey.OP_WRITE) {
      throw new TTransportException(
          TTransportException.NOT_OPEN, "Cannot write to write-only socket channel");
    }
    try {
      socketChannel.write(ByteBuffer.wrap(buf, off, len));
    } catch (IOException iox) {
      throw new TTransportException(TTransportException.UNKNOWN, iox);
    }
  }

  /** Flushes the underlying output stream if not null. */
  public void flush() throws TTransportException {
    // Not supported by SocketChannel.
  }

  /** Closes the socket. */
  public void close() {
    try {
      socketChannel.close();
    } catch (IOException e) {
      // silently ignore.
    }
  }

  /** {@inheritDoc} */
  public boolean startConnect() throws IOException {
    return socketChannel.connect(socketAddress);
  }

  /** {@inheritDoc} */
  public boolean finishConnect() throws IOException {
    return socketChannel.finishConnect();
  }
}
