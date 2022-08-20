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

import static com.facebook.thrift.swift.adapters.ThriftMessageAdapters.adaptThriftMessage;
import static com.facebook.thrift.swift.adapters.TransportAdapter.adaptTransport;

import com.facebook.thrift.TException;
import com.facebook.thrift.meta_data.FieldMetaData;
import com.facebook.thrift.protocol.TField;
import com.facebook.thrift.protocol.TList;
import com.facebook.thrift.protocol.TMap;
import com.facebook.thrift.protocol.TMessage;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TSet;
import com.facebook.thrift.protocol.TStruct;
import java.nio.ByteBuffer;
import java.util.Map;

public final class ApacheToFacebookProtocolAdapter extends TProtocol {
  protected final org.apache.thrift.protocol.TProtocol apacheProtocol;

  public ApacheToFacebookProtocolAdapter(org.apache.thrift.protocol.TProtocol apacheProtocol) {
    super(adaptTransport(apacheProtocol.getTransport()));
    this.apacheProtocol = apacheProtocol;
  }

  @Override
  public byte[] readBinary() throws TException {
    try {
      ByteBuffer buf = apacheProtocol.readBinary();
      if (buf.hasArray()
          && buf.arrayOffset() == 0
          && buf.position() == 0
          && buf.remaining() == buf.capacity()) {
        return buf.array();
      } else {
        byte[] byteArray = new byte[buf.remaining()];
        buf.get(byteArray);
        return byteArray;
      }
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeMessageBegin(TMessage message) throws TException {
    try {
      apacheProtocol.writeMessageBegin(adaptThriftMessage(message));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeMessageEnd() throws TException {
    try {
      apacheProtocol.writeMessageEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeStructBegin(TStruct struct) throws TException {
    try {
      apacheProtocol.writeStructBegin(new org.apache.thrift.protocol.TStruct(struct.name));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeStructEnd() throws TException {
    try {
      apacheProtocol.writeStructEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeFieldBegin(TField field) throws TException {
    try {
      apacheProtocol.writeFieldBegin(
          new org.apache.thrift.protocol.TField(field.name, field.type, field.id));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeFieldEnd() throws TException {
    try {
      apacheProtocol.writeFieldEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeFieldStop() throws TException {
    try {
      apacheProtocol.writeFieldStop();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeMapBegin(TMap map) throws TException {
    try {
      apacheProtocol.writeMapBegin(
          new org.apache.thrift.protocol.TMap(map.keyType, map.valueType, map.size));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeMapEnd() throws TException {
    try {
      apacheProtocol.writeMapEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    try {
      apacheProtocol.writeListBegin(new org.apache.thrift.protocol.TList(list.elemType, list.size));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeListEnd() throws TException {
    try {
      apacheProtocol.writeListEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {
    try {
      apacheProtocol.writeSetBegin(new org.apache.thrift.protocol.TSet(set.elemType, set.size));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeSetEnd() throws TException {
    try {
      apacheProtocol.writeSetEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeBool(boolean b) throws TException {
    try {
      apacheProtocol.writeBool(b);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeByte(byte b) throws TException {
    try {
      apacheProtocol.writeByte(b);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeI16(short i16) throws TException {
    try {
      apacheProtocol.writeI16(i16);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeI32(int i32) throws TException {
    try {
      apacheProtocol.writeI32(i32);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeI64(long i64) throws TException {
    try {
      apacheProtocol.writeI64(i64);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeDouble(double dub) throws TException {
    try {
      apacheProtocol.writeDouble(dub);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeString(String str) throws TException {
    try {
      apacheProtocol.writeString(str);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void writeBinary(byte[] byteArray) throws TException {
    try {
      apacheProtocol.writeBinary(ByteBuffer.wrap(byteArray));
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TMessage readMessageBegin() throws TException {
    try {
      org.apache.thrift.protocol.TMessage message = apacheProtocol.readMessageBegin();
      return new TMessage(message.name, message.type, message.seqid);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readMessageEnd() throws TException {
    try {
      apacheProtocol.readMessageEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TStruct readStructBegin() throws TException {
    try {
      org.apache.thrift.protocol.TStruct struct = apacheProtocol.readStructBegin();
      return new TStruct(struct.name);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readStructEnd() throws TException {
    try {
      apacheProtocol.readStructEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TField readFieldBegin() throws TException {
    try {
      org.apache.thrift.protocol.TField field = apacheProtocol.readFieldBegin();
      return new TField(field.name, field.type, field.id);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readFieldEnd() throws TException {
    try {
      apacheProtocol.readFieldEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TMap readMapBegin() throws TException {
    try {
      org.apache.thrift.protocol.TMap map = apacheProtocol.readMapBegin();
      return new TMap(map.keyType, map.valueType, map.size);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readMapEnd() throws TException {
    try {
      apacheProtocol.readMapEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TList readListBegin() throws TException {
    try {
      org.apache.thrift.protocol.TList list = apacheProtocol.readListBegin();
      return new TList(list.elemType, list.size);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readListEnd() throws TException {
    try {
      apacheProtocol.readListEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public TSet readSetBegin() throws TException {
    org.apache.thrift.protocol.TSet set = null;
    try {
      set = apacheProtocol.readSetBegin();
      return new TSet(set.elemType, set.size);
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public void readSetEnd() throws TException {
    try {
      apacheProtocol.readSetEnd();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public boolean readBool() throws TException {
    try {
      return apacheProtocol.readBool();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public byte readByte() throws TException {
    try {
      return apacheProtocol.readByte();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public short readI16() throws TException {
    try {
      return apacheProtocol.readI16();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public int readI32() throws TException {
    try {
      return apacheProtocol.readI32();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public long readI64() throws TException {
    try {
      return apacheProtocol.readI64();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public double readDouble() throws TException {
    try {
      return apacheProtocol.readDouble();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public String readString() throws TException {
    try {
      return apacheProtocol.readString();
    } catch (org.apache.thrift.TException e) {
      throw new RuntimeException(e);
    }
  }

  @Override
  public float readFloat() throws TException {
    throw new UnsupportedOperationException(
        "Cannot read 'float' type using protocol adapted from apache TProtocol");
  }

  @Override
  public void writeFloat(float flt) throws TException {
    throw new UnsupportedOperationException(
        "Cannot write 'float' type using protocol adapted from apache TProtocol");
  }

  @Override
  public TStruct readStructBegin(Map<Integer, FieldMetaData> metaDataMap) throws TException {

    try {
      org.apache.thrift.protocol.TStruct apacheTstruct = apacheProtocol.readStructBegin();
      return new TStruct(apacheTstruct.name);
    } catch (org.apache.thrift.TException e) {
      throw new UnsupportedOperationException(
          "Cannot handle struct metadata mapping using protocol adapted from apache TProtocol");
    }
  }
}
