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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import com.facebook.thrift.any.LazyAny;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.terse.MyStruct;
import com.facebook.thrift.test.terse.MyUnion;
import com.facebook.thrift.test.terse.NestedStruct;
import com.facebook.thrift.test.terse.SingleFieldStruct;
import com.facebook.thrift.test.terse.StructLevelTerseStruct;
import com.facebook.thrift.test.terse.Structv1;
import com.facebook.thrift.test.terse.Structv2;
import com.facebook.thrift.test.terse.Structv3;
import com.facebook.thrift.test.terse.TerseException;
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
import java.util.Collection;
import java.util.Date;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class TerseWriteProtocolTest {

  @Parameterized.Parameters
  public static Collection<Object> data() {
    return Arrays.asList(
        SerializationProtocol.TCompact,
        SerializationProtocol.TBinary,
        SerializationProtocol.TSimpleJSONBase64,
        SerializationProtocol.TSimpleJSON
        /** SerializationProtocol.TJSON * */
        );
  }

  private final SerializationProtocol serializationProtocol;

  public TerseWriteProtocolTest(SerializationProtocol serializationProtocol) {
    this.serializationProtocol = serializationProtocol;
  }

  private ByteBuf dest;
  private ByteBufTProtocol protocol;

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

  @Test
  public void testDeserializeTerseStruct() {
    TopLevelStruct st = new TopLevelStruct.Builder().build();
    serialize(st);

    // deserialize it
    TopLevelStruct read = TopLevelStruct.read0(protocol);
    assertEquals(IntrinsicDefaults.defaultInt(), read.getIntField());
    assertNotNull(read.getInnerField());
    assertEquals(0, read.getInnerField().getBinaryField().readableBytes());
    assertEquals(IntrinsicDefaults.defaultInt(), read.getInnerField().getIntField());
  }

  @Test
  public void testEmptyStruct() {
    Structv1 v1 = new Structv1.Builder().setStringField("test").build();
    assertEquals("test", v1.getStringField());

    SingleFieldStruct inner = new SingleFieldStruct.Builder().build();
    Structv2 v2 = new Structv2.Builder().setStringField("test").setInnerField(inner).build();

    assertEquals("test", v2.getStringField());
    assertEquals(size(v1), size(v2));
  }

  @Test
  public void testNestedEmptyStruct() {
    Structv1 v1 = new Structv1.Builder().setStringField("test").build();
    assertEquals("test", v1.getStringField());

    SingleFieldStruct inner =
        new SingleFieldStruct.Builder().setIntTerseField(0).setStringField("").build();
    NestedStruct ns = new NestedStruct.Builder().setInnerField(inner).setStringField("").build();
    Structv3 v3 = new Structv3.Builder().setStringField("test").setNestedField(ns).build();

    assertEquals("test", v3.getStringField());
    assertEquals(size(v1), size(v3));
  }

  @Test
  public void testNestedNonEmptyStruct() {
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

  @Test
  public void testEmptyStructWIthAdapter() {
    if (serializationProtocol != SerializationProtocol.TCompact
        && serializationProtocol != SerializationProtocol.TBinary) {
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

  @Test
  public void testTerseStructWithStructTypeAdapter() {
    if (serializationProtocol != SerializationProtocol.TCompact
        && serializationProtocol != SerializationProtocol.TBinary) {
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

  @Test
  public void testTerseStructWithDateAdapter() {
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

  @Test
  public void testUnion() {
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

  @Test
  public void testDeserializeUnion() {
    StructLevelTerseStruct st = new StructLevelTerseStruct.Builder().build();
    serialize(st);

    // deserialize it
    StructLevelTerseStruct read = StructLevelTerseStruct.read0(protocol);
    assertNotNull(read.getUnionField());
  }

  @Test
  public void testException() {
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

  @Test
  public void testExceptionDefaultValue() {
    MyUnion u = MyUnion.fromIntField3(0);
    StructLevelTerseStruct st =
        new StructLevelTerseStruct.Builder().setStringField("test").setUnionField(u).build();
    serialize(st);

    // deserialize it
    StructLevelTerseStruct read = StructLevelTerseStruct.read0(protocol);
    assertNotNull(read.getExceptionField());
  }
}
