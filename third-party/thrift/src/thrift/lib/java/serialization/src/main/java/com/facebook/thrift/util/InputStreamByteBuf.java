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

package com.facebook.thrift.util;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import io.netty.util.ByteProcessor;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.channels.GatheringByteChannel;
import java.nio.channels.ScatteringByteChannel;
import java.nio.charset.Charset;
import reactor.core.Exceptions;

/** simple Read-only ByteBuf backed by an InputStream. It only supports the read* methods */
final class InputStreamByteBuf extends ByteBuf {
  private final InputStream inputStream;

  byte[] buffer = new byte[8];

  public InputStreamByteBuf(InputStream inputStream) {
    this.inputStream = inputStream;
  }

  private void read(int length) {
    readIntoBuffer(buffer, 0, length);
  }

  /**
   * reads byte from the underlying input stream into a buffer. Ensures that `length` bytes are read
   * or that the end of the stream has been reached. This method will block until `length` bas been
   * read.
   *
   * @param buffer destination buffer for read bytes
   * @param index index in destination buffer where bytes will be copied
   * @param length number of bytes to be copied into destination buffer
   * @return number of bytes read, should always be equal to length else a RuntimeException should
   *     be thrown.
   */
  private int readIntoBuffer(byte[] buffer, int index, int length) {
    if (index + length > buffer.length) {
      throw Exceptions.propagate(
          new IllegalArgumentException(
              "Attempt read data into byte[] at index: "
                  + index
                  + " with length: "
                  + length
                  + " which is larger than the buffer length: "
                  + buffer.length));
    }

    try {
      int read = 0;
      while (read < length) {
        int result = inputStream.read(buffer, read + index, length - read);
        // Read beyond the end of buffer, throw exception
        if (result == -1) {
          throwIOOBException(read, length);
        }

        read += result;
      }

      return read;
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }
  }

  private void throwIOOBException(int read, int expected) {
    throw Exceptions.propagate(
        new IndexOutOfBoundsException(
            "Attempted to read beyond the end of InputStream, read: "
                + read
                + " bytes, expected:"
                + expected
                + " bytes"));
  }

  // read methods
  @Override
  public boolean readBoolean() {
    return readByte() != 0;
  }

  @Override
  public byte readByte() {
    read(Byte.BYTES);
    return buffer[0];
  }

  @Override
  public short readUnsignedByte() {
    return (short) (readByte() & 0xFF);
  }

  @Override
  public short readShort() {
    read(Short.BYTES);
    return (short) (buffer[0] << 8 | buffer[1] & 0xFF);
  }

  @Override
  public short readShortLE() {
    read(Short.BYTES);
    return (short) (buffer[0] & 0xff | buffer[1] << 8);
  }

  @Override
  public int readUnsignedShort() {
    return readShort() & 0xFFFF;
  }

  @Override
  public int readUnsignedShortLE() {
    return readShortLE() & 0xFFFF;
  }

  @Override
  public int readMedium() {
    int value = readUnsignedMedium();
    if ((value & 0x800000) != 0) {
      value |= 0xff000000;
    }
    return value;
  }

  @Override
  public int readMediumLE() {
    int value = readUnsignedMediumLE();
    if ((value & 0x800000) != 0) {
      value |= 0xff000000;
    }
    return value;
  }

  @Override
  public int readUnsignedMedium() {
    read(3);
    return (buffer[0] & 0xff) << 16 | (buffer[1] & 0xff) << 8 | buffer[2] & 0xff;
  }

  @Override
  public int readUnsignedMediumLE() {
    read(3);
    return buffer[0] & 0xff | (buffer[1] & 0xff) << 8 | (buffer[2] & 0xff) << 16;
  }

  @Override
  public int readInt() {
    read(Integer.BYTES);
    return (buffer[0] & 0xff) << 24
        | (buffer[1] & 0xff) << 16
        | (buffer[2] & 0xff) << 8
        | buffer[3] & 0xff;
  }

  @Override
  public int readIntLE() {
    read(Integer.BYTES);
    return (buffer[0] & 0xff) << 24
        | (buffer[1] & 0xff) << 16
        | (buffer[2] & 0xff) << 8
        | buffer[3] & 0xff;
  }

  @Override
  public long readUnsignedInt() {
    return readInt() & 0xFFFFFFFFL;
  }

  @Override
  public long readUnsignedIntLE() {
    return readIntLE() & 0xFFFFFFFFL;
  }

