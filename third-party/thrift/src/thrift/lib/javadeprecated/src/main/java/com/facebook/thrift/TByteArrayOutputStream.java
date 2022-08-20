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

package com.facebook.thrift;

import java.io.ByteArrayOutputStream;
import java.nio.charset.Charset;

/** Class that allows access to the underlying buf without doing deep copies on it. */
public class TByteArrayOutputStream extends ByteArrayOutputStream {
  public TByteArrayOutputStream(int size) {
    super(size);
  }

  public TByteArrayOutputStream() {
    super();
  }

  public byte[] get() {
    return buf;
  }

  public int len() {
    return count;
  }

  public String toString(Charset charset) {
    return new String(buf, 0, count, charset);
  }
}
