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

package com.facebook.thrift.wrapper.test;

import com.facebook.thrift.adapter.FieldContext;
import com.facebook.thrift.payload.ThriftSerializable;
import java.lang.reflect.Method;

public class PoliciedField<T> {
  private T t;

  private FieldContext<ThriftSerializable> fieldContext;

  private Integer getContext(Object o) {
    try {
      Method m = o.getClass().getDeclaredMethod("getContext");
      return (Integer) m.invoke(o);
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  public PoliciedField(T value, FieldContext<ThriftSerializable> fieldContext) {
    this.t = value;
    this.fieldContext = fieldContext;
  }

  public PoliciedField(T value) {
    this.t = value;
  }

  public T getValue() {
    return t;
  }

  public T getZonedValue() {
    return getContext(fieldContext.getRootObj()) == 0 ? null : t;
  }

  public FieldContext<ThriftSerializable> getFieldContext() {
    return fieldContext;
  }

  @Override
  public boolean equals(Object other) {
    if (other instanceof PoliciedField<?>) {
      if (((PoliciedField<?>) other).t == null && t == null) {
        return true;
      }
      if (((PoliciedField<?>) other).t == null) {
        return false;
      }
      if (((PoliciedField<?>) other).t.equals(t)) {
        return true;
      }
    }
    return false;
  }
}
