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

package com.facebook.thrift.type;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.test.universalname.TestException;
import com.facebook.thrift.test.universalname.TestRequest;
import com.facebook.thrift.test.universalname.TestUnion;
import org.junit.Before;
import org.junit.Test;

public class TypeRegistryRuntimeTest {

  @Before
  public void init() throws Exception {
    // Static binding
    new TestRequest.Builder().build();
    // The rest of the classes, TestResponse, TestException and TestUnion will be
    // registered dynamically during runtime.
  }

  @Test
  public void testDefaultPackageUri() {
    Type type = TypeRegistry.findByHashPrefix("5ccd4ad97abaf670bcc57c5e0aab");
    assertEquals(
        "test.dev/thrift/lib/java/test/universalname/TestResponse",
        type.getUniversalName().getUri());
  }

  @Test
  public void testOverrideThriftUri() {
    Type type = TypeRegistry.findByHashPrefix("ddbc4da47505");
    assertEquals("test.dev/thrift/lib/java/my_request", type.getUniversalName().getUri());
    assertEquals(TestRequest.class, type.getClazz());
  }

  @Test
  public void testExceptionThriftUri() {
    Type type = TypeRegistry.findByHashPrefix("85c6374d408754ed22f16f");
    assertEquals("test.dev/thrift/lib/java/my_exp", type.getUniversalName().getUri());
    assertEquals(TestException.class, type.getClazz());
    assertEquals(true, type.getReader() instanceof Reader);
  }

  @Test
  public void testStructThriftUri() {
    Type type = TypeRegistry.findByHashPrefix("ddbc4da47505da3a");
    assertEquals("test.dev/thrift/lib/java/my_request", type.getUniversalName().getUri());
    assertEquals(TestRequest.class, type.getClazz());
    assertEquals(true, type.getReader() instanceof Reader);
  }

  @Test
  public void testUnionThriftUri() {
    Type type = TypeRegistry.findByHashPrefix("e9584d61d838137c71b00cf3");
    assertEquals(
        "test.dev/thrift/lib/java/test/universalname/TestUnion", type.getUniversalName().getUri());
    assertEquals(TestUnion.class, type.getClazz());
    assertEquals(true, type.getReader() instanceof Reader);
  }

  @Test
  public void testFindByClassName() {
    Type type = TypeRegistry.findByClass(TestRequest.class);
    assertEquals("test.dev/thrift/lib/java/my_request", type.getUniversalName().getUri());
    assertEquals(TestRequest.class, type.getClazz());
    assertEquals(true, type.getReader() instanceof Reader);
  }

  @Test
  public void testUnknownUri() {
    Type type = TypeRegistry.findByHashPrefix("ffffffff");
    assertNull(type);
  }
}
