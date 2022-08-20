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
import java.util.ArrayDeque;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TProtocolException;
import org.apache.thrift.protocol.TSet;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;

public final class ByteBufTCompactProtocol extends ByteBufTProtocol {

  private static final TStruct ANONYMOUS_STRUCT = new TStruct("");
  private static final TField TSTOP = new TField("", TType.STOP, (short) 0);

  private static final byte PROTOCOL_ID = (byte) 0x82;
  private static final byte VERSION = 2;
  private static final byte VERSION_MASK = 0x1f; // 0001 1111
  private static final byte TYPE_MASK = (byte) 0xE0; // 1110 0000
  private static final byte TYPE_BITS = 0x07; // 0000 0111
  private static final int TYPE_SHIFT_AMOUNT = 5;

  // TCompact Types
  private static final byte BOOLEAN_TRUE = 0x01;
  private static final byte BOOLEAN_FALSE = 0x02;
  private static final byte BYTE = 0x03;
  private static final byte I16 = 0x04;
  private static final byte I32 = 0x05;
  private static final byte I64 = 0x06;
  private static final byte DOUBLE = 0x07;
  private static final byte BINARY = 0x08;
  private static final byte LIST = 0x09;
  private static final byte SET = 0x0A;
  private static final byte MAP = 0x0B;
  private static final byte STRUCT = 0x0C;
  private static final byte FLOAT = 0x0D;

  private static final byte[] ttypeToCompactType = new byte[20];
  private static final byte[] compactTypeToTType = new byte[14];

  static {
    ttypeToCompactType[TType.STOP] = TType.STOP;
    ttypeToCompactType[TType.BOOL] = BOOLEAN_TRUE;
    ttypeToCompactType[TType.BYTE] = BYTE;
    ttypeToCompactType[TType.I16] = I16;
    ttypeToCompactType[TType.I32] = I32;
    ttypeToCompactType[TType.I64] = I64;
    ttypeToCompactType[TType.DOUBLE] = DOUBLE;
    ttypeToCompactType[TType.STRING] = BINARY;
    ttypeToCompactType[TType.LIST] = LIST;
    ttypeToCompactType[TType.SET] = SET;
    ttypeToCompactType[TType.MAP] = MAP;
    ttypeToCompactType[TType.STRUCT] = STRUCT;
    ttypeToCompactType[TType.FLOAT] = FLOAT;

    compactTypeToTType[TType.STOP] = TType.STOP;
    compactTypeToTType[BOOLEAN_TRUE] = TType.BOOL;
    compactTypeToTType[BOOLEAN_FALSE] = TType.BOOL;
    compactTypeToTType[BYTE] = TType.BYTE;
    compactTypeToTType[I16] = TType.I16;
    compactTypeToTType[I32] = TType.I32;
    compactTypeToTType[I64] = TType.I64;
    compactTypeToTType[DOUBLE] = TType.DOUBLE;
    compactTypeToTType[BINARY] = TType.STRING;
    compactTypeToTType[LIST] = TType.LIST;
    compactTypeToTType[SET] = TType.SET;
    compactTypeToTType[MAP] = TType.MAP;
    compactTypeToTType[STRUCT] = TType.STRUCT;
    compactTypeToTType[FLOAT] = TType.FLOAT;
  }

  /**
   * Used to keep track of the last field for the current and previous structs, so we can do the
   * delta stuff.
   */
  private final ArrayDeque<Short> lastField_ = new ArrayDeque<>();

  private short lastFieldId_ = 0;

  /**
   * If we encounter a boolean field begin, save the TField here so it can have the value
   * incorporated.
   */
  private TField booleanField_ = null;

  /**
   * If we read a field header, and it's a boolean field, save the boolean value here so that
   * readBool can use it.
   */
  private Boolean boolValue_ = null;

  @Override
  public void reset() {
    lastField_.clear();
    lastFieldId_ = 0;
  }

  //
  // Public Writing methods.
  //

  /**
   * Write a message header to the wire. Compact Protocol messages contain the protocol version so
   * we can migrate forwards in the future if need be.
   */
  public void writeMessageBegin(TMessage message) throws TException {
    byteBuf.writeByte(PROTOCOL_ID);
    byteBuf.writeByte((VERSION & VERSION_MASK) | ((message.type << TYPE_SHIFT_AMOUNT) & TYPE_MASK));
    writeVarInt32(message.seqid);
    writeString(message.name);
  }

  /**
   * Write a struct begin. This doesn't actually put anything on the wire. We use it as an
   * opportunity to put special placeholder markers on the field stack so we can get the field id
   * deltas correct.
   */
  public void writeStructBegin(TStruct struct) throws TException {
    lastField_.push(lastFieldId_);
    lastFieldId_ = 0;
  }

