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

import java.util.HashMap;
import java.util.Map;

/** Helper class that encapsulates field metadata. */
public class TField {
  private static final Map<String, Object> EMPTY_METADATA = new HashMap();

  public TField() {
    this("", TType.STOP, (short) 0);
  }

  public TField(String name, byte type, short id) {
    this(name, type, id, EMPTY_METADATA);
  }

  public TField(String name, byte type, short id, Map<String, Object> metadata) {
    this.name = name;
    this.type = type;
    this.id = id;
    this.metadata = metadata;
  }

  public final String name;
  public final byte type;
  public final short id;
  public final Map<String, Object> metadata;

  public String toString() {
    return String.format(
        "<TField name:'%s' type:%d field-id:%d metadata='%s'>", name, type, id, metadata);
  }

  public boolean equals(TField otherField) {
    return type == otherField.type && id == otherField.id;
  }
}
