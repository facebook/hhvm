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

import com.facebook.thrift.TApplicationException;
import com.facebook.thrift.TException;
import com.facebook.thrift.transport.THeaderException;
import com.facebook.thrift.transport.THeaderTransport;
import com.facebook.thrift.transport.TTransport;
import java.util.List;
import java.util.Map;

/** Pass through protocol for use with THeaderTransport servers */
public class THeaderProtocol extends TProtocol {

  private TProtocol proto;
  private int protoId;

  private void resetProtocol() throws TProtocolException {
    // We guarantee trans_ to be of type THeaderTransport, parent class does not
    if (proto != null && protoId == ((THeaderTransport) trans_).getProtocolId()) {
      return;
    }

    protoId = ((THeaderTransport) trans_).getProtocolId();
    switch (protoId) {
      case THeaderTransport.T_BINARY_PROTOCOL:
        proto = new TBinaryProtocol(trans_, true, true);
        break;
      case THeaderTransport.T_COMPACT_PROTOCOL:
        proto = new TCompactProtocol(trans_);
        break;
      default:
        throw new TProtocolException("Unknown protocol id: " + protoId);
    }
  }

  /** Factory */
  @SuppressWarnings("serial")
  public static class Factory implements TProtocolFactory {
    List<THeaderTransport.ClientTypes> clientTypes;

    public Factory(List<THeaderTransport.ClientTypes> clientTypes) {
      this.clientTypes = clientTypes;
    }

    public Factory() {}

    public TProtocol getProtocol(TTransport trans) {
      if (trans instanceof THeaderTransport) {
        // Make sure we call the THeaderTransport specific constructor if
        // we can.
        return new THeaderProtocol((THeaderTransport) trans);
      } else {
        return new THeaderProtocol(trans, clientTypes);
      }
    }
  }

  /** Constructor */
  public THeaderProtocol(TTransport trans, List<THeaderTransport.ClientTypes> clientTypes) {
    this(new THeaderTransport(trans, clientTypes));
  }

  public THeaderProtocol(THeaderTransport trans) {
    super(trans);
    try {
      resetProtocol();
    } catch (TProtocolException tpe) {
      // This actually can't occur here - always constructed with a
      // TBinaryProtocol.
      throw new RuntimeException(tpe);
    }
  }

  @Override
  public void writeMessageBegin(TMessage message) throws TException {
    proto.writeMessageBegin(message);
  }

  @Override
  public void writeMessageEnd() throws TException {
    proto.writeMessageEnd();
  }

  @Override
  public void writeStructBegin(TStruct struct) throws TException {
    proto.writeStructBegin(struct);
  }

  @Override
  public void writeStructEnd() throws TException {
    proto.writeStructEnd();
  }

  @Override
  public void writeFieldBegin(TField field) throws TException {
    proto.writeFieldBegin(field);
  }

  @Override
  public void writeFieldEnd() throws TException {
    proto.writeFieldEnd();
  }

  @Override
  public void writeFieldStop() throws TException {
    proto.writeFieldStop();
  }

  @Override
  public void writeMapBegin(TMap map) throws TException {
    proto.writeMapBegin(map);
  }

  @Override
  public void writeMapEnd() throws TException {
    proto.writeMapEnd();
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    proto.writeListBegin(list);
  }

  @Override
  public void writeListEnd() throws TException {
    proto.writeListEnd();
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {
    proto.writeSetBegin(set);
  }

  @Override
  public void writeSetEnd() throws TException {
    proto.writeSetEnd();
  }

  @Override
  public void writeBool(boolean b) throws TException {
    proto.writeBool(b);
  }

  @Override
  public void writeByte(byte b) throws TException {
    proto.writeByte(b);
  }

  @Override
  public void writeI16(short i16) throws TException {
    proto.writeI16(i16);
  }

  @Override
  public void writeI32(int i32) throws TException {
    proto.writeI32(i32);
  }

  @Override
  public void writeI64(long i64) throws TException {
    proto.writeI64(i64);
  }

  @Override
  public void writeDouble(double dub) throws TException {
    proto.writeDouble(dub);
  }

  @Override
  public void writeFloat(float flt) throws TException {
    proto.writeFloat(flt);
  }

  @Override
  public void writeString(String str) throws TException {
    proto.writeString(str);
  }

  @Override
  public void writeBinary(byte[] bin) throws TException {
    proto.writeBinary(bin);
  }

  /** Reading methods. */

  /** Helper method to throw an error back to the endpoint */
  public void notifyEndpoint(String msg) throws TException {
    if (proto != null) {
      writeMessageBegin(new TMessage("", TMessageType.EXCEPTION, 0));
      TApplicationException ex = new TApplicationException(msg);
      ex.write(this);
      writeMessageEnd();
      trans_.flush();
    }
  }

  @Override
  public TMessage readMessageBegin() throws TException {
    // Read the next frame, and change protocols if needed
    try {
      ((THeaderTransport) trans_)._resetProtocol();
      resetProtocol();
    } catch (THeaderException e) {
      // THeaderExceptions are exceptions we want thrown back to the endpoint
      // Such as unknown transforms or protocols.  Endpoint may retry
      notifyEndpoint(e.toString());
    }
    return proto.readMessageBegin();
  }

  @Override
  public void readMessageEnd() throws TException {
    proto.readMessageEnd();
  }

  public TStruct readStructBegin(
      Map<Integer, com.facebook.thrift.meta_data.FieldMetaData> metaDataMap) throws TException {
    return proto.readStructBegin(metaDataMap);
  }

  @Override
  public void readStructEnd() throws TException {
    proto.readStructEnd();
  }

  @Override
  public TField readFieldBegin() throws TException {
    return proto.readFieldBegin();
  }

  @Override
  public void readFieldEnd() throws TException {
    proto.readFieldEnd();
  }

  @Override
  public TMap readMapBegin() throws TException {
    return proto.readMapBegin();
  }

  @Override
  public void readMapEnd() throws TException {
    proto.readMapEnd();
  }

  @Override
  public TList readListBegin() throws TException {
    return proto.readListBegin();
  }

  @Override
  public void readListEnd() throws TException {
    proto.readListEnd();
  }

  @Override
  public TSet readSetBegin() throws TException {
    return proto.readSetBegin();
  }

  @Override
  public void readSetEnd() throws TException {
    proto.readSetEnd();
  }

  @Override
  public boolean readBool() throws TException {
    return proto.readBool();
  }

  @Override
  public byte readByte() throws TException {
    return proto.readByte();
  }

  @Override
  public short readI16() throws TException {
    return proto.readI16();
  }

  @Override
  public int readI32() throws TException {
    return proto.readI32();
  }

  @Override
  public long readI64() throws TException {
    return proto.readI64();
  }

  @Override
  public double readDouble() throws TException {
    return proto.readDouble();
  }

  @Override
  public float readFloat() throws TException {
    return proto.readFloat();
  }

  @Override
  public String readString() throws TException {
    return proto.readString();
  }

  @Override
  public byte[] readBinary() throws TException {
    return proto.readBinary();
  }
}
