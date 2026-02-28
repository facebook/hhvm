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

package com.facebook.thrift.type;

import com.facebook.thrift.payload.Reader;

public class Type<T> {

  private UniversalName universalName;
  private Class<T> clazz;
  private Reader<T> reader;

  public Type(UniversalName universalName, Class<T> clazz, Reader<T> reader) {
    this.universalName = universalName;
    this.clazz = clazz;
    this.reader = reader;
  }

  public UniversalName getUniversalName() {
    return universalName;
  }

  public Class<T> getClazz() {
    return clazz;
  }

  public Reader<T> getReader() {
    return reader;
  }

  @Override
  public String toString() {
    return "Type{" + "universalName=" + universalName + ", class=" + clazz + '}';
  }
}
