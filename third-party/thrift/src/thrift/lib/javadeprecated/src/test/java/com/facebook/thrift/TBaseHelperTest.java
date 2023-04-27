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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;

import com.facebook.thrift.java.test.MySimpleStruct;
import com.facebook.thrift.java.test.MySimpleUnion;
import com.facebook.thrift.java.test.NestedStruct;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.junit.Test;

public class TBaseHelperTest {
  @Test
  public void testSetFieldValue() throws Exception {
    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    long id = 321;
    String name = "titi";
    TBaseHelper.setFieldValue(struct, (short) 1, id);
    TBaseHelper.setFieldValue(struct, (short) 2, name);
    assertThat(struct.getId(), equalTo(id));
    assertThat(struct.getName(), equalTo(name));
  }

  @Test
  public void testUnsetFieldValue() throws Exception {
    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    TBaseHelper.setFieldValue(struct, (short) 1, null);
    assertFalse(struct.isSetId());
    assertTrue(struct.isSetName());
  }

  @Test(expected = RuntimeException.class)
  public void testSetFieldValueWithUnknowFieldId() throws Exception {
    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    // Should throw, field id 55 doesn't exist
    TBaseHelper.setFieldValue(struct, (short) 55, 0);
  }

  @Test
  public void testSetFieldValueOnNestedStruct() throws Exception {
    Map<Integer, String> myMap = new HashMap<>();
    myMap.put(1, "one");
    myMap.put(2, "two");
    myMap.put(3, "three");

    Set<Integer> mySet = new HashSet<>();
    mySet.add(51);
    mySet.add(53);
    mySet.add(57);

    List<String> myList = new ArrayList<>();
    myList.add("un");
    myList.add("deux");
    myList.add("trois");

    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    NestedStruct nested = new NestedStruct(myMap, struct, mySet, myList, MySimpleUnion.caseOne(1L));

    Map<Integer, String> expectedMap = new HashMap<>();
    expectedMap.put(1, "un");
    expectedMap.put(2, "deux");
    expectedMap.put(3, "trois");
    TBaseHelper.setFieldValue(nested, (short) 1, expectedMap);

    MySimpleStruct expectedStruct = new MySimpleStruct(42L, ";-)");
    TBaseHelper.setFieldValue(nested, (short) 2, expectedStruct);

    Set<Integer> expectedSet = new HashSet<>();
    expectedSet.add(113);
    expectedSet.add(146);
    expectedSet.add(178);
    TBaseHelper.setFieldValue(nested, (short) 3, expectedSet);

    List<String> expectedList = new ArrayList<>();
    expectedList.add("toto");
    expectedList.add("titi");
    expectedList.add("tata");
    TBaseHelper.setFieldValue(nested, (short) 4, expectedList);

    assertThat(nested.getMyMap(), equalTo(expectedMap));
    assertThat(nested.getMySimpleStruct(), equalTo(expectedStruct));
    assertThat(nested.getMySet(), equalTo(expectedSet));
    assertThat(nested.getMyList(), equalTo(expectedList));
  }

  @Test
  public void testIsSet() throws Exception {
    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    assertTrue(TBaseHelper.isSet(struct, (short) 1));
    struct.unsetId();
    assertFalse(TBaseHelper.isSet(struct, (short) 1));
  }

  @Test
  public void testReflectiveIsSet() throws Exception {
    com.facebook.thrift.android.test.MySimpleStruct struct =
        new com.facebook.thrift.android.test.MySimpleStruct(123L, "toto");
    assertTrue(TBaseHelper.isSet(struct, (short) 1));

    com.facebook.thrift.android.test.MySimpleStruct struct2 =
        new com.facebook.thrift.android.test.MySimpleStruct(null, "toto");
    assertFalse(TBaseHelper.isSet(struct2, (short) 1));
    assertTrue(TBaseHelper.isSet(struct2, (short) 2));
  }

  @Test
  public void testGetFieldValue() throws Exception {
    MySimpleStruct struct = new MySimpleStruct(123L, "toto");
    assertThat(TBaseHelper.getFieldValue(struct, (short) 1), equalTo(struct.getId()));
    assertThat(TBaseHelper.getFieldValue(struct, (short) 2), equalTo(struct.getName()));
  }