  /**
   * Write a struct end. This doesn't actually put anything on the wire. We use this as an
   * opportunity to pop the last field from the current struct off of the field stack.
   */
  public void writeStructEnd() throws TException {
    lastFieldId_ = lastField_.pop();
  }

  /**
   * Write a field header containing the field id and field type. If the difference between the
   * current field id and the last one is small (< 15), then the field id will be encoded in the 4
   * MSB as a delta. Otherwise, the field id will follow the type header as a zigzag varint.
   */
  public void writeFieldBegin(TField field) throws TException {
    if (field.type == TType.BOOL) {
      // we want to possibly include the value, so we'll wait.
      booleanField_ = field;
    } else {
      writeFieldBeginInternal(field, (byte) -1);
    }
  }

  /**
   * The workhorse of writeFieldBegin. It has the option of doing a 'type override' of the type
   * header. This is used specifically in the boolean field case.
   */
  private void writeFieldBeginInternal(TField field, byte typeOverride) throws TException {
    // short lastField = lastField_.pop();

    // if there's a type override, use that.
    byte typeToWrite = typeOverride == -1 ? getCompactType(field.type) : typeOverride;

    // check if we can use delta encoding for the field id
    if (field.id > lastFieldId_ && field.id - lastFieldId_ <= 15) {
      // write them together
      byteBuf.writeByte((field.id - lastFieldId_) << 4 | typeToWrite);
    } else {
      // write them separate
      byteBuf.writeByte(typeToWrite);
      writeI16(field.id);
    }

    lastFieldId_ = field.id;
    // lastField_.push(field.id);
  }

  /** Write the STOP symbol so we know there are no more fields in this struct. */
  public void writeFieldStop() throws TException {
    byteBuf.writeByte(TType.STOP);
  }

  /**
   * Write a map header. If the map is empty, omit the key and value type headers, as we don't need
   * any additional information to skip it.
   */
  public void writeMapBegin(TMap map) throws TException {
    if (map.size == 0) {
      byteBuf.writeByte(0);
    } else {
      writeVarInt32(map.size);
      byteBuf.writeByte(getCompactType(map.keyType) << 4 | getCompactType(map.valueType));
    }
  }

  /** Write a list header. */
  public void writeListBegin(TList list) throws TException {
    writeCollectionBegin(list.elemType, list.size);
  }

  /** Write a set header. */
  public void writeSetBegin(TSet set) throws TException {
    writeCollectionBegin(set.elemType, set.size);
  }

  /**
   * Write a boolean value. Potentially, this could be a boolean field, in which case the field
   * header info isn't written yet. If so, decide what the right type header is for the value and
   * then write the field header. Otherwise, write a single byte.
   */
  public void writeBool(boolean b) throws TException {
    if (booleanField_ != null) {
      // we haven't written the field header yet
      writeFieldBeginInternal(booleanField_, b ? BOOLEAN_TRUE : BOOLEAN_FALSE);
      booleanField_ = null;
    } else {
      // we're not part of a field, so just write the value.
      byteBuf.writeByte(b ? BOOLEAN_TRUE : BOOLEAN_FALSE);
    }
  }

  public void writeByte(byte b) throws TException {
    byteBuf.writeByte(b);
  }

  public void writeI16(short i16) throws TException {
    writeVarInt32(intToZigZag(i16));
  }

  public void writeI32(int i32) throws TException {
    writeVarInt32(intToZigZag(i32));
  }

  public void writeI64(long i64) throws TException {
    writeVarInt64(longToZigzag(i64));
  }

  public void writeString(String str) throws TException {
    final int length = ByteBufUtil.utf8Bytes(str);
    writeVarInt32(length);
    if (length > 0) {
      ByteBufUtil.reserveAndWriteUtf8(byteBuf, str, length);
    }
  }

  @Override
  public void writeFloat(float flt) throws TException {
    byteBuf.writeFloat(flt);
  }

  @Override
  public void writeDouble(double dub) throws TException {
    byteBuf.writeDouble(dub);
  }

  //
  // These methods are called by structs, but don't actually have any wire
  // output or purpose.
  //

  public void writeMessageEnd() throws TException {}

  public void writeMapEnd() throws TException {}

  public void writeListEnd() throws TException {}

  public void writeSetEnd() throws TException {}

  public void writeFieldEnd() throws TException {}

  //
  // Internal writing methods
  //

