/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package com.facebook.thrift.lite.protocol;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

/** Binary protocol implementation for thrift. */
public class TBinaryProtocol {
  public static final int VERSION_1 = 0x80010000;

  protected boolean mStrictWrite = true;
  private BufferedOutputStream mTrans;
  private byte[] mBufferOut;

  public TBinaryProtocol(BufferedOutputStream trans) {
    mTrans = trans;
    mBufferOut = new byte[1024];
  }

  public BufferedOutputStream getTransport() {
    return mTrans;
  }

  public void writeMessageBegin(TMessage message) throws IOException {
    if (mStrictWrite) {
      int version = VERSION_1 | (message.type & 0xff);
      writeI32(version);
      writeString(message.name);
      writeI32(message.seqid);
    } else {
      writeString(message.name);
      writeByte(message.type);
      writeI32(message.seqid);
    }
  }

  public void writeMessageEnd() {}

  public void writeStructBegin(TStruct struct) {}

  public void writeStructEnd() {}

  public void writeFieldBegin(TField field) throws IOException {
    writeByte(field.type);
    writeI16(field.id);
  }

  public void writeFieldEnd() {}

  public void writeFieldStop() throws IOException {
    writeByte(TType.STOP);
  }

  public void writeMapBegin(TMap map) throws IOException {
    writeByte(map.keyType);
    writeByte(map.valueType);
    writeI32(map.size);
  }

  public void writeMapEnd() {}

  public void writeListBegin(TList list) throws IOException {
    writeByte(list.elemType);
    writeI32(list.size);
  }

  public void writeListEnd() {}

  public void writeSetBegin(TSet set) throws IOException {
    writeByte(set.elemType);
    writeI32(set.size);
  }

  public void writeSetEnd() {}

  public void writeBool(boolean b) throws IOException {
    writeByte(b ? (byte) 1 : (byte) 0);
  }

  public void writeByte(byte b) throws IOException {
    mBufferOut[0] = b;
    mTrans.write(mBufferOut, 0, 1);
  }

  public void writeI16(short i16) throws IOException {
    mBufferOut[0] = (byte) (0xff & (i16 >> 8));
    mBufferOut[1] = (byte) (0xff & (i16));
    mTrans.write(mBufferOut, 0, 2);
  }

  public void writeI32(int i32) throws IOException {
    mBufferOut[0] = (byte) (0xff & (i32 >> 24));
    mBufferOut[1] = (byte) (0xff & (i32 >> 16));
    mBufferOut[2] = (byte) (0xff & (i32 >> 8));
    mBufferOut[3] = (byte) (0xff & (i32));
    mTrans.write(mBufferOut, 0, 4);
  }

  public void writeI64(long i64) throws IOException {
    mBufferOut[0] = (byte) (0xff & (i64 >> 56));
    mBufferOut[1] = (byte) (0xff & (i64 >> 48));
    mBufferOut[2] = (byte) (0xff & (i64 >> 40));
    mBufferOut[3] = (byte) (0xff & (i64 >> 32));
    mBufferOut[4] = (byte) (0xff & (i64 >> 24));
    mBufferOut[5] = (byte) (0xff & (i64 >> 16));
    mBufferOut[6] = (byte) (0xff & (i64 >> 8));
    mBufferOut[7] = (byte) (0xff & (i64));
    mTrans.write(mBufferOut, 0, 8);
  }

  public void writeDouble(double dub) throws IOException {
    writeI64(Double.doubleToLongBits(dub));
  }

  public void writeFloat(float flt) throws IOException {
    writeI32(Float.floatToIntBits(flt));
  }

  public void writeString(String str) throws IOException {
    try {
      byte[] dat = str.getBytes("UTF-8");
      writeI32(dat.length);
      // keep copying and writing at most the IO buffer size at each step
      for (int off = 0; off < dat.length; off += mBufferOut.length) {
        int size = Math.min(mBufferOut.length, dat.length - off);
        System.arraycopy(dat, off, mBufferOut, 0, size);
        mTrans.write(mBufferOut, 0, size);
      }
    } catch (UnsupportedEncodingException uex) {
      throw new IOException("JVM DOES NOT SUPPORT UTF-8");
    }
  }

  public void writeBinary(byte[] bin) throws IOException {
    writeI32(bin.length);
    // keep copying and writing at most the IO buffer size at each step
    for (int off = 0; off < bin.length; off += mBufferOut.length) {
      int size = Math.min(mBufferOut.length, bin.length - off);
      System.arraycopy(bin, off, mBufferOut, 0, size);
      mTrans.write(mBufferOut, 0, size);
    }
  }
}
