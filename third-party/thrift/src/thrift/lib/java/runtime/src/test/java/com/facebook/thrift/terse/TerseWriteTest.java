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

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.test.terse.EmptyStruct;
import com.facebook.thrift.test.terse.FieldLevelTerseStruct;
import com.facebook.thrift.test.terse.InnerTerseStruct;
import com.facebook.thrift.test.terse.MyEnum;
import com.facebook.thrift.test.terse.MyStruct;
import com.facebook.thrift.test.terse.PackageLevelTerseStruct;
import com.facebook.thrift.test.terse.StructLevelTerseStruct;
import com.facebook.thrift.test.terse.TerseException;
import com.facebook.thrift.test.terse.TerseStructWithCustomDefault;
import com.facebook.thrift.test.terse.TerseStructWithPrimitiveTypeAdapter;
import com.facebook.thrift.test.terse.TerseStructWithStructTypeAdapter;
import com.facebook.thrift.util.IntrinsicDefaults;
import com.facebook.thrift.util.SerializationProtocol;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

public class TerseWriteTest {

  @Rule public ExpectedException expectedException = ExpectedException.none();

  private ByteBufTProtocol serialize(ThriftSerializable st) {
    ByteBuf dest = RpcResources.getUnpooledByteBufAllocator().buffer();
    ByteBufTProtocol protocol =
        SerializerUtil.toByteBufProtocol(SerializationProtocol.TBinary, dest);
    st.write0(protocol);
    return protocol;
  }

