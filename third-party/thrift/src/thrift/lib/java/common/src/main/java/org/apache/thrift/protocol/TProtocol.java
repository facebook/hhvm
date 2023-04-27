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

package org.apache.thrift.protocol;

import com.facebook.thrift.compatibility.Utf8CodingErrorAction;
import com.facebook.thrift.util.Utf8Util;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import java.util.Map;
import org.apache.thrift.TException;
import org.apache.thrift.transport.TTransport;

/** Protocol interface definition. */
public abstract class TProtocol {

  /** Prevent direct instantiation */
  @SuppressWarnings("unused")
  private TProtocol() {}

  /** Transport */
  protected TTransport trans_;

  /** Constructor */
  protected TProtocol(TTransport trans) {
    trans_ = trans;
  }

  /** Transport accessor */
  public TTransport getTransport() {
    return trans_;
  }

  /** Writing methods. */
  public abstract void writeMessageBegin(TMessage message) throws TException;

  public abstract void writeMessageEnd() throws TException;

  public abstract void writeStructBegin(TStruct struct) throws TException;

  public abstract void writeStructEnd() throws TException;

  public abstract void writeFieldBegin(TField field) throws TException;

  public abstract void writeFieldEnd() throws TException;

  public abstract void writeFieldStop() throws TException;

  public abstract void writeMapBegin(TMap map) throws TException;

  public abstract void writeMapEnd() throws TException;

  public abstract void writeListBegin(TList list) throws TException;

  public abstract void writeListEnd() throws TException;

  public abstract void writeSetBegin(TSet set) throws TException;

  public abstract void writeSetEnd() throws TException;

  public abstract void writeBool(boolean b) throws TException;

  public abstract void writeByte(byte b) throws TException;

  public abstract void writeI16(short i16) throws TException;

  public abstract void writeI32(int i32) throws TException;

  public abstract void writeI64(long i64) throws TException;

  public abstract void writeFloat(float flt) throws TException;

  public abstract void writeDouble(double dub) throws TException;

  public abstract void writeString(String str) throws TException;

  public abstract void writeBinary(ByteBuffer buf) throws TException;

  /** Reading methods. */
  public abstract TMessage readMessageBegin() throws TException;

  public abstract void readMessageEnd() throws TException;

  public TStruct readStructBegin(
      Map<String, Integer> namesToIds,
      Map<String, Integer> thriftNamesToIds,
      Map<Integer, TField> fieldMetadata)
      throws TException {
    return readStructBegin();
  }

  public abstract TStruct readStructBegin() throws TException;

  public abstract void readStructEnd() throws TException;

  public abstract TField readFieldBegin() throws TException;

  public abstract void readFieldEnd() throws TException;

  /**
   * Returned TMap may have size of -1, expecting the user to peek into the map, one element at a
   * time, with peekMap method.
   */
  public abstract TMap readMapBegin() throws TException;

  public boolean peekMap() throws TException {
    throw new TException("Peeking into a map not supported, likely because it's sized");
  }

  public abstract void readMapEnd() throws TException;

  /**
   * Returned TList may have size of -1, expecting the user to peek into the list, one element at a
   * time, with peekList method.
   */
  public abstract TList readListBegin() throws TException;

  public boolean peekList() throws TException {
    throw new TException("Peeking into a list not supported, likely because it's sized");
  }

  public abstract void readListEnd() throws TException;

  /**
   * Returned TSet may have size of -1, expecting the user to peek into the set, one element at a
   * time, with peekSet method.
   */
  public abstract TSet readSetBegin() throws TException;

  public boolean peekSet() throws TException {
    throw new TException("Peeking into a set not supported, likely because it's sized");
  }

  public abstract void readSetEnd() throws TException;

  public abstract boolean readBool() throws TException;

  public abstract byte readByte() throws TException;

  public abstract short readI16() throws TException;

  public abstract int readI32() throws TException;

  public abstract long readI64() throws TException;

  public abstract float readFloat() throws TException;

  public abstract double readDouble() throws TException;

  public abstract String readString() throws TException;

  public abstract ByteBuffer readBinary() throws TException;

  /**
   * Skips a binary or string field
   *
   * @throws TException
   */
  public void skipBinary() throws TException {
    readBinary();
  }

  /**
   * Reset any internal state back to a blank slate. This method only needs to be implemented for
   * stateful protocols.
   */
  public void reset() {}

  /** Return the minimum size of a type */
  protected int typeMinimumSize(byte type) {
    return 1;
  }

  protected void ensureContainerHasEnough(int size, byte type) {
    int minimumExpected = size * typeMinimumSize(type);
    ensureHasEnoughBytes(minimumExpected);
  }

  protected void ensureMapHasEnough(int size, byte keyType, byte valueType) {
    int minimumExpected = size * (typeMinimumSize(keyType) + typeMinimumSize(valueType));
    ensureHasEnoughBytes(minimumExpected);
  }

  private void ensureHasEnoughBytes(int minimumExpected) {
    int remaining = trans_.getBytesRemainingInBuffer();
    if (remaining < 0) {
      return; // Some transport are not buffered
    }
    if (remaining < minimumExpected) {
      throw new TProtocolException(
          TProtocolException.INVALID_DATA,
          "Not enough bytes to read the entire message, the data appears to be truncated");
    }
  }

  protected String readStringReportIfInvalid() {
    return Utf8Util.readStringReportIfInvalid(Unpooled.wrappedBuffer(readBinary()));
  }

  public String readString(Utf8CodingErrorAction action) {
    if (action == Utf8CodingErrorAction.REPORT) {
      return readStringReportIfInvalid();
    }
    throw new RuntimeException("Malformed UTF8 action " + action + " not implemented");
  }
}
