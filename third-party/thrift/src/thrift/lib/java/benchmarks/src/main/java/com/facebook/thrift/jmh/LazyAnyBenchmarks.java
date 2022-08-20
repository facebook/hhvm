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

package com.facebook.thrift.jmh;

import com.facebook.thrift.any.LazyAny;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.any.Image;
import com.facebook.thrift.test.any.Rectangle;
import com.facebook.thrift.test.any.SolidColor;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Level;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;
import org.openjdk.jmh.infra.Blackhole;

public class LazyAnyBenchmarks {

  private static final byte[] JPG_IMAGE = new byte[5000];

  @State(Scope.Benchmark)
  public static class Input {
    private Blackhole bh;

    @Param({"TBinary", "TCompact"})
    String protocol;

    @Param({"uri", "hash"})
    String type;

    @Param({"small", "medium"})
    String size;

    @Setup(Level.Trial)
    public void setup(Blackhole bh) {
      this.bh = bh;
      this.target = RpcResources.getUnpooledByteBufAllocator().buffer();
      this.rectangleBytes = RpcResources.getUnpooledByteBufAllocator().buffer();
      this.byteBufTProtocols =
          SerializerUtil.toByteBufProtocol(getProtocol(protocol), this.rectangleBytes);

      setupLazyAny();
    }

    ByteBuf target;
    ByteBuf rectangleBytes;
    ByteBufTProtocol byteBufTProtocols;

    @TearDown
    public void teardown() {
      target.release();
      rectangleBytes.release();
    }

    private void setupLazyAny() {
      LazyAny.Builder builder = new LazyAny.Builder(getCanvas(this.size));
      if ("uri".equals(this.type)) {
        builder.useUri();
      }

      LazyAny<SolidColor> canvas = builder.build();

      Rectangle rectangle =
          new Rectangle.Builder().setWidth(10).setLen(20).setCanvas(canvas).build();

      ByteBufTProtocol protocol =
          SerializerUtil.toByteBufProtocol(getProtocol(this.protocol), this.rectangleBytes);

      rectangle.write0(protocol);
    }
  }

  private static SerializationProtocol getProtocol(String protocol) {
    switch (protocol) {
      case "TBinary":
        return SerializationProtocol.TBinary;
      case "TCompact":
        return SerializationProtocol.TCompact;
    }
    throw new RuntimeException("Unknown protocol, " + protocol);
  }

  private static Object getCanvas(String size) {
    if ("small".equals(size)) return new SolidColor.Builder().setColor(2).build();

    return new Image.Builder().setJpg(JPG_IMAGE).build();
  }

  @Benchmark
  public void benchmarkLazyAnySerialize(Input input) {
    input.target.clear();
    LazyAny.Builder builder = new LazyAny.Builder(getCanvas(input.size));
    if ("uri".equals(input.type)) {
      builder.useUri();
    }

    LazyAny<SolidColor> canvas = builder.build();

    Rectangle rectangle = new Rectangle.Builder().setWidth(10).setLen(20).setCanvas(canvas).build();

    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(getProtocol(input.protocol), input.target);

    rectangle.write0(protocol);
    input.bh.consume(protocol);
  }

  @Benchmark
  public void benchmarkLazyAnyDeserialize(Input input) {
    input.rectangleBytes.markReaderIndex();
    Rectangle received = Rectangle.read0(input.byteBufTProtocols);
    if ("small".equals(input.size)) {
      int color = ((SolidColor) received.getCanvas().get()).getColor();
      input.bh.consume(color);
    } else {
      byte[] img = ((Image) received.getCanvas().get()).getJpg();
      input.bh.consume(img);
    }

    input.bh.consume(received);
    input.rectangleBytes.resetReaderIndex();
  }
}
