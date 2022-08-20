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

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class HashAlgorithmSHA256 implements HashAlgorithm {
  public static final HashAlgorithmSHA256 INSTANCE = new HashAlgorithmSHA256();

  private static final String THRIFT_SCHEME = "fbthrift://";

  private HashAlgorithmSHA256() {}

  @Override
  public int getMinHashBytes() {
    return 8;
  }

  @Override
  public int getDefaultHashBytes() {
    return 16;
  }

  @Override
  public ByteBuf generateHash(String uri) {
    try {
      MessageDigest digest = MessageDigest.getInstance("SHA-256");
      return Unpooled.wrappedBuffer(
          digest.digest((THRIFT_SCHEME + uri).getBytes(StandardCharsets.UTF_8)),
          0,
          getDefaultHashBytes());
    } catch (NoSuchAlgorithmException e) {
      throw new RuntimeException(e);
    }
  }
}
