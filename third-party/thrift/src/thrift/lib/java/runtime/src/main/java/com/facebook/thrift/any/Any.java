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

package com.facebook.thrift.any;

import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.type_swift.AnyStruct;
import java.util.Objects;

/**
 * Helper class for {@link AnyStruct}. This class hides the internal complexity of AnyStruct and is
 * the recommended approach to work with. Default serialization protocol is Compact if not provided.
 * For values requiring uri or hash prefix, if useUri or useHashPrefix is not set in builder object,
 * whichever is the shortest will be set as type identifier. Users can override this behavior by
 * setting either useUri or useHashPrefix. This applies for all nested values.
 *
 * <p>AnyStruct can be created through Builder object with the given value. {@link #getAny()} API
 * call will return the struct. This class can also wrap AnyStruct and extract the value with {@link
 * #get()} API. This call is lazy, object is created only when the get method is called.
 *
 * @param <T>
 */
public abstract class Any<T> extends AbstractAny<T, AnyStruct> {

  /**
   * Creates an Any by wrapping {@link AnyStruct}. Any value can be extracted with {@link #get()}
   * API.
   *
   * @param any {@link AnyStruct}
   * @return Any
   * @param <T>
   */
  public static <T> Any<T> wrap(AnyStruct any) {
    return new SerializedAny(any);
  }

  /**
   * If Any is created via Builder object, an AnyStruct is created from given value. If Any is
   * created via wrap API, wrapped AnyStruct is returned.
   *
   * @return {@link AnyStruct}
   */
  public abstract AnyStruct getAny();

  /**
   * Builder object to create Any.
   *
   * @param <T>
   */
  public static final class Builder<T> extends AnyBuilder<T, UnserializedAny<T>> {

    public Builder(T value, Class... clazz) {
      super(value, clazz);
    }

    public Builder(T value) {
      super(value);
    }

    protected void validate() {
      Objects.requireNonNull(value, "Any must have a value");
      validateCommon();
    }

    @Override
    public UnserializedAny<T> build() {
      validate();
      if (!isProtocolSet()) {
        this.standardProtocol = StandardProtocol.COMPACT;
      }
      return new UnserializedAny<>(this);
    }
  }
}
