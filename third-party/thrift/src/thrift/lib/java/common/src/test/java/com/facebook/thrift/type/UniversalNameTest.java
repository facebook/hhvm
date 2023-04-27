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

import io.netty.buffer.ByteBufUtil;
import org.junit.Test;

public class UniversalNameTest {

  @Test
  public void testHashPrefix() {
    UniversalName un = new UniversalName("foo.com/storage/cloud/bucket");
    assertEquals("foo.com/storage/cloud/bucket", un.getUri());
    assertEquals("b7f183390eaa9c1abef2f452c05bd4b2", un.getHash());
    assertEquals("b7f183390eaa9c1abef2f452c05bd4b2", ByteBufUtil.hexDump(un.getHashBytes()));
    assertEquals("b7f183390eaa9c1a", un.getHashPrefix(2));
    assertEquals("b7f183390eaa9c1a", un.getHashPrefix(3));
    assertEquals("b7f183390eaa9c1a", un.getHashPrefix(4));
    assertEquals("b7f183390eaa9c1a", ByteBufUtil.hexDump(un.getHashPrefixBytes(4)));
    // assertEquals(UniversalHashAlgorithm.SHA256, un.getAlgorithm());
    assertEquals(true, un.toString().contains("foo.com/storage/cloud/bucket"));
    assertEquals(true, un.toString().contains("b7f183390eaa9c1abef2f452c05bd4b2"));
    assertEquals(true, un.toString().contains("SHA256"));
  }

  private void checkUri(String uri, boolean valid) {
    try {
      UniversalName un = new UniversalName(uri);
      assertEquals("URI validation should pass, " + uri, true, valid);
    } catch (InvalidUniversalNameURIException expected) {
      assertEquals("URI validation should fail, " + uri, false, valid);
    }
  }

  @Test
  public void testInvalidUri() throws Exception {
    checkUri("Foo.com/cloud/bucket", false);
    checkUri("foo.COM/cloud/bucket", false);
    checkUri("foo/cloud/bucket", false);
    checkUri("foo..com/cloud/bucket", false);
    checkUri("foo.com//cloud/bucket", false);
    checkUri("foo.com/#/cloud/bucket", false);
    checkUri("foo.com/??/cloud/bucket", false);
    checkUri("foo.com/", false);
    checkUri("foo.com/storage", false);
    checkUri("foo.com/storage/", false);
    checkUri("foo.com/storage/+", false);
    checkUri("foo.com_/storage/bucket", false);
    checkUri("_foo.com/storage/Bucket", false);
    checkUri("foo", false);
  }

  @Test
  public void testValidUri() throws Exception {
    new UniversalName("foo.com/storage/Bucket");
    checkUri("facebook.com/thrift/conformance/Object", true);
    checkUri("f.com/_thri-ft/c-onformance/-_Object", true);
    checkUri("foo.c/cloud/bucket", true);
    checkUri("-foo.com/cloud/bucket", true);
    checkUri("foo-.com/cloud/bucket", true);
    checkUri("foo.com-/storage/bucket", true);
  }

  @Test
  public void testMinHashSize() throws Exception {
    UniversalName un = new UniversalName("foo.com/storage/Bucket");
    assertEquals("56f66fc99e84b522f44a72cbfc83dd4c", un.getHash());
    assertEquals("56f66fc99e84b522f44a72cbfc83dd4c", un.getHashPrefix(200));
    assertEquals("56f66fc99e84b522f44a72cb", un.getHashPrefix(12));
    assertEquals("56f66fc99e84b522f44a", un.getHashPrefix(10));
    assertEquals("56f66fc99e84b522f4", un.getHashPrefix(9));
    assertEquals("56f66fc99e84b522", un.getHashPrefix(8));
    assertEquals("56f66fc99e84b522", un.getHashPrefix(7));
    assertEquals("56f66fc99e84b522", un.getHashPrefix(5));
    assertEquals("56f66fc99e84b522", un.getHashPrefix(1));
    assertEquals("56f66fc99e84b522", un.getHashPrefix(0));
    assertEquals(true, un.preferHash());
    assertEquals(true, un.preferHash());
  }

  @Test
  public void testShortUri() throws Exception {
    UniversalName un = new UniversalName("a.co/b/c");
    assertEquals("a.co/b/c", un.getUri());
    assertEquals(false, un.preferHash());
    assertEquals(false, un.preferHash(12));
    assertEquals(false, un.preferHash(8));
    assertEquals(false, un.preferHash(7));
  }

  @Test
  public void testPreferHashSize() throws Exception {
    UniversalName un = new UniversalName("foo.com/ab/c");
    assertEquals("foo.com/ab/c", un.getUri());
    assertEquals(false, un.preferHash());
    assertEquals(true, un.preferHash(10));
    assertEquals(false, un.preferHash(13));
    assertEquals(true, un.preferHash(8));
    assertEquals(true, un.preferHash(5));
  }
}
