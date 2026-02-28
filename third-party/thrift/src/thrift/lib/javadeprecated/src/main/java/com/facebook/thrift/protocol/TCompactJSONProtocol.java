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
import com.facebook.thrift.meta_data.FieldMetaData;
import com.facebook.thrift.transport.TTransport;
import java.util.Map;
import java.util.Stack;

/**
 * Compact JSON protocol. It is similar to simple JSON protocol, only it is JSON-compliant, and
 * supports reading back. Unlike simple JSON protocol, compact JSON protocol encodes binary fields
 * with Base64 encloses Infinity and -Infinity values in quotes encloses non-string map keys in
 * quotes
 */
public class TCompactJSONProtocol extends TJSONProtocolBase {

  /** Factory */
  @SuppressWarnings("serial")
  public static class Factory implements TProtocolFactory {
    public TProtocol getProtocol(TTransport trans) {
      return new TCompactJSONProtocol(trans);
    }
  }

  /** Constructor */
  public TCompactJSONProtocol(TTransport trans) {
    super(trans);
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
    writeString(field.name);
  }

  @Override
  public void writeFieldEnd() throws TException {}

  @Override
  public void writeFieldStop() {}

  @Override
  public void writeMapBegin(TMap map) throws TException {
    writeJSONObjectStart();
  }

  @Override
  public void writeMapEnd() throws TException {
    writeJSONObjectEnd();
  }

  @Override
  public void writeListBegin(TList list) throws TException {
    writeJSONArrayStart();
  }

  @Override
  public void writeListEnd() throws TException {
    writeJSONArrayEnd();
  }

  @Override
  public void writeSetBegin(TSet set) throws TException {
    writeJSONArrayStart();
  }

  @Override
  public void writeSetEnd() throws TException {
    writeJSONArrayEnd();
  }

  @Override
  public void writeBool(boolean b) throws TException {
    writeJSONInteger(b ? 1 : 0);
  }

  /** Reading methods. */

  // Stack of nested structs that we may be in
  private Stack<Map<Integer, FieldMetaData>> structStack_ =
      new Stack<Map<Integer, FieldMetaData>>();

  @Override
  public TStruct readStructBegin(Map<Integer, FieldMetaData> metaDataMap) throws TException {
    readJSONObjectStart();
    structStack_.push(metaDataMap);
    return ANONYMOUS_STRUCT;
  }

  @Override
  public void readStructEnd() throws TException {
    structStack_.pop();
    readJSONObjectEnd();
  }

  @Override
  public TField readFieldBegin() throws TException {
    if (reader_.peek() == RBRACE[0]) {
      // No more fields at current level
      return new TField("", TType.STOP, (short) 0);
    }

    String name = readString();

    for (Map.Entry<Integer, FieldMetaData> entry : structStack_.peek().entrySet()) {
      if (entry.getValue().fieldName.equals(name)) {
        // Field name match
        return new TField(name, entry.getValue().valueMetaData.type, entry.getKey().shortValue());
      }
    }

    // Unknown field name, return id 0 and a compatible type, to let user skip it
    return new TField(name, getTypeIDForPeekedByte(context_.peekNextValue()), (short) 0);
  }

  @Override
  public void readFieldEnd() throws TException {}

  @Override
  public TMap readMapBegin() throws TException {
    readJSONObjectStart();
    return new TMap(TType.STRING, TType.STOP, -1);
  }

  @Override
  public boolean peekMap() throws TException {
    return reader_.peek() != RBRACE[0];
  }

  @Override
  public void readMapEnd() throws TException {
    readJSONObjectEnd();
  }

  @Override
  public TList readListBegin() throws TException {
    readJSONArrayStart();
    return new TList(getTypeIDForPeekedByte(reader_.peek()), -1);
  }

  @Override
  public boolean peekList() throws TException {
    return reader_.peek() != RBRACKET[0];
  }

  @Override
  public void readListEnd() throws TException {
    readJSONArrayEnd();
  }

  @Override
  public TSet readSetBegin() throws TException {
    readJSONArrayStart();
    return new TSet(getTypeIDForPeekedByte(reader_.peek()), -1);
  }

  @Override
  public boolean peekSet() throws TException {
    return reader_.peek() != RBRACKET[0];
  }

  @Override
  public void readSetEnd() throws TException {
    readJSONArrayEnd();
  }
}
