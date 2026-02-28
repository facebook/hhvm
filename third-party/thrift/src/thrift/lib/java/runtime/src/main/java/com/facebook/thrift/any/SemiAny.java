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
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.SemiAnyStruct;
import com.facebook.thrift.type_swift.TypeStruct;
import io.netty.buffer.ByteBuf;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Helper class for {@link SemiAnyStruct}. This class hides the internal complexity of SemiAnyStruct
 * and is the recommended approach to work with. For values requiring uri or hash prefix, if useUri
 * or useHashPrefix is not set in builder object, whichever is the shortest will be set as type
 * identifier. Users can override this behavior by setting either useUri or useHashPrefix. This
 * applies for all nested values.
 *
 * <p>SemiAnyStruct can be created through Builder object with the given value. {@link #getAny()}
 * API call will return the struct. This class can also wrap SemiAnyStruct and extract the value
 * with {@link #get()} API. This call is lazy, object is created only when the get method is called.
 *
 * <p>SemiAny can be promoted to {@link Any} by providing missing protocol and/or type information.
 *
 * @param <T>
 */
public abstract class SemiAny<T> extends AbstractAny<T, SemiAnyStruct> {

  /**
   * Creates a SemiAny by wrapping {@link SemiAnyStruct}. Any value can be extracted with {@link
   * #get()} API.
   *
   * @param any {@link SemiAnyStruct}
   * @return SemiAny
   * @param <T>
   */
  public static <T> SemiAny<T> wrap(SemiAnyStruct any) {
    return new SerializedSemiAny<>(any);
  }

  /**
   * If SemiAny is created via Builder object, a SemiAnyStruct is created from given value. If
   * SemiAny is created via wrap API, wrapped SemiAnyStruct is returned.
   *
   * @return {@link SemiAnyStruct}
   */
  public abstract SemiAnyStruct getAny();

  /**
   * Promotes SemiAny to {@link Any}. All fields - type, protocol, data - must be already provided.
   *
   * @return {@link Any}
   */
  public abstract Any<T> promote();

  /**
   * Promotes SemiAny to {@link Any} by providing standard protocol.
   *
   * @return {@link Any}
   */
  public abstract Any<T> promote(StandardProtocol protocol);

  /**
   * Promotes SemiAny to {@link Any} by providing protocol union.
   *
   * @return {@link Any}
   */
  public abstract Any<T> promote(ProtocolUnion protocolUnion);

  /**
   * Promotes SemiAny to {@link Any} by providing type struct and protocol union.
   *
   * @return {@link Any}
   */
  public abstract Any<T> promote(TypeStruct typeStruct, ProtocolUnion protocolUnion);

  protected SemiAny() {}

  /**
   * Builder object to create SemiAny.
   *
   * @param <T>
   */
  public static final class Builder<T> extends AnyBuilder<T, SemiAny<T>> {

    public <T> Builder() {}

    public Builder(T value, Class... clazz) {
      super(value, clazz);
    }

    public Builder(T value) {
      super(value);
    }

    protected ByteBuf data;

    /**
     * Serialized SemiAny data is set.
     *
     * @param data serialized data
     * @return Builder object
     */
    public Builder<T> setData(ByteBuf data) {
      this.data = data;
      return this;
    }

    /**
     * TypeStruct of the SemiAny.
     *
     * @param {@link com.facebook.thrift.type_swift.TypeStruct}
     * @return Builder object
     */
    public Builder<T> setType(TypeStruct typeStruct) {
      this.typeStruct = typeStruct;
      return this;
    }

    private boolean isContainerType() {
      return value instanceof List || value instanceof Map || value instanceof Set;
    }

    protected void validate() {
      if (value == null && (data == null || data.readableBytes() == 0)) {
        throw new IllegalStateException("Value or data must be provided");
      }
      if (value != null && data != null) {
        throw new IllegalStateException("Can not set both value and data");
      }
      if (typeStruct != null) {
        if (!elementTypeList.isEmpty()) {
          throw new IllegalStateException("Can not set both type struct and value type");
        }
        if (value != null && !isContainerType()) {
          throw new IllegalStateException(
              "Can not set type struct when value is non container type");
        }
      }

      validateCommon();
    }

    @Override
    public SemiAny<T> build() {
      validate();
      if (value != null) {
        return new UnserializedSemiAny<T>(this);
      } else {
        SemiAnyStruct.Builder builder = new SemiAnyStruct.Builder().setData(data);
        if (typeStruct != null) {
          builder.setType(typeStruct);
        }
        if (standardProtocol != null) {
          builder.setProtocol(ProtocolUnion.fromStandard(standardProtocol));
        }
        if (customProtocolUri != null) {
          builder.setProtocol(ProtocolUnion.fromCustom(customProtocolUri));
        }
        if (customProtocolId != null) {
          builder.setProtocol(ProtocolUnion.fromId(customProtocolId));
        }
        return new SerializedSemiAny<T>(builder.build());
      }
    }
  }
}
