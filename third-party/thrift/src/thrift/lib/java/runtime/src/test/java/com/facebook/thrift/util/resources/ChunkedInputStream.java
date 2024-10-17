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

package com.facebook.thrift.util.resources;

import static java.lang.Math.min;

import java.io.IOException;
import java.io.InputStream;

/**
 * Test class that can only read 3 bytes at time on the read(byte[], ...) to simulate input streams
 * for framed types such as snappy compression. This allows testing of specific range checks in the
 * read path
 */
public class ChunkedInputStream extends InputStream {
  private final InputStream is;

  public ChunkedInputStream(InputStream is) {
    this.is = is;
  }

  @Override
  public int read() throws IOException {
    return is.read();
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    return is.read(b, off, min(3, len));
  }
}
