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

package com.facebook.thrift.util;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.Unpooled;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.zip.DeflaterInputStream;
import org.junit.Assert;
import org.junit.Test;

public class CompressionUtilTest {
  @Test
  public void testInflateByteArray() throws Exception {
    InputStream s = Thread.currentThread().getContextClassLoader().getResourceAsStream("lorem.txt");

    DeflaterInputStream dis = new DeflaterInputStream(s);
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    byte[] buf = new byte[1024];
    int len;
    while ((len = dis.read(buf)) > 0) bos.write(buf, 0, len);
    dis.close();
    bos.close();

    ByteBufAllocator alloc = ByteBufAllocator.DEFAULT;
    byte[] bytes = bos.toByteArray();
    ByteBuf in = Unpooled.wrappedBuffer(bytes);
    ByteBuf out = CompressionUtil.inflate(alloc, in);

    String deflatedText = out.toString(StandardCharsets.UTF_8);

    s = Thread.currentThread().getContextClassLoader().getResourceAsStream("lorem.txt");

    bos = new ByteArrayOutputStream();
    assert s != null;
    while ((len = s.read(buf)) > 0) bos.write(buf, 0, len);

    String text = new String(bos.toByteArray(), StandardCharsets.UTF_8);

    Assert.assertEquals(text, deflatedText);
  }

  @Test
  public void testInflateDirectMemory() throws Exception {
    ByteBufAllocator alloc = ByteBufAllocator.DEFAULT;
    ByteBuf in = alloc.buffer();
    try {
      InputStream s =
          Thread.currentThread().getContextClassLoader().getResourceAsStream("lorem.txt");

      DeflaterInputStream dis = new DeflaterInputStream(s);
      ByteArrayOutputStream bos = new ByteArrayOutputStream();
      byte[] buf = new byte[1024];
      int len;
      while ((len = dis.read(buf)) > 0) bos.write(buf, 0, len);
      dis.close();
      bos.close();

      byte[] bytes = bos.toByteArray();
      in.writeBytes(bytes);
      ByteBuf out = CompressionUtil.inflate(alloc, in);

      String deflatedText = out.toString(StandardCharsets.UTF_8);

      s = Thread.currentThread().getContextClassLoader().getResourceAsStream("lorem.txt");

      bos = new ByteArrayOutputStream();
      assert s != null;
      while ((len = s.read(buf)) > 0) bos.write(buf, 0, len);

      String text = new String(bos.toByteArray(), StandardCharsets.UTF_8);

      Assert.assertEquals(text, deflatedText);
    } finally {
      in.release();
    }
  }
}