  @Override
  public long readLong() {
    read(Long.BYTES);
    return ((long) buffer[0] & 0xff) << 56
        | ((long) buffer[1] & 0xff) << 48
        | ((long) buffer[2] & 0xff) << 40
        | ((long) buffer[3] & 0xff) << 32
        | ((long) buffer[4] & 0xff) << 24
        | ((long) buffer[5] & 0xff) << 16
        | ((long) buffer[6] & 0xff) << 8
        | (long) buffer[7] & 0xff;
  }

  @Override
  public long readLongLE() {
    read(Long.BYTES);
    return (long) buffer[0] & 0xff
        | ((long) buffer[1] & 0xff) << 8
        | ((long) buffer[2] & 0xff) << 16
        | ((long) buffer[3] & 0xff) << 24
        | ((long) buffer[4] & 0xff) << 32
        | ((long) buffer[5] & 0xff) << 40
        | ((long) buffer[6] & 0xff) << 48
        | ((long) buffer[7] & 0xff) << 56;
  }

  @Override
  public char readChar() {
    return (char) readShort();
  }

  @Override
  public float readFloat() {
    return Float.intBitsToFloat(readInt());
  }

  @Override
  public double readDouble() {
    return Double.longBitsToDouble(readLong());
  }

  @Override
  public ByteBuf readBytes(int length) {
    ByteBuf dst = Unpooled.buffer(length);
    try {
      for (int i = 0, b; i < length; i++) {
        b = inputStream.read();
        if (b == -1) {
          break;
        }
        dst.writeByte(b);
      }
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }

    return dst;
  }

  @Override
  public ByteBuf readBytes(ByteBuf dst) {
    try {
      int b;
      while ((b = inputStream.read()) != -1) {
        dst.writeByte(b);
      }
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }
    return this;
  }

  @Override
  public ByteBuf readBytes(ByteBuf dst, int length) {
    try {
      for (int i = 0; i < length; i++) {
        dst.writeByte(inputStream.read());
      }
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }
    return this;
  }

  @Override
  public ByteBuf readBytes(ByteBuf dst, int dstIndex, int length) {
    dst.markWriterIndex();
    dst.writerIndex(dstIndex);
    readBytes(dst, length);
    dst.resetWriterIndex();
    return this;
  }

  @Override
  public ByteBuf readBytes(byte[] dst) {
    return readBytes(dst, 0, dst.length);
  }

  @Override
  public ByteBuf readBytes(byte[] dst, int dstIndex, int length) {
    readIntoBuffer(dst, dstIndex, length);
    return this;
  }

  @Override
  public ByteBuf readBytes(ByteBuffer dst) {
    try {
      int b;
      while ((b = inputStream.read()) != -1) {
        dst.putInt(b);
      }
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }

    return this;
  }

  @Override
  public ByteBuf readBytes(OutputStream out, int length) throws IOException {
    for (int i = 0; i < length; i++) {
      out.write(inputStream.read());
    }
    return this;
  }

  @Override
  public int readBytes(GatheringByteChannel out, int length) throws IOException {
    byte[] buffer = new byte[length];
    int i = inputStream.read(buffer, 0, length);
    ByteBuffer byteBuffer = ByteBuffer.wrap(buffer, 0, i);
    return out.write(byteBuffer);
  }

  @Override
  public CharSequence readCharSequence(int length, Charset charset) {
    byte[] buffer = new byte[length];
    int read = readIntoBuffer(buffer, 0, length);
    return new String(buffer, 0, read, charset);
  }

