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

package com.facebook.thrift.adapter.common;

import com.facebook.thrift.adapter.TypeAdapter;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.payload.ThriftSerializable;
import org.apache.thrift.protocol.TProtocol;

public abstract class StructTypeAdapter<T, K extends ThriftSerializable> implements TypeAdapter<T> {
  @Override
  public final void toThrift(T t, TProtocol protocol) {
    to(t).write0(protocol);
  }

  @Override
  public final T fromThrift(TProtocol protocol) {
    final K read = getReader().read(protocol);
    return from(read);
  }

  protected abstract Reader<K> getReader();

  protected abstract T from(K k);

  protected abstract K to(T t);
}
