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
import com.facebook.thrift.test.thrift.any.TestStruct;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import com.facebook.thrift.util.SerializationProtocolUtil;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Objects;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class SemiAnyStandardProtocolTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(
        StandardProtocol.COMPACT,
        StandardProtocol.BINARY,
        StandardProtocol.JSON,
        StandardProtocol.SIMPLE_JSON);
  }

  private final StandardProtocol standardProtocol;

  public SemiAnyStandardProtocolTest(StandardProtocol standardProtocol) {
    this.standardProtocol = standardProtocol;
  }

  private <T> SemiAny<T> createSemiAny(T o) {
    return new SemiAny.Builder<>(o).setProtocol(standardProtocol).build();
  }

  private <T> SemiAny<T> createSemiAny(T o, Class... clazz) {
    return new SemiAny.Builder<>(o, clazz).setProtocol(standardProtocol).build();
  }

  private <T> TypeName getType(SemiAny<T> any) {
    return Objects.requireNonNull(any.getAny().getType()).getName();
  }

  private <T> TypeName getType(Any<T> any) {
    return Objects.requireNonNull(any.getAny().getType()).getName();
  }

  private <T> void assertParamsEmpty(SemiAny<T> any) {
    assertNull(Objects.requireNonNull(any.getAny().getType()).getParams());
  }

  private <T> void assertParamsEmpty(Any<T> any) {
    assertNull(Objects.requireNonNull(any.getAny().getType()).getParams());
  }

  private <T> TypeName getParams(SemiAny<T> any, int inx) {
    return any.getAny().getType().getParams().get(inx).getName();
  }

  @Test
  public void testBool() {
    SemiAny<Boolean> any = createSemiAny(true);
    assertEquals(true, any.get());
    assertTrue(getType(any).isSetBoolType());
    assertParamsEmpty(any);

    SemiAny<Boolean> received = SemiAny.wrap(any.getAny());
    assertEquals(true, received.get());
    assertTrue(getType(received).isSetBoolType());
    assertParamsEmpty(received);
  }

  @Test
  public void testInteger() {
    SemiAny<Integer> any = createSemiAny(500);
    assertEquals(500, (int) any.get());
    assertTrue(getType(any).isSetI32Type());
    assertParamsEmpty(any);

    SemiAny<Integer> received = SemiAny.wrap(any.getAny());
    assertEquals(500, (int) received.get());
    assertTrue(getType(received).isSetI32Type());
    assertParamsEmpty(received);
  }

  private void assertListOfString(SemiAny<List<String>> any) {
    assertEquals(3, any.get().size());
    assertArrayEquals(new String[] {"foo", "bar", "baz"}, any.get().toArray());
    assertTrue(getType(any).isSetListType());
    assertTrue(getParams(any, 0).isSetStringType());
  }

  @Test
  public void testListOfString() {
    SemiAny<List<String>> any = createSemiAny(Arrays.asList("foo", "bar", "baz"), String.class);
    assertListOfString(any);

    SemiAny<List<String>> received = SemiAny.wrap(any.getAny());
    assertListOfString(received);
  }

  @Test
  public void testPromoteFromValue() {
    SemiAny<Integer> semiAny = createSemiAny(500);
    assertEquals(500, (int) semiAny.get());
    assertTrue(getType(semiAny).isSetI32Type());
    assertParamsEmpty(semiAny);

    Any<Integer> any = semiAny.promote();
    assertEquals(500, (int) any.get());
    assertTrue(getType(any).isSetI32Type());
    assertParamsEmpty(any);
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
  }

  private ByteBuf createData() {
    return Unpooled.wrappedBuffer(new byte[] {30});
  }

  @Test
  public void testPromoteFromData() {
    ByteBuf data = createData();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    SemiAny semiAny =
        new SemiAny.Builder<>().setData(data).setType(type).setProtocol(standardProtocol).build();

    Any<Integer> any = semiAny.promote();
    assertEquals(ByteBufUtil.hexDump(data), ByteBufUtil.hexDump(any.getAny().getData()));
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
  }

  @Test
  public void testPromoteFromDataWithProtocol() {
    ByteBuf data = createData();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).setType(type).build();

    Any<Integer> any = semiAny.promote(standardProtocol);
    assertEquals(ByteBufUtil.hexDump(data), ByteBufUtil.hexDump(any.getAny().getData()));
  }

  @Test
  public void testPromoteFromListWithProtocol() {
    List<Integer> list = new ArrayList<>();
    list.add(7);

    SemiAny<List<Integer>> semiAny = new SemiAny.Builder<>(list, Integer.class).build();
    assertTrue(getType(semiAny).isSetListType());
    assertTrue(getParams(semiAny, 0).isSetI32Type());

    Any<List<Integer>> any = semiAny.promote(standardProtocol);
    assertTrue(getType(any).isSetListType());
    assertTrue(any.getAny().getType().getParams().get(0).getName().isSetI32Type());
    assertEquals(7, (long) any.get().get(0));
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
  }

  @Test
  public void testPromoteFromDataWithProtocolUnion() {
    ByteBuf data = createData();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).setType(type).build();

    Any<Integer> any = semiAny.promote(ProtocolUnion.fromStandard(standardProtocol));
    assertEquals(ByteBufUtil.hexDump(data), ByteBufUtil.hexDump(any.getAny().getData()));
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
  }

  @Test
  public void testPromoteFromValueWithProtocolUnion() {
    TypeStruct type =
        new TypeStruct.Builder().setName(TypeName.fromStringType(Void.UNUSED)).build();
    SemiAny<String> semiAny = new SemiAny.Builder<>("foo").build();

    Any<String> any = semiAny.promote(ProtocolUnion.fromStandard(standardProtocol));
    assertEquals("foo", any.get());
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
    assertEquals(type, any.getAny().getType());
  }

  @Test
  public void testPromoteFromDataWithTypeAndProtocolUnion() {
    ByteBuf data = createData();
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    SemiAny semiAny = new SemiAny.Builder<>().setData(data).build();

    Any<Integer> any = semiAny.promote(type, ProtocolUnion.fromStandard(standardProtocol));
    assertEquals(ByteBufUtil.hexDump(data), ByteBufUtil.hexDump(any.getAny().getData()));
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
    assertEquals(type, any.getAny().getType());
  }

  @Test
  public void testPromoteFromValueWithTypeAndProtocolUnion() {
    TypeStruct type = new TypeStruct.Builder().setName(TypeName.fromI32Type(Void.UNUSED)).build();
    SemiAny<List<String>> semiAny =
        new SemiAny.Builder<>(Arrays.asList("foo"), String.class).build();

    Any<List<String>> any = semiAny.promote(type, ProtocolUnion.fromStandard(standardProtocol));
    assertEquals("foo", any.get().get(0));
    assertEquals(standardProtocol, any.getAny().getProtocol().getStandard());
    assertEquals(type, any.getAny().getType());
  }

  @Test
  public void testSemiAnyAdapter() {
    SemiAny<List<Integer>> any = createSemiAny(Arrays.asList(5, 6, 7), Integer.class);
    TestStruct st = new TestStruct.Builder().setSemianyField(any).build();

    byte[] bytes =
        SerializerUtil.toByteArray(st, SerializationProtocolUtil.getProtocol(standardProtocol));
    TestStruct received =
        SerializerUtil.fromByteArray(
            TestStruct.asReader(), bytes, SerializationProtocolUtil.getProtocol(standardProtocol));
    assertArrayEquals(
        new Integer[] {5, 6, 7}, ((List<Integer>) received.getSemianyField().get()).toArray());
  }

  @Test
  public void testSemiAnyAdapterPromoteToAny() {
    SemiAny<List<Integer>> semiAny = createSemiAny(Arrays.asList(5, 6, 7), Integer.class);
    TestStruct st = new TestStruct.Builder().setSemianyField(semiAny).build();

    byte[] bytes =
        SerializerUtil.toByteArray(st, SerializationProtocolUtil.getProtocol(standardProtocol));
    TestStruct received =
        SerializerUtil.fromByteArray(
            TestStruct.asReader(), bytes, SerializationProtocolUtil.getProtocol(standardProtocol));
    Any<List<Integer>> any = received.getSemianyField().promote();

    assertArrayEquals(new Integer[] {5, 6, 7}, any.get().toArray());
  }
}
