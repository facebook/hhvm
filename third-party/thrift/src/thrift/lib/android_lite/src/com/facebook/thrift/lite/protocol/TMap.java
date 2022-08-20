/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package com.facebook.thrift.lite.protocol;

/** Helper class that encapsulates map metadata. */
public final class TMap {

  /* package */ final byte keyType;
  /* package */ final byte valueType;
  /* package */ final int size;

  public TMap() {
    this(TType.STOP, TType.STOP, 0);
  }

  public TMap(byte k, byte v, int s) {
    keyType = k;
    valueType = v;
    size = s;
  }
}
