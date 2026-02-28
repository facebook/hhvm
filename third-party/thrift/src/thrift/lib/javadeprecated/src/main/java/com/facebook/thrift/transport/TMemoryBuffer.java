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

import com.facebook.thrift.TByteArrayOutputStream;
import java.io.UnsupportedEncodingException;

/** Memory buffer-based implementation of the TTransport interface. */
public class TMemoryBuffer extends TTransport {

  /** */
  public TMemoryBuffer(int size) {
    reset(size);
  }

  @Override
  public boolean isOpen() {
    return true;
  }

  @Override
  public void open() {
    /* Do nothing */
  }

  @Override
  public void close() {
    /* Do nothing */
  }

  @Override
  public int read(byte[] buf, int off, int len) throws TTransportException {
    byte[] src = arr_.get();
    int amtToRead = (len > arr_.len() - pos_ ? arr_.len() - pos_ : len);
    if (amtToRead > 0) {
      System.arraycopy(src, pos_, buf, off, amtToRead);
      pos_ += amtToRead;
    }
    return amtToRead;
  }

  @Override
  public void write(byte[] buf, int off, int len) {
    arr_.write(buf, off, len);
  }

  /**
   * Output the contents of the memory buffer as a String, using the supplied encoding
   *
   * @param enc the encoding to use
   * @return the contents of the memory buffer as a String
   */
  public String toString(String enc) throws UnsupportedEncodingException {
    return arr_.toString(enc);
  }

  public String inspect() {
    String buf = "";
    byte[] bytes = arr_.toByteArray();
    for (int i = 0; i < bytes.length; i++) {
      buf += (pos_ == i ? "==>" : "") + Integer.toHexString(bytes[i] & 0xff) + " ";
    }
    return buf;
  }

  // The contents of the buffer
  private TByteArrayOutputStream arr_;

  // Position to read next byte from
  private int pos_;

  public int length() {
    return arr_.len();
  }

  public void reset(int size) {
    arr_ = new TByteArrayOutputStream(size);
    pos_ = 0;
  }

  public byte[] getArray() {
    return arr_.get();
  }

  public int getArrayPos() {
    return pos_;
  }
}
