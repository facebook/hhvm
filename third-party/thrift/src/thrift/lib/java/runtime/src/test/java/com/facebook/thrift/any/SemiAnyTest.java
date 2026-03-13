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

import static org.junit.jupiter.api.Assertions.*;

import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.standard_type.Void;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.List;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class SemiAnyTest {

  private ByteBuf createData() {
    return Unpooled.wrappedBuffer(new byte[] {30});
  }

  @Test
  public void testNullData() {
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              new SemiAny.Builder<>().setData(null).build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Value or data must be provided"));
  }

  @Test
  public void testEmptyData() {
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer(new byte[] {})).build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Value or data must be provided"));
  }

  @Test
  public void testPromoteMissingType() {
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              semiAny.promote(StandardProtocol.COMPACT);
            });
    Assertions.assertTrue(ex.getMessage().contains("Type and data must be provided"));
  }

  @Test
  public void testSetTypeStructAndValueType() {
    TypeStruct type =
        new TypeStruct.Builder().setName(TypeName.fromStringType(Void.UNUSED)).build();
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              SemiAny<List<String>> semiAny =
                  new SemiAny.Builder<>(Arrays.asList("foo"), String.class).setType(type).build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Can not set both type struct and value type"));
  }

  @Test
  public void testSetTypeStructForPrimitiveValue() {
    TypeStruct type =
        new TypeStruct.Builder().setName(TypeName.fromStringType(Void.UNUSED)).build();
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              SemiAny<String> semiAny = new SemiAny.Builder<>("foo").setType(type).build();
            });
    Assertions.assertTrue(
        ex.getMessage().contains("Can not set type struct when value is non container type"));
  }

  @Test
  public void testPromoteNullType() {
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    NullPointerException ex =
        Assertions.assertThrows(
            NullPointerException.class,
            () -> {
              semiAny.promote((StandardProtocol) null);
            });
    Assertions.assertTrue(ex.getMessage().contains("Protocol can not be null"));
  }

  @Test
  public void testPromoteMissingTypeMissingProtocol() {
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              semiAny.promote();
            });
    Assertions.assertTrue(ex.getMessage().contains("Type, protocol and data must be provided"));
  }

  @Test
  public void testPromoteBothDataAndValueSet() {
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              new SemiAny.Builder<>().setData(createData()).setValue("foo").build();
            });
    Assertions.assertTrue(ex.getMessage().contains("Can not set both value and data"));
  }

  @Test
  public void testPromoteNullTypeWithProtocol() {
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    NullPointerException ex =
        Assertions.assertThrows(
            NullPointerException.class,
            () -> {
              semiAny.promote(null, ProtocolUnion.fromStandard(StandardProtocol.COMPACT));
            });
    Assertions.assertTrue(ex.getMessage().contains("TypeStruct can not be null"));
  }

  @Test
  public void testPromoteNullProtocol() {
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    NullPointerException ex =
        Assertions.assertThrows(
            NullPointerException.class,
            () -> {
              semiAny.promote(type, null);
            });
    Assertions.assertTrue(ex.getMessage().contains("ProtocolUnion can not be null"));
  }

  private static ByteBuf mySerializer(Object o) {
    return Unpooled.wrappedBuffer(((String) o).getBytes());
  }

  private static Object myDeserializer(TypeStruct typeStruct, ByteBuf data) {
    byte[] bytes = new byte[data.readableBytes()];
    data.readBytes(bytes);
    return new String(bytes);
  }

  @Test
  public void testGetWithMissingInfo() {
    SemiAny semiAny =
        new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer("foo".getBytes())).build();
    IllegalStateException ex =
        Assertions.assertThrows(
            IllegalStateException.class,
            () -> {
              semiAny.get();
            });
    Assertions.assertTrue(
        ex.getMessage().contains("Type, protocol and data must be provided to get the value"));
  }

  @Test
  public void testPromoteWithCustomSerializerUri() {
    Any.registerSerializer("my-serializer", SemiAnyTest::mySerializer);
    Any.registerDeserializer("my-serializer", SemiAnyTest::myDeserializer);

    SemiAny semiAny =
        new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer("foo".getBytes())).build();
    SemiAny received = SemiAny.wrap(semiAny.getAny());

    Any any =
        semiAny.promote(
            new TypeStruct.Builder().build(), ProtocolUnion.fromCustom("my-serializer"));
    assertEquals("foo", any.get());

    received.getAny().getData().resetReaderIndex();
    any =
        received.promote(
            new TypeStruct.Builder().build(), ProtocolUnion.fromCustom("my-serializer"));
    assertEquals("foo", any.get());
  }

  @Test
  public void testPromoteWithCustomSerializerId() {
    Any.registerSerializer(7L, SemiAnyTest::mySerializer);
    Any.registerDeserializer(7L, SemiAnyTest::myDeserializer);

    SemiAny semiAny =
        new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer("foo".getBytes())).build();
    Any any = semiAny.promote(new TypeStruct.Builder().build(), ProtocolUnion.fromId(7L));
    assertEquals("foo", any.get());
  }
}
