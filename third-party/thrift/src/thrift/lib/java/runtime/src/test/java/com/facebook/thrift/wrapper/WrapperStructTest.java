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

package com.facebook.thrift.wrapper;

import static junit.framework.TestCase.assertTrue;
import static org.junit.Assert.*;

import com.facebook.thrift.any.Any;
import com.facebook.thrift.test.wrapper.MutableTerseWrappedTestStruct;
import com.facebook.thrift.test.wrapper.TerseWrappedTestStruct;
import com.facebook.thrift.test.wrapper.TestStruct;
import com.facebook.thrift.test.wrapper.WrappedTestException;
import com.facebook.thrift.test.wrapper.WrappedTestStruct;
import com.facebook.thrift.util.IntrinsicDefaults;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.wrapper.test.PoliciedField;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Collection;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class WrapperStructTest {

  private final boolean exception;

  public WrapperStructTest(boolean exception) {
    this.exception = exception;
  }

  @Parameterized.Parameters
  public static Collection<Boolean> data() {
    return Arrays.asList(false, true);
  }

  private byte[] serializeWrapped(WrappedTestStruct struct) {
    return SerializerUtil.toByteArray(struct, SerializationProtocol.TCompact);
  }

  private byte[] serialize(TestStruct struct) {
    return SerializerUtil.toByteArray(struct, SerializationProtocol.TCompact);
  }

  private WrappedTestStruct copy(WrappedTestException from) {
    WrappedTestStruct.Builder to = new WrappedTestStruct.Builder();

    for (Field f : from.getClass().getDeclaredFields()) {
      try {
        Field t = to.getClass().getDeclaredField(f.getName());
        f.setAccessible(true);
        t.setAccessible(true);
        t.set(to, f.get(from));
      } catch (Throwable t) {
        // ignore
      }
    }

    return to.build();
  }

  private WrappedTestStruct deserializeWrapped(byte[] bytes) {
    // test both Struct and Exception
    if (exception) {
      return copy(
          SerializerUtil.fromByteArray(
              WrappedTestException.asReader(), bytes, SerializationProtocol.TCompact));
    }

    return SerializerUtil.fromByteArray(
        WrappedTestStruct.asReader(), bytes, SerializationProtocol.TCompact);
  }

  private TestStruct deserialize(byte[] bytes) {
    return SerializerUtil.fromByteArray(
        TestStruct.asReader(), bytes, SerializationProtocol.TCompact);
  }

  @Test
  public void testBoolFieldWrapper() {
    TestStruct st = new TestStruct.Builder().setContext(1).setWrappedBooleanField(true).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(true, st.isWrappedBooleanField());
    assertEquals(true, wrapped.getWrappedBooleanField().getValue());
    assertEquals(true, wrapped.getWrappedBooleanField().getZonedValue());

    st = new TestStruct.Builder().setContext(0).setWrappedBooleanField(true).build();
    wrapped = deserializeWrapped(serialize(st));

    assertEquals(true, st.isWrappedBooleanField());
    assertEquals(true, wrapped.getWrappedBooleanField().getValue());
    assertNull(wrapped.getWrappedBooleanField().getZonedValue());
  }

  @Test
  public void testIntFieldWrapper() {
    TestStruct st = new TestStruct.Builder().setContext(1).setWrappedIntField(3).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(3, st.getWrappedIntField());
    assertEquals(3, (int) wrapped.getWrappedIntField().getValue());
    assertEquals(3, (int) wrapped.getWrappedIntField().getZonedValue());

    st = new TestStruct.Builder().setContext(0).setWrappedIntField(3).build();
    wrapped = deserializeWrapped(serialize(st));

    assertEquals(3, st.getWrappedIntField());
    assertEquals(3, (int) wrapped.getWrappedIntField().getValue());
    assertNull(wrapped.getWrappedIntField().getZonedValue());
  }

  @Test
  public void testSetIntFieldWrapper() {
    WrappedTestStruct wrapped =
        new WrappedTestStruct.Builder()
            .setContext(1)
            .setWrappedIntField(new PoliciedField<Integer>(20))
            .build();
    TestStruct st = deserialize(serializeWrapped(wrapped));

    assertEquals(20, st.getWrappedIntField());
    assertEquals(20, (int) wrapped.getWrappedIntField().getValue());
  }

  @Test
  public void testByteBufFieldWrapper() {
    byte[] bytes = new byte[] {1, 2, 3};
    TestStruct st = new TestStruct.Builder().setContext(0).setWrappedByteBufField(bytes).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertTrue(Arrays.equals(bytes, st.getWrappedByteBufField()));
    assertTrue(
        Arrays.equals(
            bytes, ByteBufUtil.getBytes((ByteBuf) wrapped.getWrappedByteBufField().getValue())));
    assertNull(wrapped.getWrappedByteBufField().getZonedValue());
  }

  @Test
  public void testByteBufInnerFieldWrapper() {
    byte[] bytes = new byte[] {1, 2, 3};
    TestStruct st = new TestStruct.Builder().setContext(1).setWrappedByteBufField2(bytes).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertTrue(Arrays.equals(bytes, st.getWrappedByteBufField2()));
    assertTrue(
        Arrays.equals(
            bytes, ByteBufUtil.getBytes((ByteBuf) wrapped.getWrappedByteBufField2().getValue())));
    assertTrue(
        Arrays.equals(
            bytes,
            ByteBufUtil.getBytes((ByteBuf) wrapped.getWrappedByteBufField2().getZonedValue())));
  }

  @Test
  public void testSingleAdaptedIntFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder().setContext(1).setWrappedSingleAdaptedIntField(200).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(200, st.getWrappedSingleAdaptedIntField());
    assertEquals("200", (String) wrapped.getWrappedSingleAdaptedIntField().getValue());
    assertEquals("200", (String) wrapped.getWrappedSingleAdaptedIntField().getZonedValue());
  }

  @Test
  public void testDoubleAdaptedIntFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder().setContext(1).setWrappedDoubleAdaptedIntField(100).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(100, st.getWrappedDoubleAdaptedIntField());
    assertEquals(100L, (long) wrapped.getWrappedDoubleAdaptedIntField().getValue());
    assertEquals(100L, (long) wrapped.getWrappedDoubleAdaptedIntField().getZonedValue());
  }

  @Test
  public void testTerseFields() {
    TerseWrappedTestStruct st = new TerseWrappedTestStruct.Builder().build();
    assertEquals(IntrinsicDefaults.defaultInt(), st.getContext());
    assertEquals(IntrinsicDefaults.defaultBoolean(), st.getWrappedBooleanField().getValue());
    assertEquals(IntrinsicDefaults.defaultInt(), (int) st.getWrappedIntField().getValue());
    assertEquals(IntrinsicDefaults.defaultByteBuf(), st.getWrappedByteBufField().getValue());
    assertEquals(IntrinsicDefaults.defaultByteBuf(), st.getWrappedByteBufField2().getValue());
    assertEquals(
        IntrinsicDefaults.defaultLong(), (long) st.getWrappedDoubleAdaptedIntField().getValue());
    assertEquals("0", st.getWrappedSingleAdaptedIntField().getValue());
  }

  @Test
  public void testEquals() {
    WrappedTestStruct st1 =
        new WrappedTestStruct.Builder()
            .setContext(4)
            .setWrappedBooleanField(new PoliciedField<>(true))
            .setWrappedIntField(new PoliciedField<>(7))
            .setWrappedSingleAdaptedIntField(new PoliciedField<>("9"))
            .setWrappedDoubleAdaptedIntField(new PoliciedField<>(100L))
            .setWrappedByteBufField(new PoliciedField<>(Unpooled.wrappedBuffer("foo".getBytes())))
            .setWrappedByteBufField2(new PoliciedField<>(Unpooled.wrappedBuffer("bar".getBytes())))
            .build();

    WrappedTestStruct st2 =
        new WrappedTestStruct.Builder()
            .setContext(4)
            .setWrappedBooleanField(new PoliciedField<>(true))
            .setWrappedIntField(new PoliciedField<>(7))
            .setWrappedSingleAdaptedIntField(new PoliciedField<>("9"))
            .setWrappedDoubleAdaptedIntField(new PoliciedField<>(100L))
            .setWrappedByteBufField(new PoliciedField<>(Unpooled.wrappedBuffer("foo".getBytes())))
            .setWrappedByteBufField2(new PoliciedField<>(Unpooled.wrappedBuffer("bar".getBytes())))
            .build();

    assertEquals(st1, st2);
  }

  @Test
  public void testSetTerseFields() {
    TerseWrappedTestStruct st =
        new TerseWrappedTestStruct.Builder()
            .setWrappedBooleanField(new PoliciedField<>(true))
            .setWrappedIntField(new PoliciedField<>(7))
            .setWrappedSingleAdaptedIntField(new PoliciedField<>("9"))
            .setWrappedDoubleAdaptedIntField(new PoliciedField<>(100L))
            .setWrappedByteBufField(new PoliciedField<>(Unpooled.wrappedBuffer("foo".getBytes())))
            .setWrappedByteBufField2(new PoliciedField<>(Unpooled.wrappedBuffer("bar".getBytes())))
            .build();
    assertEquals(IntrinsicDefaults.defaultInt(), st.getContext());
    assertEquals(true, st.getWrappedBooleanField().getValue());
    assertEquals(7, (int) st.getWrappedIntField().getValue());
    assertEquals(
        ByteBufUtil.hexDump("foo".getBytes()),
        ByteBufUtil.hexDump(st.getWrappedByteBufField().getValue()));
    assertEquals(
        ByteBufUtil.hexDump("bar".getBytes()),
        ByteBufUtil.hexDump(st.getWrappedByteBufField2().getValue()));
    assertEquals(100L, (long) st.getWrappedDoubleAdaptedIntField().getValue());
    assertEquals("9", st.getWrappedSingleAdaptedIntField().getValue());
  }

  @Test
  public void testMutableIntFieldWrapper() {
    TestStruct st = new TestStruct.Builder().setContext(1).setWrappedIntField(3).build();
    MutableTerseWrappedTestStruct wrapped =
        SerializerUtil.fromByteArray(
            MutableTerseWrappedTestStruct.asReader(),
            serialize(st),
            SerializationProtocol.TCompact);

    assertEquals(3, st.getWrappedIntField());
    assertEquals(3, (int) wrapped.getWrappedIntField().getValue());
    assertEquals(3, (int) wrapped.getWrappedIntField().getZonedValue());

    st = new TestStruct.Builder().setContext(0).setWrappedIntField(3).build();
    wrapped =
        SerializerUtil.fromByteArray(
            MutableTerseWrappedTestStruct.asReader(),
            serialize(st),
            SerializationProtocol.TCompact);

    assertEquals(3, st.getWrappedIntField());
    assertEquals(3, (int) wrapped.getWrappedIntField().getValue());
    assertNull(wrapped.getWrappedIntField().getZonedValue());
  }

  @Test
  public void testMutableSingleAdaptedIntFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder().setContext(1).setWrappedSingleAdaptedIntField(200).build();
    MutableTerseWrappedTestStruct wrapped =
        SerializerUtil.fromByteArray(
            MutableTerseWrappedTestStruct.asReader(),
            serialize(st),
            SerializationProtocol.TCompact);

    assertEquals(200, st.getWrappedSingleAdaptedIntField());
    assertEquals("200", (String) wrapped.getWrappedSingleAdaptedIntField().getValue());
    assertEquals("200", (String) wrapped.getWrappedSingleAdaptedIntField().getZonedValue());
  }

  @Test
  public void testMutableDoubleAdaptedIntFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder().setContext(1).setWrappedDoubleAdaptedIntField(100).build();
    MutableTerseWrappedTestStruct wrapped =
        SerializerUtil.fromByteArray(
            MutableTerseWrappedTestStruct.asReader(),
            serialize(st),
            SerializationProtocol.TCompact);

    assertEquals(100, st.getWrappedDoubleAdaptedIntField());
    assertEquals(100L, (long) wrapped.getWrappedDoubleAdaptedIntField().getValue());
    assertEquals(100L, (long) wrapped.getWrappedDoubleAdaptedIntField().getZonedValue());
  }

  @Test
  public void testMutableTerseFields() {
    MutableTerseWrappedTestStruct st = new MutableTerseWrappedTestStruct.Builder().build();
    assertEquals(IntrinsicDefaults.defaultInt(), st.getContext());
    assertEquals(IntrinsicDefaults.defaultBoolean(), st.getWrappedBooleanField().getValue());
    assertEquals(IntrinsicDefaults.defaultInt(), (int) st.getWrappedIntField().getValue());
    assertEquals(IntrinsicDefaults.defaultByteBuf(), st.getWrappedByteBufField().getValue());
    assertEquals(IntrinsicDefaults.defaultByteBuf(), st.getWrappedByteBufField2().getValue());
    assertEquals(
        IntrinsicDefaults.defaultLong(), (long) st.getWrappedDoubleAdaptedIntField().getValue());
    assertEquals("0", st.getWrappedSingleAdaptedIntField().getValue());
  }

  @Test
  public void testIntListFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder().setContext(1).setListAdaptedIntField(Arrays.asList(2)).build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(2, (long) st.getListAdaptedIntField().get(0));
    assertEquals("2", wrapped.getListAdaptedIntField().getValue().get(0));
  }

  @Test
  public void testAnyListFieldWrapper() {
    TestStruct st =
        new TestStruct.Builder()
            .setContext(1)
            .setAnyListField(Arrays.asList(new Any.Builder("foo").build()))
            .build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals("foo", st.getAnyListField().get(0).get());
    assertEquals("foo", wrapped.getAnyListField().getValue().get(0).get());
  }

  @Test
  public void testMultipleAdapterAndWrapper() {
    TestStruct st =
        new TestStruct.Builder()
            .setContext(1)
            .setWrappedDoubleAdaptedIntListField(Arrays.asList(3))
            .build();
    WrappedTestStruct wrapped = deserializeWrapped(serialize(st));

    assertEquals(3, (long) st.getWrappedDoubleAdaptedIntListField().get(0));
    assertEquals("3", wrapped.getWrappedDoubleAdaptedIntListField().getValue().getValue().get(0));
  }
}
