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

package com.facebook.thrift.protocol;

import com.facebook.thrift.util.IntrinsicDefaults;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TSet;
import org.apache.thrift.protocol.TStruct;

/**
 * This protocol is used in generated thrift objects to create an adapted instance from intrinsic
 * default values. An instance of this class is given to the type adapter and an intrinsic default
 * adapted type is created.
 *
 * <p>It is a singleton class and only read primitive type methods are implemented.
 */
public class ByteBufIntrinsicDefaultTProtocol extends ByteBufTProtocol {
  private static final ByteBufIntrinsicDefaultTProtocol INSTANCE =
      new ByteBufIntrinsicDefaultTProtocol();

  private ByteBufIntrinsicDefaultTProtocol() {}

  public static ByteBufIntrinsicDefaultTProtocol getInstance() {
    return INSTANCE;
  }

  @Override
  public void writeBinaryAsByteBuf(ByteBuf bin) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getWritableBinaryAsByteBuf(int size) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf readBinaryAsSlice() throws TException {
    return Unpooled.EMPTY_BUFFER;
  }

  @Override
  public void writeMessageBegin(TMessage message) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeMessageEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeStructBegin(TStruct struct) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeStructEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeFieldBegin(TField field) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeFieldEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeFieldStop() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeMapBegin(TMap map) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeMapEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeListEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {}

  @Override
  public void writeSetEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeBool(boolean b) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeByte(byte b) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeI16(short i16) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeI32(int i32) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeI64(long i64) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeFloat(float flt) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeDouble(double dub) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeString(String str) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void writeBinary(ByteBuffer buf) throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public TMessage readMessageBegin() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void readMessageEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  private static final TStruct ANONYMOUS_STRUCT = new TStruct();

  @Override
  public TStruct readStructBegin() throws TException {
    return ANONYMOUS_STRUCT;
  }

  @Override
  public void readStructEnd() throws TException {}

  @Override
  public TField readFieldBegin() throws TException {
    return new TField();
  }

  @Override
  public void readFieldEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public TMap readMapBegin() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void readMapEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public TList readListBegin() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void readListEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public TSet readSetBegin() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void readSetEnd() throws TException {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean readBool() throws TException {
    return IntrinsicDefaults.defaultBoolean();
  }

  @Override
  public byte readByte() throws TException {
    return IntrinsicDefaults.defaultByte();
  }

  @Override
  public short readI16() throws TException {
    return IntrinsicDefaults.defaultShort();
  }

  @Override
  public int readI32() throws TException {
    return IntrinsicDefaults.defaultInt();
  }

  @Override
  public long readI64() throws TException {
    return IntrinsicDefaults.defaultLong();
  }

  @Override
  public float readFloat() throws TException {
    return IntrinsicDefaults.defaultFloat();
  }

  @Override
  public double readDouble() throws TException {
    return IntrinsicDefaults.defaultDouble();
  }

  @Override
  public String readString() throws TException {
    return IntrinsicDefaults.defaultString();
  }

  @Override
  public ByteBuffer readBinary() throws TException {
    return IntrinsicDefaults.defaultByteBuffer();
  }
}
