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

import static org.junit.Assert.*;

import com.facebook.thrift.standard_type.StandardProtocol;
import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.standard_type.Void;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.List;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class SemiAnyTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  private ByteBuf createData() {
    return Unpooled.wrappedBuffer(new byte[] {30});
  }

  @Test
  public void testNullData() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Value or data must be provided");
    new SemiAny.Builder<>().setData(null).build();
  }

  @Test
  public void testEmptyData() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Value or data must be provided");
    new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer(new byte[] {})).build();
  }

  @Test
  public void testPromoteMissingType() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Type and data must be provided");
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    semiAny.promote(StandardProtocol.COMPACT);
  }

  @Test
  public void testSetTypeStructAndValueType() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Can not set both type struct and value type");
    TypeStruct type =
        new TypeStruct.Builder().setName(TypeName.fromStringType(Void.UNUSED)).build();
    SemiAny<List<String>> semiAny =
        new SemiAny.Builder<>(Arrays.asList("foo"), String.class).setType(type).build();
  }

  @Test
  public void testSetTypeStructForPrimitiveValue() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Can not set type struct when value is non container type");
    TypeStruct type =
        new TypeStruct.Builder().setName(TypeName.fromStringType(Void.UNUSED)).build();
    SemiAny<String> semiAny = new SemiAny.Builder<>("foo").setType(type).build();
  }

  @Test
  public void testPromoteNullType() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("Protocol can not be null");
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    semiAny.promote((StandardProtocol) null);
  }

  @Test
  public void testPromoteMissingTypeMissingProtocol() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Type, protocol and data must be provided");
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    semiAny.promote();
  }

  @Test
  public void testPromoteBothDataAndValueSet() {
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Can not set both value and data");
    new SemiAny.Builder<>().setData(createData()).setValue("foo").build();
  }

  @Test
  public void testPromoteNullTypeWithProtocol() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("TypeStruct can not be null");
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    semiAny.promote(null, ProtocolUnion.fromStandard(StandardProtocol.COMPACT));
  }

  @Test
  public void testPromoteNullProtocol() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("ProtocolUnion can not be null");
    ByteBuf data = createData();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    semiAny.promote(type, null);
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
    expectedException.expect(IllegalStateException.class);
    expectedException.expectMessage("Type, protocol and data must be provided to get the value");
    SemiAny semiAny =
        new SemiAny.Builder<>().setData(Unpooled.wrappedBuffer("foo".getBytes())).build();
    semiAny.get();
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