  /**
   * Abstract method for writing the start of lists and sets. List and sets on the wire differ only
   * by the type indicator.
   */
  protected void writeCollectionBegin(byte elemType, int size) throws TException {
    if (size <= 14) {
      byteBuf.writeByte(size << 4 | getCompactType(elemType));
    } else {
      byteBuf.writeByte(0xf0 | getCompactType(elemType));
      writeVarInt32(size);
    }
  }

  //
  // Reading methods.
  //

  /** Read a message header. */
  public TMessage readMessageBegin() throws TException {
    byte protocolId = readByte();
    if (protocolId != PROTOCOL_ID) {
      throw new TProtocolException(
          "Expected protocol id "
              + Integer.toHexString(PROTOCOL_ID)
              + " but got "
              + Integer.toHexString(protocolId));
    }
    byte versionAndType = readByte();
    byte version = (byte) (versionAndType & VERSION_MASK);
    if (version != VERSION) {
      throw new TProtocolException("Expected version " + VERSION + " but got " + version);
    }
    byte type = (byte) ((versionAndType >> TYPE_SHIFT_AMOUNT) & TYPE_BITS);
    int seqid = readVarInt32();
    String messageName = readString();
    return new TMessage(messageName, type, seqid);
  }

  /**
   * Read a struct begin. There's nothing on the wire for this, but it is our opportunity to push a
   * new struct begin marker onto the field stack.
   */
  public TStruct readStructBegin() throws TException {
    lastField_.push(lastFieldId_);
    lastFieldId_ = 0;
    return ANONYMOUS_STRUCT;
  }

  /**
   * Doesn't actually consume any wire data, just removes the last field for this struct from the
   * field stack.
   */
  public void readStructEnd() throws TException {
    // consume the last field we read off the wire.
    lastFieldId_ = lastField_.pop();
  }

  /** Read a field header off the wire. */
  public TField readFieldBegin() throws TException {
    byte type = readByte();

    // if it's a stop, then we can return immediately, as the struct is over.
    if (type == TType.STOP) {
      return TSTOP;
    }

    short fieldId;

    // mask off the 4 MSB of the type header. it could contain a field id delta.
    short modifier = (short) ((type & 0xf0) >> 4);
    if (modifier == 0) {
      // not a delta. look ahead for the zigzag varint field id.
      fieldId = readI16();
    } else {
      // has a delta. add the delta to the last read field id.
      fieldId = (short) (lastFieldId_ + modifier);
    }

    TField field = new TField("", getTType((byte) (type & 0x0f)), fieldId);

    // if this happens to be a boolean field, the value is encoded in the type
    if (isBoolType(type)) {
      // save the boolean value in a special instance variable.
      boolValue_ = (byte) (type & 0x0f) == BOOLEAN_TRUE ? Boolean.TRUE : Boolean.FALSE;
    }

    // push the new field onto the field stack so we can keep the deltas going.
    lastFieldId_ = field.id;
    return field;
  }

  /**
   * Read a map header off the wire. If the size is zero, skip reading the key and value type. This
   * means that 0-length maps will yield TMaps without the "correct" types.
   */
  public TMap readMapBegin() throws TException {
    int size = readVarInt32();
    byte keyAndValueType = size == 0 ? 0 : readByte();
    return new TMap(
        getTType((byte) (keyAndValueType >> 4)), getTType((byte) (keyAndValueType & 0xf)), size);
  }

  /**
   * Read a list header off the wire. If the list size is 0-14, the size will be packed into the
   * element type header. If it's a longer list, the 4 MSB of the element type header will be 0xF,
   * and a varint will follow with the true size.
   */
  public TList readListBegin() throws TException {
    byte size_and_type = readByte();
    int size = (size_and_type >> 4) & 0x0f;
    if (size == 15) {
      size = readVarInt32();
    }
    byte type = getTType(size_and_type);
    return new TList(type, size);
  }

  /**
   * Read a set header off the wire. If the set size is 0-14, the size will be packed into the
   * element type header. If it's a longer set, the 4 MSB of the element type header will be 0xF,
   * and a varint will follow with the true size.
   */
  public TSet readSetBegin() throws TException {
    return new TSet(readListBegin());
  }

  @Override
  public byte readByte() throws TException {
    return byteBuf.readByte();
  }

  /**
   * Read a boolean off the wire. If this is a boolean field, the value should already have been
   * read during readFieldBegin, so we'll just consume the pre-stored value. Otherwise, read a byte.
   */
  public boolean readBool() throws TException {
    if (boolValue_ != null) {
      boolean result = boolValue_;
      boolValue_ = null;
      return result;
    }
    return readByte() == BOOLEAN_TRUE;
  }

