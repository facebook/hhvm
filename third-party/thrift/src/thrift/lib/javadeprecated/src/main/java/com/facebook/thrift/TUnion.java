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

package com.facebook.thrift;

import com.facebook.thrift.meta_data.FieldMetaData;
import com.facebook.thrift.protocol.TField;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolException;
import com.facebook.thrift.protocol.TStruct;
import com.facebook.thrift.protocol.TType;
import java.util.List;
import java.util.Map;
import java.util.Set;

@SuppressWarnings("serial")
public abstract class TUnion<Me extends TUnion<Me>> implements TBase {

  protected Object value_;
  protected int setField_;

  protected TUnion() {
    setField_ = 0;
    value_ = null;
  }

  protected TUnion(int fieldId, Object value) {
    setFieldValue(fieldId, value);
  }

  protected TUnion(TUnion<Me> other) {
    if (!other.getClass().equals(this.getClass())) {
      throw new ClassCastException();
    }
    setField_ = other.setField_;
    value_ = TBaseHelper.deepCopyUnchecked(other.value_);
  }

  public int getSetField() {
    return setField_;
  }

  public Object getFieldValue() {
    return value_;
  }

  public Object getFieldValue(int fieldId) {
    if (fieldId != setField_) {
      throw new IllegalArgumentException(
          "Cannot get the value of field "
              + fieldId
              + " because union's set field is "
              + setField_);
    }

    return getFieldValue();
  }

  public boolean isSet() {
    return setField_ != 0;
  }

  public boolean isSet(int fieldId) {
    return setField_ == fieldId;
  }

  public void read(TProtocol iprot) throws TException {
    setField_ = 0;
    value_ = null;
    iprot.readStructBegin(getMetaDataMap());
    TField field = iprot.readFieldBegin();
    if (field.type != TType.STOP) {
      value_ = readValue(iprot, field);
      if (value_ != null) {
        setField_ = field.id;
      }
      iprot.readFieldEnd();
      // this is so that we will eat the stop byte. we could put a check here to
      // make sure that it actually *is* the stop byte, but it's faster to do it
      // this way.
      iprot.readFieldBegin();
      iprot.readFieldEnd();
    }
    iprot.readStructEnd();
  }

  public void setFieldValue(int fieldId, Object value) {
    if (value == null) {
      throw new IllegalArgumentException(
          String.format("TUnion value for field id '%d' can't be null!", fieldId));
    }
    checkType((short) fieldId, value);
    setField_ = (short) fieldId;
    value_ = value;
  }

  public void write(TProtocol oprot) throws TException {
    if (getSetField() == 0 || getFieldValue() == null) {
      throw new TProtocolException("Cannot write a TUnion with no set value!");
    }
    oprot.writeStructBegin(getStructDesc());
    oprot.writeFieldBegin(getFieldDesc(setField_));
    writeValue(oprot, (short) setField_, value_);
    oprot.writeFieldEnd();
    oprot.writeFieldStop();
    oprot.writeStructEnd();
  }

  /** Generic implementation using reflection, codegen can override this in a more efficient way. */
  protected void checkType(short fieldId, Object value)
      throws ClassCastException, IllegalArgumentException {
    TField tField = TBaseHelper.getTField(this, fieldId);
    if (tField == null) {
      throw new IllegalArgumentException(
          "field #" + fieldId + " not found in Thrift struct " + this.getClass().getSimpleName());
    }

    if (!validateType(value, tField.type)) {
      throw new ClassCastException(
          String.format(
              "Was expecting value of type id %d for field id %d, but got %s",
              tField.type, fieldId, value.getClass().getSimpleName()));
    }
  }

  private static boolean validateType(Object value, short typeId) {
    switch (typeId) {
      case TType.BOOL:
        return value instanceof Boolean;
      case TType.BYTE:
        return value instanceof Byte;
      case TType.DOUBLE:
        return value instanceof Double;
      case TType.I16:
        return value instanceof Short;
      case TType.I32:
        return value instanceof Integer;
      case TType.I64:
        return value instanceof Long;
      case TType.STRING:
        return value instanceof String;
      case TType.STRUCT:
        return value instanceof TBase;
      case TType.MAP:
        return value instanceof Map;
      case TType.SET:
        return value instanceof Set;
      case TType.LIST:
        return value instanceof List;
      case TType.ENUM:
        return value instanceof TEnum;
      case TType.FLOAT:
        return value instanceof Float;
      default:
        return false;
    }
  }

  /**
   * Implementation should be generated to read the right stuff from the wire based on the field
   * header.
   */
  protected abstract Object readValue(TProtocol iprot, TField field) throws TException;

  protected abstract void writeValue(TProtocol oprot, short setField, Object value)
      throws TException;

  protected abstract TStruct getStructDesc();

  protected abstract TField getFieldDesc(int setField);

  public abstract Me deepCopy();

  protected int compareToImpl(Me other) {
    if (other == null) {
      // See docs for java.lang.Comparable
      throw new NullPointerException();
    }

    if (other == this) {
      return 0;
    }

    int lastComparison = TBaseHelper.compareTo(this.setField_, other.setField_);
    if (lastComparison != 0) {
      return lastComparison;
    }

    return TBaseHelper.compareToUnchecked(this.getFieldValue(), other.getFieldValue());
  }

  protected boolean equalsNobinaryImpl(Me other) {
    if (other == null || this.getClass() != other.getClass()) {
      return false;
    }
    if (other == this) {
      return true;
    }
    if (this.getSetField() != other.getSetField()) {
      return false;
    }
    if (this.getFieldValue() == null || other.getFieldValue() == null) {
      return this.getFieldValue() == null && other.getFieldValue() == null;
    }
    return TBaseHelper.equalsNobinaryUnchecked(this.getFieldValue(), other.getFieldValue());
  }

  protected boolean equalsSlowImpl(Me other) {
    if (other == null || this.getClass() != other.getClass()) {
      return false;
    }
    if (other == this) {
      return true;
    }
    if (this.getSetField() != other.getSetField()) {
      return false;
    }
    if (this.getFieldValue() == null || other.getFieldValue() == null) {
      return this.getFieldValue() == null && other.getFieldValue() == null;
    }
    return TBaseHelper.equalsSlowUnchecked(this.getFieldValue(), other.getFieldValue());
  }

  @Override
  public String toString(int indent, boolean prettyPrint) {
    int fieldId = getSetField();
    Object v = getFieldValue();
    if (v == null) {
      return String.format("<%s uninitialized>", this.getClass().getSimpleName());
    }

    String vStr = null;
    if (v instanceof byte[]) {
      vStr = bytesToStr((byte[]) v);
    } else if (TBase.class.isAssignableFrom(v.getClass())) {
      vStr = ((TBase) v).toString(indent, prettyPrint);
    } else {
      vStr = v.toString();
    }
    return String.format(
        "<%s %s:%s>", this.getClass().getSimpleName(), getFieldDesc(getSetField()).name, vStr);
  }

  @Override
  public String toString() {
    return toString(1, true);
  }

  protected abstract Map<Integer, FieldMetaData> getMetaDataMap();

  private static String bytesToStr(byte[] bytes) {
    StringBuilder sb = new StringBuilder();
    int size = Math.min(bytes.length, 128);
    for (int i = 0; i < size; i++) {
      if (i != 0) {
        sb.append(" ");
      }
      String digit = Integer.toHexString(bytes[i]);
      sb.append(digit.length() > 1 ? digit : "0" + digit);
    }
    if (bytes.length > 128) {
      sb.append(" ...");
    }
    return sb.toString();
  }
}
