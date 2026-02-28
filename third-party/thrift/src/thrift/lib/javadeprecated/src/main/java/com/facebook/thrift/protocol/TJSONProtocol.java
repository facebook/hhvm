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

import com.facebook.thrift.TException;
import com.facebook.thrift.transport.TTransport;
import java.util.Map;

/**
 * JSON protocol implementation for thrift.
 *
 * <p>This is a full-featured protocol supporting write and read.
 *
 * <p>Please see the C++ class header for a detailed description of the protocol's wire format.
 */
public class TJSONProtocol extends TJSONProtocolBase {

  /** Factory for JSON protocol objects */
  @SuppressWarnings("serial")
  public static class Factory implements TProtocolFactory {

    public TProtocol getProtocol(TTransport trans) {
      return new TJSONProtocol(trans);
    }
  }

  /** Constructor */
  public TJSONProtocol(TTransport trans) {
    super(trans);
  }

  @Override
  public void reset() {
    super.reset();
  }

  @Override
  public void writeStructBegin(TStruct struct) throws TException {
    writeJSONObjectStart();
  }

  @Override
  public void writeStructEnd() throws TException {
    writeJSONObjectEnd();
  }

  @Override
  public void writeFieldBegin(TField field) throws TException {
    writeJSONInteger(field.id);
    writeJSONObjectStart();
    writeJSONString(getTypeNameForTypeID(field.type));
  }

  @Override
  public void writeFieldEnd() throws TException {
    writeJSONObjectEnd();
  }

  @Override
  public void writeFieldStop() {}

  @Override
  public void writeMapBegin(TMap map) throws TException {
    writeJSONArrayStart();
    writeJSONString(getTypeNameForTypeID(map.keyType));
    writeJSONString(getTypeNameForTypeID(map.valueType));
    writeJSONInteger(map.size);
    writeJSONObjectStart();
  }

  @Override
  public void writeMapEnd() throws TException {
    writeJSONObjectEnd();
    writeJSONArrayEnd();
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    writeJSONArrayStart();
    writeJSONString(getTypeNameForTypeID(list.elemType));
    writeJSONInteger(list.size);
  }

  @Override
  public void writeListEnd() throws TException {
    writeJSONArrayEnd();
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {
    writeJSONArrayStart();
    writeJSONString(getTypeNameForTypeID(set.elemType));
    writeJSONInteger(set.size);
  }

  @Override
  public void writeSetEnd() throws TException {
    writeJSONArrayEnd();
  }

  /** Reading methods. */
  @Override
  public TStruct readStructBegin(
      Map<Integer, com.facebook.thrift.meta_data.FieldMetaData> metaDataMap) throws TException {
    readJSONObjectStart();
    return ANONYMOUS_STRUCT;
  }

  @Override
  public void readStructEnd() throws TException {
    readJSONObjectEnd();
  }

  @Override
  public TField readFieldBegin() throws TException {
    byte ch = reader_.peek();
    byte type;
    short id = 0;
    if (ch == RBRACE[0]) {
      type = TType.STOP;
    } else {
      id = (short) readJSONInteger();
      readJSONObjectStart();
      type = getTypeIDForTypeName(readJSONString(false).get());
    }
    return new TField("", type, id);
  }

  @Override
  public void readFieldEnd() throws TException {
    readJSONObjectEnd();
  }

  @Override
  public TMap readMapBegin() throws TException {
    readJSONArrayStart();
    byte keyType = getTypeIDForTypeName(readJSONString(false).get());
    byte valueType = getTypeIDForTypeName(readJSONString(false).get());
    int size = (int) readJSONInteger();
    readJSONObjectStart();
    return new TMap(keyType, valueType, size);
  }

  @Override
  public void readMapEnd() throws TException {
    readJSONObjectEnd();
    readJSONArrayEnd();
  }

  @Override
  public TList readListBegin() throws TException {
    readJSONArrayStart();
    byte elemType = getTypeIDForTypeName(readJSONString(false).get());
    int size = (int) readJSONInteger();
    return new TList(elemType, size);
  }

  @Override
  public void readListEnd() throws TException {
    readJSONArrayEnd();
  }

  @Override
  public TSet readSetBegin() throws TException {
    readJSONArrayStart();
    byte elemType = getTypeIDForTypeName(readJSONString(false).get());
    int size = (int) readJSONInteger();
    return new TSet(elemType, size);
  }

  @Override
  public void readSetEnd() throws TException {
    readJSONArrayEnd();
  }
}
