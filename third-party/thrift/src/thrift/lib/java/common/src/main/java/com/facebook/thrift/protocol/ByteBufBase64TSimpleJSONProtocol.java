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
import java.nio.CharBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import org.apache.thrift.TException;

public class ByteBufBase64TSimpleJSONProtocol extends ByteBufAbstractTSimpleJSONProtocol {

  private static final int EMPTY_STRUCT_SIZE = 3;

  @Override
  public void writeBinary(ByteBuffer bin) throws TException {
    ByteBuffer encode = Base64.getEncoder().withoutPadding().encode(bin);
    CharBuffer decode = StandardCharsets.UTF_8.decode(encode);
    writeString(decode.toString());
  }

  @Override
  public ByteBuffer readBinary() throws TException {
    byte[] decode = Base64.getDecoder().decode(readString());
    return ByteBuffer.wrap(decode);
  }

  public int getEmptyStructSize() {
    return EMPTY_STRUCT_SIZE;
  }
}
