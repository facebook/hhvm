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

package com.facebook.thrift.terse;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.facebook.thrift.any.LazyAny;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.terse.AdaptedTerseStruct;
import com.facebook.thrift.test.terse.MyStruct;
import com.facebook.thrift.test.terse.MyUnion;
import com.facebook.thrift.test.terse.NestedStruct;
import com.facebook.thrift.test.terse.SingleFieldStruct;
import com.facebook.thrift.test.terse.StructLevelTerseStruct;
import com.facebook.thrift.test.terse.Structv1;
import com.facebook.thrift.test.terse.Structv2;
import com.facebook.thrift.test.terse.Structv3;
import com.facebook.thrift.test.terse.TerseException;
import com.facebook.thrift.test.terse.TerseStruct;
import com.facebook.thrift.test.terse.TerseStructSingleField;
import com.facebook.thrift.test.terse.TerseStructWithDateAdapter;
import com.facebook.thrift.test.terse.TerseStructWithPrimitiveTypeAdapter;
import com.facebook.thrift.test.terse.TerseStructWithStructTypeAdapter;
import com.facebook.thrift.test.terse.TopLevelStruct;
import com.facebook.thrift.util.IntrinsicDefaults;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.util.Arrays;
import java.util.Date;
import java.util.stream.Stream;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

public class TerseWriteProtocolTest {

  static Stream<Arguments> data() {
    return Stream.of(
        Arguments.of(SerializationProtocol.TCompact),
        Arguments.of(SerializationProtocol.TBinary),
        Arguments.of(SerializationProtocol.TSimpleJSONBase64),
        Arguments.of(SerializationProtocol.TSimpleJSON),
        Arguments.of(SerializationProtocol.TJSON));
  }

  private SerializationProtocol serializationProtocol;

  private ByteBuf dest;
  private ByteBufTProtocol protocol;

  private ByteBufTProtocol createNewProtocol() {
    return SerializerUtil.toByteBufProtocol(serializationProtocol, dest);
  }

  private void serialize(ThriftSerializable st) {
    dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    protocol = SerializerUtil.toByteBufProtocol(serializationProtocol, dest);
    st.write0(protocol);
  }

  private int size(ThriftSerializable t) {
    dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    protocol = SerializerUtil.toByteBufProtocol(serializationProtocol, dest);
    t.write0(protocol);
    return dest.readableBytes();
  }

