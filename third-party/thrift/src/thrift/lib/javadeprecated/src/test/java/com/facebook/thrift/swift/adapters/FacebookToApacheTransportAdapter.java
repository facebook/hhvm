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

package com.facebook.thrift.swift.adapters;

import org.apache.thrift.transport.TTransportException;

public class FacebookToApacheTransportAdapter extends org.apache.thrift.transport.TTransport {
  protected final com.facebook.thrift.transport.TTransport facebookTransport;

  public FacebookToApacheTransportAdapter(
      com.facebook.thrift.transport.TTransport facebookTransport) {
    this.facebookTransport = facebookTransport;
  }

  @Override
  public void close() {
    facebookTransport.close();
  }

  @Override
  public void consumeBuffer(int len) {
    facebookTransport.consumeBuffer(len);
  }

  @Override
  public void flush() throws TTransportException {
    try {
      facebookTransport.flush();
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public byte[] getBuffer() {
    return facebookTransport.getBuffer();
  }

  @Override
  public int getBufferPosition() {
    return facebookTransport.getBufferPosition();
  }

  @Override
  public int getBytesRemainingInBuffer() {
    return facebookTransport.getBytesRemainingInBuffer();
  }

  @Override
  public boolean isOpen() {
    return facebookTransport.isOpen();
  }

  @Override
  public void open() throws TTransportException {
    try {
      facebookTransport.open();
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public boolean peek() {
    return facebookTransport.peek();
  }

  @Override
  public int read(byte[] buf, int off, int len) throws TTransportException {
    try {
      return facebookTransport.read(buf, off, len);
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public int readAll(byte[] buf, int off, int len) throws TTransportException {
    try {
      return facebookTransport.readAll(buf, off, len);
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public void write(byte[] buf) throws TTransportException {
    try {
      facebookTransport.write(buf);
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public void write(byte[] buf, int off, int len) throws TTransportException {
    try {
      facebookTransport.write(buf, off, len);
    } catch (com.facebook.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }
}
