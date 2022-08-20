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

package com.facebook.thrift;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.core.Is.is;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.java.test.BigEnum;
import com.facebook.thrift.java.test.MySimpleStruct;
import com.facebook.thrift.java.test.MySimpleUnion;
import com.facebook.thrift.java.test.SmallEnum;
import com.facebook.thrift.java.test.StructMutable;
import com.facebook.thrift.java.test.StructWithAllTypes;
import com.facebook.thrift.java.test.StructWithOptional;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import org.junit.Test;

public class StructTest {
  @Test
  public void testStructHashcode() throws Exception {
    MySimpleStruct defaultStruct = new MySimpleStruct();
    assertThat(defaultStruct.hashCode(), is(not(equalTo(0))));

    MySimpleStruct struct1 = new MySimpleStruct(1, "Foo");
    MySimpleStruct struct2 = new MySimpleStruct(2, "Bar");

    assertThat(struct1.hashCode(), is(not(equalTo(0))));
    assertThat(struct2.hashCode(), is(not(equalTo(0))));
    assertThat(struct1.hashCode(), is(not(equalTo(struct2.hashCode()))));
  }

  @Test
  public void testSmallEnum() throws Exception {
    assertThat(SmallEnum.findByValue(SmallEnum.RED.getValue()), equalTo(SmallEnum.RED));
    assertThat(SmallEnum.findByValue(Integer.MAX_VALUE), equalTo(null));
  }

  @Test
  public void testBigEnum() throws Exception {
    assertThat(BigEnum.findByValue(BigEnum.ONE.getValue()), equalTo(BigEnum.ONE));
    assertThat(BigEnum.findByValue(Integer.MAX_VALUE), equalTo(null));
  }

  @Test
  public void testStructEquality() throws Exception {
    StructWithOptional orig = new StructWithOptional();
    orig.setId(1L);
    orig.setName("Toto");
    orig.setId2(2);
    orig.setName2("Titi");

    StructWithOptional copy = orig.deepCopy();
    assertThat(orig, equalTo(copy));

    copy = orig.deepCopy();
    copy.unsetName2();
    assertThat(orig, not(equalTo(copy)));
    assertThat(copy, not(equalTo(orig)));

    copy = orig.deepCopy();
    copy.unsetId2();
    assertThat(orig, not(equalTo(copy)));
    assertThat(copy, not(equalTo(orig)));

    // set a regular field to null
    orig.setName(null);
    copy = orig.deepCopy();
    assertThat(orig, equalTo(copy));

    copy = orig.deepCopy();
    copy.unsetName2();
    assertThat(orig, not(equalTo(copy)));
    assertThat(copy, not(equalTo(orig)));

    copy = orig.deepCopy();
    copy.unsetId2();
    assertThat(orig, not(equalTo(copy)));
    assertThat(copy, not(equalTo(orig)));

    // unset an optional non-nullable
    orig.unsetId2();
    copy = orig.deepCopy();
    assertThat(orig, equalTo(copy));

    copy = orig.deepCopy();
    copy.setId2(0);
    assertThat(orig, not(equalTo(copy)));
    assertThat(copy, not(equalTo(orig)));
  }

  @Test
  public void testStructAllTypeEquality() throws Exception {
    StructWithAllTypes orig =
        new StructWithAllTypes(
            true,
            (byte) 1,
            (short) 2,
            3,
            4L,
            5.f,
            6.0,
            "toto",
            "binarydata".getBytes(),
            new ArrayList<Integer>() {
              {
                add(1);
                add(2);
                add(3);
              }
            },
            new HashSet<Integer>() {
              {
                add(3);
                add(4);
                add(5);
              }
            },
            new HashMap<Integer, Integer>() {
              {
                put(1, 2);
                put(3, 4);
                put(5, 6);
              }
            },
            SmallEnum.RED,
            new MySimpleStruct(),
            MySimpleUnion.caseOne(123));

    StructWithAllTypes copy = orig.deepCopy();
    assertThat(orig, equalTo(copy));

    copy = orig.deepCopy();
    copy.setBin("XXXXX".getBytes());
    assertThat(orig, not(equalTo(copy)));

    copy = orig.deepCopy();
    StructWithAllTypes copy2 = orig.deepCopy();
    copy.setL(123L);
    copy2.setL(321L);
    assertThat(copy, not(equalTo(copy2)));

    copy.unsetL();
    copy2.unsetL();
    assertThat(copy, equalTo(copy2));

    copy = orig.deepCopy();
    copy2 = orig.deepCopy();
    copy.setMyEnum(SmallEnum.BLUE);
    copy2.setMyEnum(SmallEnum.GREEN);
    assertThat(copy, not(equalTo(copy2)));

    copy.setMyEnum(null);
    assertThat(copy, not(equalTo(copy2)));

    copy2.setMyEnum(null);
    assertThat(copy, equalTo(copy2));

    copy.setMyEnum(SmallEnum.BLUE);
    copy2.setMyEnum(SmallEnum.GREEN);
    copy.unsetMyEnum();
    copy2.unsetMyEnum();
    assertThat(copy, equalTo(copy2));

    // test optional field
    copy = orig.deepCopy();
    copy2 = orig.deepCopy();
    assertThat(copy, equalTo(copy2));
    copy2.unsetBb();
    assertThat(copy, not(equalTo(copy2)));
  }

  @Test
  public void testAndroidStructWithOptionalEquality() throws Exception {
    com.facebook.thrift.android.test.StructWithOptional orig =
        new com.facebook.thrift.android.test.StructWithOptional(1L, "toto", 2, "titi");

    com.facebook.thrift.android.test.StructWithOptional copy =
        new com.facebook.thrift.android.test.StructWithOptional(1L, "toto", 2, null);
    assertThat(orig, not(equalTo(copy)));

    com.facebook.thrift.android.test.StructWithOptional copy2 =
        new com.facebook.thrift.android.test.StructWithOptional(1L, "toto", null, "titi");
    assertThat(orig, not(equalTo(copy2)));
  }

  @Test
  public void testStructInequality() throws Exception {
    MySimpleStruct struct1 = new MySimpleStruct(1, "Foo");
    MySimpleStruct struct2 = new MySimpleStruct(2, "Bar");
    assertThat(struct1, is(not(equalTo(struct2))));
    assertThat(struct1, is(not(equalTo(new Object()))));
  }

  @Test
  public void testMutableDefaultValues() throws Exception {
    StructMutable structMutable = new StructMutable();
    assertThat(structMutable.getMyInt16(), equalTo((short) 42));
    assertThat(structMutable.getMyInt32(), equalTo(422));
    assertThat(structMutable.getMyInt64(), equalTo((long) 422222222));
    assertThat(structMutable.getMyString(), equalTo("foo"));
    assertThat(structMutable.isMyBool(), equalTo(true));
    assertThat(structMutable.getMyDouble(), equalTo(42.42));
    assertThat(structMutable.getMySet().size(), equalTo(3));
  }
}