  private boolean protocolNotSupported() {
    return (serializationProtocol != SerializationProtocol.TCompact
        && serializationProtocol != SerializationProtocol.TBinary);
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testDeserializeTerseStruct(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    TopLevelStruct st = new TopLevelStruct.Builder().build();
    serialize(st);

    // deserialize it
    protocol = createNewProtocol();
    TopLevelStruct read = TopLevelStruct.read0(protocol);
    assertEquals(IntrinsicDefaults.defaultInt(), read.getIntField());
    assertNotNull(read.getInnerField());
    assertEquals(0, read.getInnerField().getBinaryField().readableBytes());
    assertEquals(IntrinsicDefaults.defaultInt(), read.getInnerField().getIntField());
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testEmptyStruct(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    Structv1 v1 = new Structv1.Builder().setStringField("test").build();
    assertEquals("test", v1.getStringField());

    SingleFieldStruct inner = new SingleFieldStruct.Builder().build();
    size(inner);
    Structv2 v2 = new Structv2.Builder().setStringField("test").setInnerField(inner).build();

    assertEquals("test", v2.getStringField());
    assertEquals(size(v1), size(v2));
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testNestedEmptyStruct(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    Structv1 v1 = new Structv1.Builder().setStringField("test").build();
    assertEquals("test", v1.getStringField());

    SingleFieldStruct inner =
        new SingleFieldStruct.Builder().setIntTerseField(0).setStringField("").build();
    NestedStruct ns = new NestedStruct.Builder().setInnerField(inner).setStringField("").build();
    Structv3 v3 = new Structv3.Builder().setStringField("test").setNestedField(ns).build();

    assertEquals("test", v3.getStringField());
    assertEquals(size(v1), size(v3));
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testNestedNonEmptyStruct(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    Structv1 v1 = new Structv1.Builder().setStringField("test").build();
    assertEquals("test", v1.getStringField());

    // int field is optional, should create nested structure
    SingleFieldStruct inner =
        new SingleFieldStruct.Builder().setIntField(0).setStringField("").build();
    NestedStruct ns = new NestedStruct.Builder().setInnerField(inner).setStringField("").build();
    Structv3 v3 = new Structv3.Builder().setStringField("test").setNestedField(ns).build();

    assertEquals("test", v3.getStringField());
    assertNotEquals(size(v1), size(v3));
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testEmptyStructWIthAdapter(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    if (protocolNotSupported()) {
      return;
    }
    // Inner struct not empty
    ByteBuf buf = Unpooled.wrappedBuffer(new byte[] {1, 2, 3, 4});
    TerseStructWithPrimitiveTypeAdapter inner =
        new TerseStructWithPrimitiveTypeAdapter.Builder()
            .setIntField(0)
            .setBinaryField(buf)
            .build();
    TopLevelStruct st = new TopLevelStruct.Builder().setInnerField(inner).build();
    assertEquals(IntrinsicDefaults.defaultInt(), st.getIntField());

    int size = size(st);

    // Inner struct empty
    buf = Unpooled.wrappedBuffer(new byte[0]);
    inner =
        new TerseStructWithPrimitiveTypeAdapter.Builder()
            .setIntField(0)
            .setBinaryField(buf)
            .build();
    st = new TopLevelStruct.Builder().setInnerField(inner).build();

    int emptySize = size(st);

    // empty size should be at least 4 bytes less.
    assertTrue(size - emptySize >= 4);
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testTerseStructWithStructTypeAdapter(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    if (protocolNotSupported()) {
      return;
    }
    TerseStructSingleField base = new TerseStructSingleField.Builder().setIntField(300).build();
    int size = size(base);

    TerseStructWithStructTypeAdapter st =
        new TerseStructWithStructTypeAdapter.Builder().setIntField(300).build();
    assertEquals(300, st.getIntField());
    assertNotNull(st.getAnyField());
    assertNotNull(st.getAnyField().getAny());
    // Any should not be serialized.
    assertEquals(size, size(st));

    MyStruct m = new MyStruct.Builder().setLongField(100L).build();
    LazyAny lazyAny = new LazyAny.Builder(m).build();
    st =
        new TerseStructWithStructTypeAdapter.Builder()
            .setIntField(300)
            .setAnyField(lazyAny)
            .build();
    // Any should be serialized.
    assertTrue(size(st) > size);
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testTerseStructWithDateAdapter(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    TerseStructWithDateAdapter st =
        new TerseStructWithDateAdapter.Builder().setStringField("foo").build();
    assertEquals("foo", st.getStringField());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getDateField().getTime());
    int size = size(st);

    st =
        new TerseStructWithDateAdapter.Builder()
            .setStringField("foo")
            .setDateField(new Date(0))
            .build();
    assertEquals("foo", st.getStringField());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getDateField().getTime());
    assertEquals(size, size(st));

    st =
        new TerseStructWithDateAdapter.Builder()
            .setStringField("foo")
            .setDateField(new Date(6700000000L))
            .build();
    assertEquals("foo", st.getStringField());
    assertEquals(6700000000L, st.getDateField().getTime());
    assertTrue(size(st) > size);
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testUnion(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    // even the value is intrinsic default, this should be written
    MyUnion u = MyUnion.fromIntField3(0);
    StructLevelTerseStruct st1 =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();
    assertEquals("test", st1.getStringField());

    u = MyUnion.fromIntField3(5);
    StructLevelTerseStruct st2 =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();
    assertEquals("test", st2.getStringField());

    assertEquals(size(st1), size(st2));

    // nothing is set
    u = new MyUnion();
    StructLevelTerseStruct st3 =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();

    assertTrue(size(st1) > size(st3));
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testDeserializeUnion(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    StructLevelTerseStruct st = new StructLevelTerseStruct.Builder().build();
    serialize(st);

    // deserialize it
    protocol = createNewProtocol();
    StructLevelTerseStruct read = StructLevelTerseStruct.read0(protocol);
    assertNotNull(read.getUnionField());
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testException(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    MyUnion u = MyUnion.fromIntField3(0);
    StructLevelTerseStruct st1 =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();
    assertEquals("test", st1.getStringField());

    TerseException e = new TerseException.Builder().setCode(0).setMsg("").build();
    StructLevelTerseStruct st2 =
        new StructLevelTerseStruct.Builder()
            .setStringField("test")
            .setUnionField(u)
            .setExceptionField(e)
            .build();

    assertEquals(size(st1), size(st2));
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testExceptionDefaultValue(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    MyUnion u = MyUnion.fromIntField3(0);
    StructLevelTerseStruct st =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();
    serialize(st);

    // deserialize it
    protocol = createNewProtocol();
    StructLevelTerseStruct read = StructLevelTerseStruct.read0(protocol);
    assertNotNull(read.getExceptionField());
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testAdaptedTerseStruct(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    if (protocolNotSupported()) {
      return;
    }
    AdaptedTerseStruct st = new AdaptedTerseStruct.Builder().build();
    serialize(st);

    protocol = createNewProtocol();
    TerseStruct received = TerseStruct.read0(protocol);
    assertEquals(IntrinsicDefaults.defaultBoolean(), received.isBooleanField());
    assertEquals(IntrinsicDefaults.defaultShort(), received.getShortField());
    assertEquals(IntrinsicDefaults.defaultInt(), received.getIntField());
    assertEquals(IntrinsicDefaults.defaultLong(), received.getLongField());
    assertEquals(IntrinsicDefaults.defaultByteArray(), received.getB1());
    assertEquals(IntrinsicDefaults.defaultList(), received.getBinaryListField());
    assertEquals(null, received.isOptionalBooleanField());
    assertEquals(null, received.getOptionalB1());
    assertEquals(IntrinsicDefaults.defaultInt(), received.getIntField2());
    assertEquals(IntrinsicDefaults.defaultInt(), received.getDoubleTypedefIntField());
  }

  @ParameterizedTest
  @MethodSource("data")
  public void testBuildFromOther(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
    TerseStructWithDateAdapter st =
        new TerseStructWithDateAdapter.Builder()
            .setStringField("foo")
            .setDateField(new Date(54321))
            .build();
    byte[] bytes = SerializerUtil.toByteArray(st, serializationProtocol);

    TerseStructWithDateAdapter other = new TerseStructWithDateAdapter.Builder(st).build();
    assertTrue(Arrays.equals(bytes, SerializerUtil.toByteArray(other, serializationProtocol)));
  }
}
