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

import static org.junit.Assert.assertEquals;

import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.any.Circle;
import com.facebook.thrift.test.any.Image;
import com.facebook.thrift.test.any.Position;
import com.facebook.thrift.test.any.Rectangle;
import com.facebook.thrift.test.any.SolidColor;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.util.Arrays;
import java.util.List;
import org.junit.Test;

public class LazyAnyTest {

  @Test
  public void testSimpleAny() {
    LazyAny<SolidColor> lazyAny =
        new LazyAny.Builder(new SolidColor.Builder().setColor(2).build()).build();

    Rectangle rectangle =
        new Rectangle.Builder().setWidth(10).setLen(20).setCanvas(lazyAny).build();

    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TBinary, dest);

    rectangle.write0(protocol);
    Rectangle received = Rectangle.read0(protocol);

    assertEquals(rectangle.getWidth(), received.getWidth());
    assertEquals(rectangle.getLen(), received.getLen());
    assertEquals(2, ((SolidColor) received.getCanvas().get()).getColor());
  }

  @Test
  public void testLazyAnyList() {
    LazyAny<Circle> circle =
        new LazyAny.Builder(new Circle.Builder().setColor(2).setRadius(5).build()).build();
    LazyAny<Image> image =
        new LazyAny.Builder(new Image.Builder().setJpg(new byte[] {1, 2, 3}).build()).build();
    LazyAny<Rectangle> rect =
        new LazyAny.Builder(
                new Rectangle.Builder()
                    .setColor(2)
                    .setWidth(100)
                    .setLen(200)
                    .setPosition(new Position.Builder().setX(5).setY(6).build())
                    .setCanvas(image)
                    .build())
            .build();

    // todo: Type Adapters is not support in container types. Fix the test when supported.
    List<LazyAny> list = Arrays.asList(circle, rect);

    //    Drawing drawing = new Drawing.Builder().setName("draw1").setShapes(list).build();
    //
    //    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    //    ByteBufTProtocol protocol =
    //        SerializerUtil.toByteBufProtocol(SerializationProtocol.TBinary, dest);
    //
    //    drawing.write0(protocol);
    //    Drawing received = Drawing.read0(protocol);
    //
    //    assertEquals(drawing.getName(), received.getName());
    //    assertEquals(drawing.getShapes().size(), received.getShapes().size());
    //
    //    assertEquals(
    //        5, ((Circle) new SerializedLazyAny(received.getShapes().get(0)).get()).getRadius());
  }
}