  @Test
  public void testReflectiveGetFieldValue() throws Exception {
    com.facebook.thrift.android.test.MySimpleStruct struct =
        new com.facebook.thrift.android.test.MySimpleStruct(123L, "toto");
    assertThat(TBaseHelper.getFieldValue(struct, (short) 1), equalTo(struct.getId()));
    assertThat(TBaseHelper.getFieldValue(struct, (short) 2), equalTo(struct.getName()));
  }

  @Test
  public void testToString() throws Exception {
    long id = 321;
    String name = "titi";

    MySimpleStruct struct = new MySimpleStruct(id, name);
    com.facebook.thrift.android.test.MySimpleStruct struct2 =
        new com.facebook.thrift.android.test.MySimpleStruct(id, name);

    assertThat(struct.toString(), equalTo(struct2.toString()));
    assertThat(struct.toString(1, false), equalTo(struct2.toString(1, false)));
  }

  @Test
  public void testToStringOfUnion() throws Exception {
    MySimpleUnion struct = MySimpleUnion.caseOne(1L);
    com.facebook.thrift.android.test.MySimpleUnion struct2 =
        com.facebook.thrift.android.test.MySimpleUnion.caseOne(1L);

    assertThat(struct.toString(), equalTo(struct2.toString()));
    assertThat(struct.toString(1, false), equalTo(struct2.toString(1, false)));

    long id = 321;
    String name = "titi";

    MySimpleStruct simpleStruct = new MySimpleStruct(id, name);
    com.facebook.thrift.android.test.MySimpleStruct simpleStruct2 =
        new com.facebook.thrift.android.test.MySimpleStruct(id, name);

    MySimpleUnion struct3 = MySimpleUnion.caseFour(simpleStruct);
    com.facebook.thrift.android.test.MySimpleUnion struct4 =
        com.facebook.thrift.android.test.MySimpleUnion.caseFour(simpleStruct2);

    assertThat(struct3.toString(), equalTo(struct4.toString()));
    assertThat(struct3.toString(1, false), equalTo(struct4.toString(1, false)));
  }

  @Test
  public void testToStringWithNull() throws Exception {
    long id = 321;
    String name = null;

    MySimpleStruct struct = new MySimpleStruct(id, name);
    com.facebook.thrift.android.test.MySimpleStruct struct2 =
        new com.facebook.thrift.android.test.MySimpleStruct(id, name);

    assertThat(struct.toString(), equalTo(struct2.toString()));
    assertThat(struct.toString(1, false), equalTo(struct2.toString(1, false)));
  }

  @Test
  public void testNestedToString() throws Exception {
    long id = 321;
    String name = "titi";

    Map<Integer, String> myMap = new HashMap<>();
    myMap.put(1, "one");
    myMap.put(2, "two");
    myMap.put(3, "three");

    Set<Integer> mySet = new HashSet<>();
    mySet.add(51);
    mySet.add(53);
    mySet.add(57);

    List<String> myList = new ArrayList<>();
    myList.add("un");
    myList.add("deux");
    myList.add("trois");

    MySimpleStruct struct = new MySimpleStruct(id, name);
    MySimpleUnion union = MySimpleUnion.caseOne(1L);
    NestedStruct nested = new NestedStruct(myMap, struct, mySet, myList, union);

    com.facebook.thrift.android.test.MySimpleStruct struct2 =
        new com.facebook.thrift.android.test.MySimpleStruct(id, name);
    com.facebook.thrift.android.test.MySimpleUnion union2 =
        com.facebook.thrift.android.test.MySimpleUnion.caseOne(1L);
    com.facebook.thrift.android.test.NestedStruct nested2 =
        new com.facebook.thrift.android.test.NestedStruct(myMap, struct2, mySet, myList, union2);

    assertThat(struct.toString(), equalTo(struct2.toString()));
    assertThat(struct.toString(1, false), equalTo(struct2.toString(1, false)));
  }
}
