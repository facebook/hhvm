/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package com.facebook.thrift.protocol;

import com.facebook.thrift.TException;
import com.facebook.thrift.meta_data.FieldMetaData;
import com.facebook.thrift.transport.TTransport;
import java.util.Map;

/**
 * JSON protocol implementation for thrift.
 *
 * <p>This should not be confused with the TJSONProtocol. This protocol store data in JSON-styled
 * "field name" -> "field value", but it doesn't preserve field id nor field type (which doesn't
 * give the user the same backward/forward compatibility guarantees).
 *
 * <p>It is not recommended to use this protocol for RPC that needs backward/forward compatibility
 * guarantees.
 */
public class TSimpleJSONProtocol extends TProtocol {
  private final AbstractTSimpleJSONProtocol delegate;

  /** Factory */
  public static class Factory implements TProtocolFactory {
    private final boolean useBase64;

    public Factory() {
      this(false);
    }

    public Factory(boolean useBase64) {
      this.useBase64 = useBase64;
    }

    public TProtocol getProtocol(TTransport trans) {
      return new TSimpleJSONProtocol(trans, this.useBase64);
    }

    public TProtocol getProtocol(TTransport trans, boolean useBase64) {
      return new TSimpleJSONProtocol(trans, this.useBase64 || useBase64);
    }
  }

  /** Constructor */
  public TSimpleJSONProtocol(TTransport trans) {
    this(trans, false);
  }

  /**
   * Creates new TSimpleJSONProtocol with the option to enabled base64 encoding.
   *
   * @param trans transport encoded too
   * @param useBase64 if true will encode binary using base 64
   */
  public TSimpleJSONProtocol(TTransport trans, boolean useBase64) {
    super(trans);
    this.delegate =
        useBase64 ? new Base64TSimpleJSONProtocol(trans) : new DefaultTSimpleJSONProtocol(trans);
  }

  @Override
  public void writeMessageBegin(TMessage message) throws TException {
    delegate.writeMessageBegin(message);
  }

  @Override
  public void writeMessageEnd() throws TException {
    delegate.writeMessageEnd();
  }

  @Override
  public void writeStructBegin(TStruct struct) throws TException {
    delegate.writeStructBegin(struct);
  }

  @Override
  public void writeStructEnd() throws TException {
    delegate.writeStructEnd();
  }

  @Override
  public void writeFieldBegin(TField field) throws TException {
    delegate.writeFieldBegin(field);
  }

  @Override
  public void writeFieldEnd() {
    delegate.writeFieldEnd();
  }

  @Override
  public void writeFieldStop() {
    delegate.writeFieldStop();
  }

  @Override
  public void writeMapBegin(TMap map) throws TException {
    delegate.writeMapBegin(map);
  }

  @Override
  public void writeMapEnd() throws TException {
    delegate.writeMapEnd();
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    delegate.writeListBegin(list);
  }

  @Override
  public void writeListEnd() throws TException {
    delegate.writeListEnd();
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {
    delegate.writeSetBegin(set);
  }

  @Override
  public void writeSetEnd() throws TException {
    delegate.writeSetEnd();
  }

  @Override
  public void writeBool(boolean b) throws TException {
    delegate.writeBool(b);
  }

  @Override
  public void writeByte(byte b) throws TException {
    delegate.writeByte(b);
  }

  @Override
  public void writeI16(short i16) throws TException {
    delegate.writeI16(i16);
  }

  @Override
  public void writeI32(int i32) throws TException {
    delegate.writeI32(i32);
  }

  public void _writeStringData(String s) throws TException {
    delegate._writeStringData(s);
  }

  @Override
  public void writeI64(long i64) throws TException {
    delegate.writeI64(i64);
  }

  @Override
  public void writeFloat(float flt) throws TException {
    delegate.writeFloat(flt);
  }

  @Override
  public void writeDouble(double dub) throws TException {
    delegate.writeDouble(dub);
  }

  @Override
  public void writeString(String str) throws TException {
    delegate.writeString(str);
  }

  @Override
  public void writeBinary(byte[] bin) throws TException {
    delegate.writeBinary(bin);
  }

  public void readJSONSyntaxChar(byte[] b) throws TException {
    delegate.readJSONSyntaxChar(b);
  }

  public void readJSONSyntaxString(byte[] expected) throws TException {
    delegate.readJSONSyntaxString(expected);
  }

  public float readJSONFloat() throws TException {
    return delegate.readJSONFloat();
  }

  @Override
  public TMessage readMessageBegin() throws TException {
    return delegate.readMessageBegin();
  }

  @Override
  public void readMessageEnd() throws TException {
    delegate.readMessageEnd();
  }

  @Override
  public TStruct readStructBegin(Map<Integer, FieldMetaData> fieldMetadata) throws TException {
    return delegate.readStructBegin(fieldMetadata);
  }

  @Override
  public TStruct readStructBegin() {
    return delegate.readStructBegin();
  }

  @Override
  public void readStructEnd() throws TException {
    delegate.readStructEnd();
  }

  @Override
  public TField readFieldBegin() throws TException {
    return delegate.readFieldBegin();
  }

  @Override
  public void readFieldEnd() throws TException {
    delegate.readFieldEnd();
  }

  @Override
  public TMap readMapBegin() throws TException {
    return delegate.readMapBegin();
  }

  @Override
  public boolean peekMap() throws TException {
    return delegate.peekMap();
  }

  @Override
  public void readMapEnd() throws TException {
    delegate.readMapEnd();
  }

  @Override
  public TList readListBegin() throws TException {
    return delegate.readListBegin();
  }

  @Override
  public boolean peekList() throws TException {
    return delegate.peekList();
  }

  @Override
  public void readListEnd() throws TException {
    delegate.readListEnd();
  }

  @Override
  public TSet readSetBegin() throws TException {
    return delegate.readSetBegin();
  }

  @Override
  public boolean peekSet() throws TException {
    return delegate.peekSet();
  }

  @Override
  public void readSetEnd() throws TException {
    delegate.readSetEnd();
  }

  @Override
  public boolean readBool() throws TException {
    return delegate.readBool();
  }

  @Override
  public byte readByte() throws TException {
    return delegate.readByte();
  }

  @Override
  public short readI16() throws TException {
    return delegate.readI16();
  }

  @Override
  public int readI32() throws TException {
    return delegate.readI32();
  }

  @Override
  public long readI64() throws TException {
    return delegate.readI64();
  }

  @Override
  public double readDouble() throws TException {
    return delegate.readDouble();
  }

  @Override
  public float readFloat() throws TException {
    return delegate.readFloat();
  }

  @Override
  public String readString() throws TException {
    return delegate.readString();
  }

  @Override
  public byte[] readBinary() throws TException {
    return delegate.readBinary();
  }

  @Override
  public void skipBinary() throws TException {
    delegate.skipBinary();
  }

  public static byte getTypeIDForPeekedByte(byte peekedByte) throws TException {
    return AbstractTSimpleJSONProtocol.getTypeIDForPeekedByte(peekedByte);
  }

  public void pushWriteContext(AbstractTSimpleJSONProtocol.Context c) {
    delegate.pushWriteContext(c);
  }

  public void popWriteContext() {
    delegate.popWriteContext();
  }

  public void assertContextIsNotMapKey(String invalidKeyType)
      throws AbstractTSimpleJSONProtocol.CollectionMapKeyException {
    delegate.assertContextIsNotMapKey(invalidKeyType);
  }
}
