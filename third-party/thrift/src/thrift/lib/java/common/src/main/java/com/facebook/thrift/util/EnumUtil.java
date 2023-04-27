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

import com.facebook.thrift.enums.BaseEnum;
import com.facebook.thrift.enums.ThriftEnum;

public class EnumUtil {

  /**
   * Returns the value of the enum. Java enums and thrift open enums implement base enum which
   * provides the i32 value and the type. Java enums return one of the defined value, or 0 for
   * undefined value, thrift enums return one of the defined value or unrecognized value.
   *
   * @param baseEnum Any java enum or thrift open enum
   * @return i32 value of the enum value.
   */
  public static int getValue(BaseEnum baseEnum) {
    if (baseEnum == null) {
      // This is the existing behavior
      return 0;
    }
    if (baseEnum.isClosedEnum()) {
      return baseEnum.getValue();
    }

    ThriftEnum t = (ThriftEnum) baseEnum;
    if (t.isValueUnrecognized()) {
      return t.getUnrecognizedValue();
    }

    return t.getValue();
  }
}