  /** Read an i16 from the wire as a zigzag varint. */
  public short readI16() throws TException {
    return (short) zigzagToInt(readVarInt32());
  }

  /** Read an i32 from the wire as a zigzag varint. */
  public int readI32() throws TException {
    return zigzagToInt(readVarInt32());
  }

  /** Read an i64 from the wire as a zigzag varint. */
  public long readI64() throws TException {
    return zigzagToLong(readVarInt64());
  }

  @Override
  public float readFloat() throws TException {
    return byteBuf.readFloat();
  }

  @Override
  public double readDouble() throws TException {
    return byteBuf.readDouble();
  }

  /** Reads a byte[] (via readBinary), and then UTF-8 decodes it. */
  public String readString() throws TException {
    final int size = readVarInt32();
    return byteBuf.readCharSequence(size, StandardCharsets.UTF_8).toString();
  }

  /** Read a byte[] from the wire. */
  public ByteBuffer readBinary() throws TException {
    final int size = readVarInt32();
    final ByteBuffer byteBuffer;
    byte[] bytes = new byte[size];
    byteBuffer = ByteBuffer.wrap(bytes);
    byteBuf.readBytes(bytes);
    return byteBuffer;
  }

  // These methods are here for the struct to call, but don't have any wire
  // encoding.
  //
  public void readMessageEnd() throws TException {}

  public void readFieldEnd() throws TException {}

  public void readMapEnd() throws TException {}

  public void readListEnd() throws TException {}

  public void readSetEnd() throws TException {}

  //
  // type testing and converting
  //

  private static boolean isBoolType(byte b) {
    int lowerNibble = b & 0x0f;
    return lowerNibble == BOOLEAN_TRUE || lowerNibble == BOOLEAN_FALSE;
  }

  /** Given a TCompactProtocol Type constant, convert it to its corresponding TType value. */
  private static byte getTType(final byte type) {
    return compactTypeToTType[type & 0x0f];
  }

  /** Given a TType value, find the appropriate TCompactProtocol Type constant. */
  private static byte getCompactType(final byte ttype) {
    return ttypeToCompactType[ttype];
  }

  /**
   * Convert l into a zigzag long. This allows negative numbers to be represented compactly as a
   * varint.
   */
  private static long longToZigzag(long l) {
    return (l << 1) ^ (l >> 63);
  }

  /**
   * Convert n into a zigzag int. This allows negative numbers to be represented compactly as a
   * varint.
   */
  private static int intToZigZag(int n) {
    return (n << 1) ^ (n >> 31);
  }

  /** Convert from zigzag int to int. */
  private static int zigzagToInt(int n) {
    return (n >>> 1) ^ -(n & 1);
  }

  /** Convert from zigzag long to long. */
  private static long zigzagToLong(long n) {
    return (n >>> 1) ^ -(n & 1);
  }

  private int readVarInt32() {
    int result = 0;
    int shift = 0;

    while (true) {
      final byte b = byteBuf.readByte();
      result |= (b & 0x7f) << shift;
      if ((b & 0x80) != 0x80) return result;
      shift += 7;
    }
  }

  private long readVarInt64() {
    int shift = 0;
    long result = 0;

    while (true) {
      final byte b = byteBuf.readByte();
      result |= (long) (b & 0x7f) << shift;
      if ((b & 0x80) != 0x80) return result;
      shift += 7;
    }
  }

  private void writeVarInt64(long n) {
    while (true) {
      if ((n & ~0x7FL) == 0) {
        byteBuf.writeByte((byte) n);
        return;
      } else {
        final byte b = ((byte) ((n & 0x7F) | 0x80));
        byteBuf.writeByte(b);
        n >>>= 7;
      }
    }
  }

  private void writeVarInt32(int n) {
    while (true) {
      if ((n & ~0x7F) == 0) {
        byteBuf.writeByte(n);
        return;
      } else {
        final byte b = (byte) ((n & 0x7F) | 0x80);
        byteBuf.writeByte(b);
        n >>>= 7;
      }
    }
  }

  /** Write a byte array, using a varint for the size. */
  public void writeBinary(ByteBuffer bin) throws TException {
    writeBinaryAsByteBuf(Unpooled.wrappedBuffer(bin));
  }

  public void writeBinaryAsByteBuf(ByteBuf bin) throws TException {
    getWritableBinaryAsByteBuf(bin.readableBytes()).writeBytes(bin);
  }

  public ByteBuf getWritableBinaryAsByteBuf(int size) throws TException {
    writeVarInt32(size);
    return byteBuf;
  }

  public ByteBuf readBinaryAsSlice() {
    final int size = readVarInt32();
    return byteBuf.readSlice(size);
  }
}
