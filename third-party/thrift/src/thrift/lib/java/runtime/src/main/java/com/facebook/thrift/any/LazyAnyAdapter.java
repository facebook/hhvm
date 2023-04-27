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

import com.facebook.thrift.adapter.TypeAdapter;
import org.apache.thrift.conformance.Any;

/**
 * A {@link TypeAdapter} from serializing and deserialize a {@link com.facebook.thrift.any.LazyAny}
 */
public class LazyAnyAdapter implements TypeAdapter<Any, LazyAny> {
  @Override
  public LazyAny fromThrift(Any any) {
    return new SerializedLazyAny(any);
  }

  @Override
  public Any toThrift(LazyAny lazyAny) {
    if (lazyAny == null) {
      return null;
    }
    return lazyAny.getAny();
  }
}
