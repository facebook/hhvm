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

package com.facebook.thrift.enums;

import com.facebook.swift.codec.ThriftEnumValue;
import com.facebook.thrift.payload.ThriftSerializable;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocol;

/**
 * Base class for all open type enums. If the value is a defined enum value, it is stored in value
 * field, if it is non-defined value, stored in unrecognized value field.
 *
 * @param <T>
 */
public abstract class ThriftEnum<T extends ThriftEnum>
    implements BaseEnum, ThriftSerializable, Comparable<T> {

  protected int value;
  protected int unrecognizedValue;

  /**
   * @return Returns the integer value of the defined enum.
   */
  @Override
  @ThriftEnumValue
  public int getValue() {
    return this.value;
  }

  /**
   * @return Returns the unrecognized value of the enum if the value is not defined.
   */
  public int getUnrecognizedValue() {
    return unrecognizedValue;
  }

  /**
   * @return true if the enum value is unrecognized, false otherwise.
   */
  public abstract boolean isValueUnrecognized();

  /**
   * @return Ordinal value of the enum. Unrecognized value has the last ordinal value.
   */
  public abstract int ordinal();

  /**
   * @return Name of the enum defined in the IDL.
   */
  public abstract String name();

  public boolean isClosedEnum() {
    return false;
  }

  public void write0(TProtocol oprot) throws TException {
    oprot.writeI32(this.getValue());
  }

  public int compareTo(T o) {
    int n = this.value - o.value;
    if (n != 0) {
      return n;
    }
    return this.unrecognizedValue - o.unrecognizedValue;
  }

  @Override
  public int hashCode() {
    return value;
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof ThriftEnum<?>)) {
      return false;
    }
    return ((T) o).value == this.value && ((T) o).unrecognizedValue == this.unrecognizedValue;
  }
}
