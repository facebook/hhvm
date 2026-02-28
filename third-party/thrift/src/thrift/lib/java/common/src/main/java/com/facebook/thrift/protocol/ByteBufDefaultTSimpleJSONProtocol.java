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

package com.facebook.thrift.protocol;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import org.apache.thrift.TException;

public class ByteBufDefaultTSimpleJSONProtocol extends ByteBufAbstractTSimpleJSONProtocol {

  private static final int EMPTY_STRUCT_SIZE = 3;

  @Override
  public void writeBinary(ByteBuffer bin) throws TException {
    String data;
    if (bin.hasArray()) {
      data =
          new String(
              bin.array(),
              bin.position() + bin.arrayOffset(),
              bin.limit() - bin.position() - bin.arrayOffset(),
              StandardCharsets.UTF_8);
    } else {
      byte[] buffer = new byte[bin.remaining()];
      bin.get(buffer);
      data = new String(buffer, StandardCharsets.UTF_8);
    }
    writeString(data);
  }

  @Override
  public ByteBuffer readBinary() throws TException {
    return ByteBuffer.wrap(readString().getBytes(StandardCharsets.UTF_8));
  }

  public int getEmptyStructSize() {
    return EMPTY_STRUCT_SIZE;
  }
}
