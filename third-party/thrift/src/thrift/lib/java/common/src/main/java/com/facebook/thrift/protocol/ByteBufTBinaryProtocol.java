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

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TProtocolException;
import org.apache.thrift.protocol.TSet;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;

public final class ByteBufTBinaryProtocol extends ByteBufTProtocol {
  private static final int VERSION_MASK = 0xffff0000;
  private static final int VERSION_1 = 0x80010000;
  private static final TStruct ANONYMOUS_STRUCT = new TStruct();

  private boolean strictRead;
  private boolean strictWrite;

  public void wrap(boolean strictRead, boolean strictWrite, ByteBuf byteBuf) {
    super.wrap(byteBuf);
    this.strictRead = strictRead;
    this.strictWrite = strictWrite;
  }

  public void wrap(ByteBuf byteBuf) {
    super.wrap(byteBuf);
    this.strictRead = true;
    this.strictWrite = true;
  }

  @Override
  public void writeMessageBegin(TMessage message) throws TException {
    if (strictWrite) {
      final int version = VERSION_1 | message.type;
      writeI32(version);
      writeString(message.name);
      writeI32(message.seqid);
    } else {
      writeString(message.name);
      writeByte(message.type);
      writeI32(message.seqid);
    }
  }

  @Override
  public void writeMessageEnd() throws TException {}

  @Override
  public void writeStructBegin(TStruct struct) throws TException {}

  @Override
  public void writeStructEnd() throws TException {}

  @Override
  public void writeFieldBegin(TField field) throws TException {
    writeByte(field.type);
    writeI16(field.id);
  }

  @Override
  public void writeFieldEnd() throws TException {}

  @Override
  public void writeFieldStop() throws TException {
    writeByte(TType.STOP);
  }

  @Override
  public void writeMapBegin(TMap map) throws TException {
    writeByte(map.keyType);
    writeByte(map.valueType);
    writeI32(map.size);
  }

  @Override
  public void writeMapEnd() throws TException {}

  @Override
  public void writeListBegin(TList list) throws TException {
    writeByte(list.elemType);
    writeI32(list.size);
  }

  @Override
  public void writeListEnd() throws TException {}

  @Override
  public void writeSetBegin(TSet set) throws TException {
    writeByte(set.elemType);
    writeI32(set.size);
  }

  @Override
  public void writeSetEnd() throws TException {}

  @Override
  public void writeBool(boolean b) throws TException {
    byteBuf.writeBoolean(b);
  }

  @Override
  public void writeByte(byte b) throws TException {
    byteBuf.writeByte(b);
  }

  @Override
  public void writeI16(short i16) throws TException {
    byteBuf.writeShort(i16);
  }

  @Override
  public void writeI32(int i32) throws TException {
    byteBuf.writeInt(i32);
  }

  @Override
  public void writeI64(long i64) throws TException {
    byteBuf.writeLong(i64);
  }

  @Override
  public void writeDouble(double dub) throws TException {
    byteBuf.writeDouble(dub);
  }

  @Override
  public void writeFloat(float flt) throws TException {
    byteBuf.writeFloat(flt);
  }

  @Override
  public void writeString(String str) throws TException {
    final int length = ByteBufUtil.utf8Bytes(str);
    writeI32(length);
    ByteBufUtil.reserveAndWriteUtf8(byteBuf, str, length);
  }

  @Override
  public void writeBinary(ByteBuffer buf) throws TException {
    writeBinaryAsByteBuf(Unpooled.wrappedBuffer(buf));
  }

  @Override
  public void writeBinaryAsByteBuf(ByteBuf bin) throws TException {
    getWritableBinaryAsByteBuf(bin.readableBytes()).writeBytes(bin);
  }

  @Override
  public ByteBuf getWritableBinaryAsByteBuf(int size) throws TException {
    writeI32(size);
    return byteBuf;
  }

  @Override
  public TMessage readMessageBegin() throws TException {
    final int size = readI32();
    if (size < 0) {
      final int version = size & VERSION_MASK;
      if (version != VERSION_1) {
        throw new TProtocolException(
            TProtocolException.BAD_VERSION, "Bad version in readMessageBegin");
      }
      return new TMessage(readString(), (byte) (size & 0x000000ff), readI32());
    } else {
      if (strictRead) {
        throw new TProtocolException(
            TProtocolException.BAD_VERSION, "Missing version in readMessageBegin, old client?");
      }
      return new TMessage(readStringBody(size), readByte(), readI32());
    }
  }

  @Override
  public void readMessageEnd() throws TException {}

  @Override
  public TStruct readStructBegin() throws TException {
    return ANONYMOUS_STRUCT;
  }

  @Override
  public void readStructEnd() throws TException {}

  @Override
  public TField readFieldBegin() throws TException {
    final byte type = readByte();
    final short id = type == TType.STOP ? 0 : readI16();
    return new TField("", type, id);
  }

  @Override
  public void readFieldEnd() throws TException {}

  @Override
  public TMap readMapBegin() throws TException {
    final byte keyType = readByte();
    final byte valueType = readByte();
    final int size = readI32();
    return new TMap(keyType, valueType, size);
  }

  @Override
  public void readMapEnd() throws TException {}

  @Override
  public TList readListBegin() throws TException {
    final byte type = readByte();
    final int size = readI32();
    return new TList(type, size);
  }

  @Override
  public void readListEnd() throws TException {}

  @Override
  public TSet readSetBegin() throws TException {
    final byte type = readByte();
    final int size = readI32();
    return new TSet(type, size);
  }

  @Override
  public void readSetEnd() throws TException {}

  @Override
  public boolean readBool() throws TException {
    return byteBuf.readBoolean();
  }

  @Override
  public byte readByte() throws TException {
    return byteBuf.readByte();
  }

  @Override
  public short readI16() throws TException {
    return byteBuf.readShort();
  }

  @Override
  public int readI32() throws TException {
    return byteBuf.readInt();
  }

  @Override
  public long readI64() throws TException {
    return byteBuf.readLong();
  }

  @Override
  public double readDouble() throws TException {
    return byteBuf.readDouble();
  }

  public float readFloat() throws TException {
    return byteBuf.readFloat();
  }

  @Override
  public String readString() throws TException {
    final int size = readI32();
    return readStringBody(size);
  }

  private String readStringBody(int size) throws TException {
    return byteBuf.readSlice(size).toString(StandardCharsets.UTF_8);
  }

  @Override
  public ByteBuffer readBinary() throws TException {
    final int size = readI32();
    final ByteBuffer byteBuffer;
    byte[] bytes = new byte[size];
    byteBuffer = ByteBuffer.wrap(bytes);
    byteBuf.readBytes(bytes);
    return byteBuffer;
  }

  @Override
  public ByteBuf readBinaryAsSlice() throws TException {
    int size = readI32();
    return byteBuf.readSlice(size);
  }
}
