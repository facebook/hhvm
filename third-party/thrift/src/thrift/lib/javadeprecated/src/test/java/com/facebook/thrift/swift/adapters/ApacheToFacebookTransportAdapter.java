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

import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.transport.TTransportException;

public final class ApacheToFacebookTransportAdapter extends TTransport {
  protected final org.apache.thrift.transport.TTransport apacheTransport;

  ApacheToFacebookTransportAdapter(org.apache.thrift.transport.TTransport apacheTransport) {
    this.apacheTransport = apacheTransport;
  }

  @Override
  public void close() {
    apacheTransport.close();
  }

  @Override
  public void consumeBuffer(int len) {
    apacheTransport.consumeBuffer(len);
  }

  @Override
  public void flush() throws TTransportException {
    try {
      apacheTransport.flush();
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public byte[] getBuffer() {
    return apacheTransport.getBuffer();
  }

  @Override
  public int getBufferPosition() {
    return apacheTransport.getBufferPosition();
  }

  @Override
  public int getBytesRemainingInBuffer() {
    return apacheTransport.getBytesRemainingInBuffer();
  }

  @Override
  public boolean isOpen() {
    return apacheTransport.isOpen();
  }

  @Override
  public void open() throws TTransportException {
    try {
      apacheTransport.open();
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public boolean peek() {
    return apacheTransport.peek();
  }

  @Override
  public int read(byte[] buf, int off, int len) throws TTransportException {
    try {
      return apacheTransport.read(buf, off, len);
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public int readAll(byte[] buf, int off, int len) throws TTransportException {
    try {
      return apacheTransport.readAll(buf, off, len);
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public void write(byte[] buf) throws TTransportException {
    try {
      apacheTransport.write(buf);
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }

  @Override
  public void write(byte[] buf, int off, int len) throws TTransportException {
    try {
      apacheTransport.write(buf, off, len);
    } catch (org.apache.thrift.transport.TTransportException e) {
      throw new TTransportException(e.getType(), e.getMessage(), e.getCause());
    }
  }
}
