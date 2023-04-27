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

package com.facebook.thrift.meta_data;

import com.facebook.thrift.protocol.TType;

/**
 * FieldValueMetaData and collection of subclasses to store metadata about the value(s) of a field
 */
@SuppressWarnings("serial")
public class FieldValueMetaData implements java.io.Serializable {
  public final byte type;

  public FieldValueMetaData(byte type) {
    this.type = type;
  }

  public boolean isStruct() {
    return type == TType.STRUCT;
  }

  public boolean isContainer() {
    return type == TType.LIST || type == TType.MAP || type == TType.SET;
  }
}
