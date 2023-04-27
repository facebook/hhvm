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
import com.facebook.thrift.type.HashAlgorithmSHA256;
import com.facebook.thrift.type_swift.TypeStruct;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Base builder object for Any and SemiAny.
 *
 * @param <T> The type which Any and SemiAny holds
 * @param <A> AnyStruct or SemiAnyStruct
 */
public abstract class AnyBuilder<T, A extends AbstractAny> {
  protected T value;

  protected String customProtocolUri;

  protected Long customProtocolId;

  protected StandardProtocol standardProtocol;

  protected boolean useHashPrefix;

  protected boolean useUri;

  protected List<Class> elementTypeList = new ArrayList<>();

  protected int numberOfBytes = HashAlgorithmSHA256.INSTANCE.getMinHashBytes();

  protected TypeStruct typeStruct;

  /** Any and SemiAny builder */
  public <T, A> AnyBuilder() {}

  public AnyBuilder(T value) {
    this.value = value;
  }

  public AnyBuilder(T value, Class... clazz) {
    this.value = value;
    elementTypeList.addAll(Arrays.asList(clazz));
  }

  /** Payload of Any and SemiAny */
  public AnyBuilder<T, A> setValue(T value) {
    this.value = value;
    return this;
  }

  /** External custom protocol id. */
  public AnyBuilder<T, A> setCustomProtocol(Long customProtocolId) {
    this.customProtocolId = customProtocolId;
    return this;
  }

  /** Serialization protocol of Any and SemiAny payload when standard protocol is used. */
  public AnyBuilder<T, A> setProtocol(StandardProtocol standardProtocol) {
    this.standardProtocol = standardProtocol;
    return this;
  }

  /** Custom serialization protocol uri of Any and SemiAny payload. */
  public AnyBuilder<T, A> setCustomProtocol(String customProtocolUri) {
    this.customProtocolUri = customProtocolUri;
    return this;
  }

  /**
   * Use hash prefix to identify object types. Any and SemiAny uses 8 bytes of hash for the object
   * type by default. Optional parameter, when not set, hash or uri of the type is set automatically
   * based on the length of these identifiers.
   */
  public AnyBuilder<T, A> useHashPrefix() {
    this.useHashPrefix = true;
    return this;
  }

  /**
   * Use numberOfBytes length hash prefix to identify object types. Optional parameter, when not
   * set, hash or uri of the type is set automatically based on the length of these identifiers.
   */
  public AnyBuilder<T, A> useHashPrefix(int numberOfBytes) {
    this.useHashPrefix = true;
    this.numberOfBytes = numberOfBytes;
    return this;
  }

  /**
   * Use uri to identify object types. Optional parameter, when not set, hash or uri of the type is
   * set automatically based on the length of these identifiers.
   */
  public AnyBuilder<T, A> useUri() {
    this.useUri = true;
    return this;
  }

  private boolean isContainerType() {
    return value instanceof List || value instanceof Map || value instanceof Set;
  }

  protected void validateCommon() {
    if ((standardProtocol != null && customProtocolUri != null)
        || (standardProtocol != null && customProtocolId != null)
        || (customProtocolUri != null && customProtocolId != null)) {
      throw new IllegalStateException("Can not set multiple protocol");
    }
    if (useHashPrefix && useUri) {
      throw new IllegalStateException("Can not set both useHashPrefix and useUri");
    }
    if (numberOfBytes < 1) {
      throw new IllegalArgumentException("Number of hash prefix must be at least 1");
    }
    if (typeStruct == null) {
      if (value instanceof List && elementTypeList.isEmpty()) {
        throw new IllegalArgumentException("List element type class is not provided");
      }
      if (value instanceof Set && elementTypeList.isEmpty()) {
        throw new IllegalArgumentException("Set element type class is not provided");
      }
      if (value instanceof Map && elementTypeList.isEmpty()) {
        throw new IllegalArgumentException(
            "Map key and value element type classes are not provided");
      }
    }
  }

  protected boolean isProtocolSet() {
    return standardProtocol != null || customProtocolUri != null || customProtocolId != null;
  }

  public abstract A build();
}