  @Override
  public int readBytes(FileChannel out, long position, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf skipBytes(int length) {
    try {
      inputStream.skip(length);
    } catch (IOException e) {
      throw Exceptions.propagate(e);
    }
    return this;
  }

  @Override
  public ByteBuf readSlice(int length) {
    return Unpooled.wrappedBuffer(readBytes(length));
  }

  @Override
  public ByteBuf readRetainedSlice(int length) {
    return Unpooled.wrappedBuffer(readBytes(length));
  }

  // -- Unsupported methods;
  @Override
  public int capacity() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf capacity(int newCapacity) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int maxCapacity() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBufAllocator alloc() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteOrder order() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf order(ByteOrder endianness) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf unwrap() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isDirect() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isReadOnly() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf asReadOnly() {
    throw new UnsupportedOperationException();
  }

  @Override
  public int readerIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf readerIndex(int readerIndex) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writerIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writerIndex(int writerIndex) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setIndex(int readerIndex, int writerIndex) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int readableBytes() {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writableBytes() {
    throw new UnsupportedOperationException();
  }

  @Override
  public int maxWritableBytes() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isReadable() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isReadable(int size) {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isWritable() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isWritable(int size) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf clear() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf markReaderIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf resetReaderIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf markWriterIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf resetWriterIndex() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf discardReadBytes() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf discardSomeReadBytes() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf ensureWritable(int minWritableBytes) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int ensureWritable(int minWritableBytes, boolean force) {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean getBoolean(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public byte getByte(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public short getUnsignedByte(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public short getShort(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public short getShortLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getUnsignedShort(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getUnsignedShortLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getMedium(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getMediumLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getUnsignedMedium(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getUnsignedMediumLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getInt(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getIntLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public long getUnsignedInt(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public long getUnsignedIntLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public long getLong(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public long getLongLE(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public char getChar(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public float getFloat(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public double getDouble(int index) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, ByteBuf dst) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, ByteBuf dst, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, ByteBuf dst, int dstIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, byte[] dst) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, byte[] dst, int dstIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, ByteBuffer dst) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf getBytes(int index, OutputStream out, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getBytes(int index, GatheringByteChannel out, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getBytes(int index, FileChannel out, long position, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public CharSequence getCharSequence(int index, int length, Charset charset) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBoolean(int index, boolean value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setByte(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setShort(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setShortLE(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setMedium(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setMediumLE(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setInt(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setIntLE(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setLong(int index, long value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setLongLE(int index, long value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setChar(int index, int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setFloat(int index, float value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setDouble(int index, double value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, ByteBuf src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, ByteBuf src, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, ByteBuf src, int srcIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, byte[] src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, byte[] src, int srcIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setBytes(int index, ByteBuffer src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int setBytes(int index, InputStream in, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int setBytes(int index, ScatteringByteChannel in, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int setBytes(int index, FileChannel in, long position, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf setZero(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int setCharSequence(int index, CharSequence sequence, Charset charset) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBoolean(boolean value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeByte(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeShort(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeShortLE(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeMedium(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeMediumLE(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeInt(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeIntLE(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeLong(long value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeLongLE(long value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeChar(int value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeFloat(float value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeDouble(double value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(ByteBuf src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(ByteBuf src, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(ByteBuf src, int srcIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(byte[] src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(byte[] src, int srcIndex, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeBytes(ByteBuffer src) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writeBytes(InputStream in, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writeBytes(ScatteringByteChannel in, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writeBytes(FileChannel in, long position, int length) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf writeZero(int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int writeCharSequence(CharSequence sequence, Charset charset) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int indexOf(int fromIndex, int toIndex, byte value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int bytesBefore(byte value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int bytesBefore(int length, byte value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int bytesBefore(int index, int length, byte value) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int forEachByte(ByteProcessor processor) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int forEachByte(int index, int length, ByteProcessor processor) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int forEachByteDesc(ByteProcessor processor) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int forEachByteDesc(int index, int length, ByteProcessor processor) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf copy() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf copy(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf slice() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf retainedSlice() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf slice(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf retainedSlice(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf duplicate() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf retainedDuplicate() {
    throw new UnsupportedOperationException();
  }

  @Override
  public int nioBufferCount() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer nioBuffer() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer nioBuffer(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer internalNioBuffer(int index, int length) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuffer[] nioBuffers() {
    return new ByteBuffer[0];
  }

  @Override
  public ByteBuffer[] nioBuffers(int index, int length) {
    return new ByteBuffer[0];
  }

  @Override
  public boolean hasArray() {
    throw new UnsupportedOperationException();
  }

  @Override
  public byte[] array() {
    return new byte[0];
  }

  @Override
  public int arrayOffset() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean hasMemoryAddress() {
    throw new UnsupportedOperationException();
  }

  @Override
  public long memoryAddress() {
    throw new UnsupportedOperationException();
  }

  @Override
  public String toString(Charset charset) {
    throw new UnsupportedOperationException();
  }

  @Override
  public String toString(int index, int length, Charset charset) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int hashCode() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean equals(Object obj) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int compareTo(ByteBuf buffer) {
    throw new UnsupportedOperationException();
  }

  @Override
  public String toString() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf retain(int increment) {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf retain() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf touch() {
    throw new UnsupportedOperationException();
  }

  @Override
  public ByteBuf touch(Object hint) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int refCnt() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean release() {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean release(int decrement) {
    throw new UnsupportedOperationException();
  }
}
