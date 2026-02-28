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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TreeMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class TypeRegistryTest {

  /**
   * Test data
   * 1adf51f75f27b0a6ad24d61a99740fa0=UniversalName{uri='facebook.com/thrift/conformance/Object15'},
   * 21142e0898974336ea08e21cbb25d301=UniversalName{uri='facebook.com/thrift/conformance/Object1'},
   * 21d3d2ffa4145d984f624fb119ecb82c=UniversalName{uri='facebook.com/thrift/conformance/Object9'},
   * 24cc6a13c816f580e7edd7481eea9095=UniversalName{uri='facebook.com/thrift/conformance/Object6'},
   * 47e4f48ecea06c3d1f438b8033ed8094=UniversalName{uri='facebook.com/thrift/conformance/Object5'},
   * 48c3eb9f4d440466ab0ae35595409cc3=UniversalName{uri='facebook.com/thrift/conformance/Object10'},
   * 496797da83f1c1a78c8becb162509595=UniversalName{uri='facebook.com/thrift/conformance/Object12'},
   * 55141c6fb32a48cb7b723e1362954f74=UniversalName{uri='facebook.com/thrift/conformance/Object14'},
   * 57f3de5f68345631a35d70568768e48f=UniversalName{uri='facebook.com/thrift/conformance/Object2'},
   * 5b1388462f00e567880eff35410cc929=UniversalName{uri='facebook.com/thrift/conformance/Object17'},
   * 6d3fc2c94228b82c0b9deb6cca9cbf1f=UniversalName{uri='facebook.com/thrift/conformance/Object11'},
   * 6e8b9eea5da808df7e2c5486d039dfaa=UniversalName{uri='facebook.com/thrift/conformance/Object7'},
   * 8f87059f6ccf36fe54f9df560f9782d6=UniversalName{uri='facebook.com/thrift/conformance/Object18'},
   * 9fb078b2a741bd2192ecd7172f73a00e=UniversalName{uri='facebook.com/thrift/conformance/Object16'},
   * b01f521340cd78eb039ea6b0165ebb9c=UniversalName{uri='facebook.com/thrift/conformance/Object8'},
   * bb187fd7a0f052fd2b19990669045b4d=UniversalName{uri='facebook.com/thrift/conformance/Object4'},
   * c8f7ed418018ec0cdb8dfc3c90a20c6a=UniversalName{uri='facebook.com/thrift/conformance/Object3'},
   * e47a302b614f2525f3f2ee748d005c14=UniversalName{uri='facebook.com/thrift/conformance/Object19'},
   * e4def5c5ad3f845943e7627163991b7d=UniversalName{uri='facebook.com/thrift/conformance/Object13'},
   * fb582ea2a9513a558c6d3de31e416ace=UniversalName{uri='facebook.com/thrift/conformance/Object0'}
   */
  private final Class[] classes =
      new Class[] {
        Object.class, UniversalName.class, Type.class, Exception.class, Throwable.class,
        String.class, Integer.class, Byte.class, Float.class, Double.class,
        Map.class, List.class, LinkedList.class, Set.class, TreeMap.class,
        Timer.class, Thread.class, Math.class, AtomicInteger.class, AtomicLong.class
      };

  @BeforeEach
  public void init() {
    for (int i = 0; i < 20; i++) {
      TypeRegistry.add(
          new Type(
              new UniversalName("facebook.com/thrift/conformance/Object" + i),
              classes[i],
              p -> null));
    }
  }

  @AfterEach
  public void clear() {
    TypeRegistry.clear();
  }

  @Test
  public void testRegistry() throws Exception {
    assertEquals(20, TypeRegistry.size());
    Type type = TypeRegistry.findByHashPrefix("2114");

    assertEquals("facebook.com/thrift/conformance/Object1", type.getUniversalName().getUri());
    assertEquals(UniversalName.class, type.getClazz());
    type =
        TypeRegistry.findByUniversalName(
            new UniversalName("facebook.com/thrift/conformance/Object1"));
    assertEquals(UniversalName.class, type.getClazz());

    type = TypeRegistry.findByHashPrefix("1adf");
    assertEquals("facebook.com/thrift/conformance/Object15", type.getUniversalName().getUri());
    assertEquals(Timer.class, type.getClazz());
    type =
        TypeRegistry.findByUniversalName(
            new UniversalName("facebook.com/thrift/conformance/Object15"));
    assertEquals(Timer.class, type.getClazz());

    type = TypeRegistry.findByHashPrefix("fb58");
    assertEquals("facebook.com/thrift/conformance/Object0", type.getUniversalName().getUri());
    assertEquals(Object.class, type.getClazz());

    type = TypeRegistry.findByHashPrefix(toByteBuf("e4de"));
    assertEquals("facebook.com/thrift/conformance/Object13", type.getUniversalName().getUri());
    assertEquals(Set.class, type.getClazz());
  }

  @Test
  public void testFindByClassName() throws Exception {
    Type type = TypeRegistry.findByClass(Type.class);
    assertEquals("57f3de5f68345631a35d70568768e48f", type.getUniversalName().getHash());
  }

  @Test
  public void testFindByNonExistPrefix() throws Exception {
    assertEquals(null, TypeRegistry.findByHashPrefix("11111111"));
  }

  @Test
  public void testAmbiguousPrefix() throws Exception {
    AmbiguousUniversalNameException exception =
        assertThrows(
            AmbiguousUniversalNameException.class,
            () -> {
              TypeRegistry.findByHashPrefix("21");
            });
    assertTrue(exception.getMessage().contains("21"));
  }

  @Test
  public void testExist() throws Exception {
    assertTrue(TypeRegistry.exist(UniversalName.class));
    assertTrue(TypeRegistry.exist(Map.class));
    assertFalse(TypeRegistry.exist(Method.class));

    assertTrue(TypeRegistry.exist(new UniversalName("facebook.com/thrift/conformance/Object12")));
    assertFalse(TypeRegistry.exist(new UniversalName("facebook.com/thrift/conformance/Object120")));
  }

  @Test
  public void testExistingUniversalNameAndClass() throws Exception {
    IllegalArgumentException exception =
        assertThrows(
            IllegalArgumentException.class,
            () -> {
              TypeRegistry.add(
                  new Type(
                      new UniversalName("facebook.com/thrift/conformance/Object12"),
                      UniversalName.class,
                      null));
            });
    assertTrue(
        exception.getMessage().contains("Universal name already registered with another class"));
  }

  @Test
  public void testExistingUniversalName() throws Exception {
    IllegalArgumentException exception =
        assertThrows(
            IllegalArgumentException.class,
            () -> {
              TypeRegistry.add(
                  new Type(
                      new UniversalName("facebook.com/thrift/conformance/Object12"),
                      Constructor.class,
                      null));
            });
    assertTrue(
        exception.getMessage().contains("Universal name already registered with another class"));
  }

  @Test
  public void testExistingClass() throws Exception {
    IllegalArgumentException exception =
        assertThrows(
            IllegalArgumentException.class,
            () -> {
              TypeRegistry.add(
                  new Type(
                      new UniversalName("facebook.com/thrift/conformance/Object100"),
                      UniversalName.class,
                      null));
            });
    assertTrue(
        exception
            .getMessage()
            .contains("Class name already registered with another universal name"));
  }

  @Test
  public void testShortUri() throws Exception {
    UniversalName un = new UniversalName("a.co/b/c");
    TypeRegistry.add(new Type(un, TypeRegistryTest.class, null));
    Type type = TypeRegistry.findByUniversalName(new UniversalName("a.co/b/c"));
    assertEquals(un, type.getUniversalName());
  }

  @Test
  public void testLocalCache() throws Exception {
    Type type = TypeRegistry.findByHashPrefix("2114");
    assertEquals("facebook.com/thrift/conformance/Object1", type.getUniversalName().getUri());

    type = TypeRegistry.findByHashPrefix("2114");
    assertEquals("facebook.com/thrift/conformance/Object1", type.getUniversalName().getUri());
  }

  @Test
  public void testEmptyRegistry() {
    TypeRegistry.clear();
    Type type = TypeRegistry.findByHashPrefix("2114");
    assertEquals(null, type);
  }

  private ByteBuf toByteBuf(String hex) {
    return Unpooled.wrappedBuffer(ByteBufUtil.decodeHexDump(hex));
  }

  @Test
  public void testCustomHashGenerator() {
    TypeRegistry.add(
        new Type(
            new UniversalName(
                "foo.com/thrift/conformance/1",
                p -> {
                  return toByteBuf("abababababababab");
                }),
            HashAlgorithmSHA256.class,
            p -> null));
    TypeRegistry.add(
        new Type(
            new UniversalName(
                "foo.com/thrift/conformance/2",
                p -> {
                  return toByteBuf("abababababababac");
                }),
            Runtime.class,
            p -> null));

    Type type = TypeRegistry.findByHashPrefix("abababababababab");
    assertEquals("foo.com/thrift/conformance/1", type.getUniversalName().getUri());

    type = TypeRegistry.findByHashPrefix("abababababababac");
    assertEquals("foo.com/thrift/conformance/2", type.getUniversalName().getUri());

    try {
      type = TypeRegistry.findByHashPrefix("ababababab");
      fail();
    } catch (AmbiguousUniversalNameException a) {
      // expected.
    }
  }

  @Test
  public void testTypeListAmbiguousPrefix() throws Exception {
    AmbiguousUniversalNameException exception =
        assertThrows(
            AmbiguousUniversalNameException.class,
            () -> {
              TypeRegistry.findByHashPrefix("0577");
            });
    assertTrue(exception.getMessage().contains("0577"));
  }
}