  @Test
  public void testStructLevelTerseStruct() {
    StructLevelTerseStruct st = new StructLevelTerseStruct.Builder().build();
    assertEquals(IntrinsicDefaults.defaultBoolean(), st.isBoolField());
    assertEquals(IntrinsicDefaults.defaultByte(), st.getByteField());
    assertEquals(IntrinsicDefaults.defaultShort(), st.getShortField());
    assertEquals(IntrinsicDefaults.defaultInt(), st.getIntField());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getLongField());
    assertEquals(IntrinsicDefaults.defaultFloat(), st.getFloatField(), 0);
    assertEquals(IntrinsicDefaults.defaultDouble(), st.getDoubleField(), 0);
    assertEquals(IntrinsicDefaults.defaultString(), st.getStringField());
    assertArrayEquals(IntrinsicDefaults.defaultByteArray(), st.getBinaryField());
    assertEquals(IntrinsicDefaults.defaultEnum(), st.getEnumField().getValue());
    assertEquals(IntrinsicDefaults.defaultList().size(), st.getListField().size());
    assertEquals(IntrinsicDefaults.defaultSet().size(), st.getSetField().size());
    assertEquals(IntrinsicDefaults.defaultMap().size(), st.getMapField().size());
    assertNotNull(st.getStructField());
    assertNotNull(st.getInnerField());
  }

  @Test
  public void testTerseStructWithCustomDefault() {
    TerseStructWithCustomDefault st = new TerseStructWithCustomDefault.Builder().build();
    assertEquals(true, st.isBoolField());
    assertEquals(1, st.getByteField());
    assertEquals(2, st.getShortField());
    assertEquals(3, st.getIntField());
    assertEquals(4, st.getLongField());
    assertEquals(5.0f, st.getFloatField(), 0);
    assertEquals(6.0d, st.getDoubleField(), 0);
    assertEquals("7", st.getStringField());
    assertArrayEquals(new byte[] {56}, st.getBinaryField());
    assertEquals(MyEnum.ME1, st.getEnumField());
    assertEquals(1, (short) st.getListField().get(0));
    assertEquals(1, st.getSetField().iterator().next().intValue());
    assertEquals(1, st.getMapField().size());
    assertEquals(1, (short) st.getMapField().get((short) 1));
    assertNotNull(st.getStructField());
  }

  @Test
  public void testTerseStructWithCustomDefaultDeserialized() {
    ByteBufTProtocol protocol = serialize(new EmptyStruct.Builder().build());
    // create from an empty struct
    TerseStructWithCustomDefault st = TerseStructWithCustomDefault.read0(protocol);

    assertEquals(IntrinsicDefaults.defaultBoolean(), st.isBoolField());
    assertEquals(IntrinsicDefaults.defaultByte(), st.getByteField());
    assertEquals(IntrinsicDefaults.defaultShort(), st.getShortField());
    assertEquals(IntrinsicDefaults.defaultInt(), st.getIntField());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getLongField());
    assertEquals(IntrinsicDefaults.defaultFloat(), st.getFloatField(), 0);
    assertEquals(IntrinsicDefaults.defaultDouble(), st.getDoubleField(), 0);
    assertEquals(IntrinsicDefaults.defaultString(), st.getStringField());
    assertArrayEquals(IntrinsicDefaults.defaultByteArray(), st.getBinaryField());
    assertEquals(MyEnum.ME0, st.getEnumField());
    assertEquals(IntrinsicDefaults.defaultList(), st.getListField());
    assertEquals(IntrinsicDefaults.defaultSet(), st.getSetField());
    assertEquals(IntrinsicDefaults.defaultMap(), st.getMapField());
    assertNotNull(st.getStructField());
  }

  @Test
  public void testFieldLevelTerseStruct() {
    FieldLevelTerseStruct st = new FieldLevelTerseStruct.Builder().build();
    assertEquals(IntrinsicDefaults.defaultBoolean(), st.isTerseBoolField());
    assertEquals(IntrinsicDefaults.defaultByte(), st.getTerseByteField());
    assertEquals(IntrinsicDefaults.defaultShort(), st.getTerseShortField());
    assertEquals(IntrinsicDefaults.defaultInt(), st.getTerseIntField());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getTerseLongField());
    assertEquals(IntrinsicDefaults.defaultFloat(), st.getTerseFloatField(), 0);
    assertEquals(IntrinsicDefaults.defaultDouble(), st.getTerseDoubleField(), 0);
    assertEquals(IntrinsicDefaults.defaultString(), st.getTerseStringField());
    assertArrayEquals(IntrinsicDefaults.defaultByteArray(), st.getTerseBinaryField());
    assertEquals(IntrinsicDefaults.defaultEnum(), st.getTerseEnumField().getValue());
    assertEquals(IntrinsicDefaults.defaultList().size(), st.getTerseListField().size());
    assertEquals(IntrinsicDefaults.defaultSet().size(), st.getTerseSetField().size());
    assertEquals(IntrinsicDefaults.defaultMap().size(), st.getTerseMapField().size());

    // optional
    assertNull(st.isBoolField());

    assertEquals(0, st.getByteField());
    assertEquals(0, st.getShortField());
    assertEquals(0, st.getIntField());
    assertNull(st.getLongField());
    assertEquals(0f, st.getFloatField(), 0);
    assertEquals(0d, st.getDoubleField(), 0);
    assertNull(st.getStringField());
    assertNull(st.getBinaryField());
    assertEquals(0, st.getEnumField().getValue());
    assertNull(st.getListField());
    assertNull(st.getSetField());
    assertNull(st.getMapField());
    assertNull(st.getStructField());
  }

  @Test
  public void testTerseStructWithPrimitiveTypeAdapter() {
    TerseStructWithPrimitiveTypeAdapter st =
        new TerseStructWithPrimitiveTypeAdapter.Builder().build();
    assertEquals(IntrinsicDefaults.defaultInt(), st.getIntField());
    assertEquals(0, st.getBinaryField().readableBytes());
  }

  @Test
  public void testTerseStructWithPrimitiveTypeAdapterNullValue() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("must not be null");
    TerseStructWithPrimitiveTypeAdapter st =
        new TerseStructWithPrimitiveTypeAdapter.Builder().setBinaryField(null).build();
    serialize(st);
  }

  @Test
  public void testTerseStructWithStructTypeAdapterNullValue() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("must not be null");
    TerseStructWithStructTypeAdapter st =
        new TerseStructWithStructTypeAdapter.Builder().setAnyField(null).build();
    serialize(st);
  }

  @Test
  public void testNonTerseStruct() {
    MyStruct st = new MyStruct.Builder().build();
    assertEquals(0, st.getIntField());
    assertNull(st.getStructField());
  }

  @Test
  public void testPackageLevelTerseStruct() {
    PackageLevelTerseStruct st = new PackageLevelTerseStruct.Builder().build();
    assertEquals(IntrinsicDefaults.defaultBoolean(), st.isBoolVal());
    assertEquals(IntrinsicDefaults.defaultByte(), st.getByteVal());
    assertEquals(IntrinsicDefaults.defaultShort(), st.getI16Val());
    assertEquals(IntrinsicDefaults.defaultInt(), st.getI32Val());
    assertEquals(IntrinsicDefaults.defaultLong(), st.getI64Val());
    assertEquals(IntrinsicDefaults.defaultFloat(), st.getFloatVal(), 0);
    assertEquals(IntrinsicDefaults.defaultDouble(), st.getDoubleVal(), 0);
    assertEquals(IntrinsicDefaults.defaultString(), st.getStringVal());
    assertArrayEquals(IntrinsicDefaults.defaultByteArray(), st.getBinaryVal());
    assertEquals(IntrinsicDefaults.defaultList().size(), st.getListVal().size());
    assertEquals(IntrinsicDefaults.defaultSet().size(), st.getSetVal().size());
    assertEquals(IntrinsicDefaults.defaultMap().size(), st.getMapVal().size());
    assertNotNull(st.getStructVal());
    assertNull(st.getOptStructVal());
    assertNull(st.getOptI32Val());
    assertNotNull(st.getUnionVal());
  }

  @Test
  public void testTerseException() {
    TerseException ex = new TerseException.Builder().build();
    assertEquals(IntrinsicDefaults.defaultInt(), ex.getCode());
    assertEquals(IntrinsicDefaults.defaultString(), ex.getMsg());
  }

  @Test
  public void testStructLevelTerseStructNullUnionValue() {
    expectedException.expect(NullPointerException.class);
    expectedException.expectMessage("unionField must not be null");
    StructLevelTerseStruct st =
        new StructLevelTerseStruct.Builder()
            .setStringField("test")
            .setBinaryField(new byte[1])
            .setEnumField(MyEnum.ME1)
            .setListField(new ArrayList<>())
            .setSetField(new HashSet<>())
            .setMapField(new HashMap<>())
            .setStructField(new MyStruct.Builder().build())
            .setInnerField(new InnerTerseStruct.Builder().build())
            .setUnionField(null)
            .build();
    serialize(st);
  }
}
