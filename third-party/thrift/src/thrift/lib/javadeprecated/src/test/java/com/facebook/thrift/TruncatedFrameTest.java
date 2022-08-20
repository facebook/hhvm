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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import com.facebook.thrift.java.test.MyListStruct;
import com.facebook.thrift.java.test.MyMapStruct;
import com.facebook.thrift.java.test.MySetStruct;
import com.facebook.thrift.java.test.MyStringStruct;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TCompactProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolException;
import com.facebook.thrift.protocol.TType;
import com.facebook.thrift.transport.TMemoryInputTransport;
import org.junit.Test;

public class TruncatedFrameTest {
  private static final byte[] kBinaryListEncoding = {
    TType.LIST, // Field Type = List
    (byte) 0x00,
    (byte) 0x01, // Field id = 1
    TType.I64, // List type = i64
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0xFF, // List length (255 > 3!)
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // value = 1L
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x02, // value = 2L
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x03, // value = 3L
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactListEncoding = {
    (byte) 0b00011001, // field id delta (0001) + type (1001) = List
    (byte) 0b11100110, // list size (0111) and 7>3 + list type (0110) = i64
    (byte) 0x02, // value = 1 (zigzag encoded)
    (byte) 0x04, // value = 2 (zigzag encoded)
    (byte) 0x06, // value = 3 (zigzag encoded)
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactListEncoding2 = {
    (byte) 0b00011001, // field id delta (0001) + type (1001) = List
    (byte) 0b11110110, // list size magic marker (1111) + list type (0110) = i64
    (byte) 0x64, // list actual size (varint of 1 byte here) = 100
    (byte) 0x02, // value = 1 (zigzag encoded)
    (byte) 0x04, // value = 2 (zigzag encoded)
    (byte) 0x06, // value = 3 (zigzag encoded)
    (byte) 0x00, // Stop
  };

  public static void testTruncated(TBase struct, TProtocol iprot) throws Exception {
    try {
      struct.read(iprot);
      assertTrue("Not reachable", false);
    } catch (TProtocolException ex) {
      assertEquals(
          "Not enough bytes to read the entire message, the data appears to be truncated",
          ex.getMessage());
    }
  }

  @Test
  public void testListBinary() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kBinaryListEncoding);
    TProtocol iprot = new TBinaryProtocol(buf);
    testTruncated(new MyListStruct(), iprot);
  }

  @Test
  public void testListCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactListEncoding);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MyListStruct(), iprot);
  }

  @Test
  public void testLongListCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactListEncoding2);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MyListStruct(), iprot);
  }

  private static final byte[] kBinarySetEncoding = {
    TType.SET, // Field Type = Set
    (byte) 0x00,
    (byte) 0x01, // Field id = 1
    TType.I64, // Set type = i64
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0xFF, // Set length (255 > 3!)
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // value = 1L
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x02, // value = 2L
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x03, // value = 3L
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactSetEncoding = {
    (byte) 0b00011010, // field id delta (0001) + type (1010) = Set
    (byte) 0b01110110, // set size (0111) and 7>3 + set type (0110) = i64
    (byte) 0x02, // value = 1 (zigzag encoded)
    (byte) 0x04, // value = 2 (zigzag encoded)
    (byte) 0x06, // value = 3 (zigzag encoded)
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactSetEncoding2 = {
    (byte) 0b00011010, // field id delta (0001) + type (1010) = Set
    (byte) 0b11110110, // set size magic marker (1111) + set type (0110) = i64
    (byte) 0x64, // set actual size (varint of 1 byte here) = 100
    (byte) 0x02, // value = 1 (zigzag encoded)
    (byte) 0x04, // value = 2 (zigzag encoded)
    (byte) 0x06, // value = 3 (zigzag encoded)
    (byte) 0x00, // Stop
  };

  @Test
  public void testSetBinary() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kBinarySetEncoding);
    TProtocol iprot = new TBinaryProtocol(buf);
    testTruncated(new MySetStruct(), iprot);
  }

  @Test
  public void testSetCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactSetEncoding);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MySetStruct(), iprot);
  }

  @Test
  public void testLongSetCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactSetEncoding2);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MySetStruct(), iprot);
  }

  private static final byte[] kBinaryMapEncoding = {
    TType.MAP, // field type = Map
    (byte) 0x00,
    (byte) 0x01, // field id = 1
    TType.I64, // key type = i64
    TType.STRING, // value type = string
    (byte) 0x00,
    (byte) 0xFF,
    (byte) 0xFF,
    (byte) 0xFF, // size = 0x00FFFFFF
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00, // key = 0
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // string size = 1
    (byte) 0x30, // string value = "0"
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // key = 1
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // string size = 1
    (byte) 0x31, // string value = "1"
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x02, // key = 2
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x01, // string size = 1
    (byte) 0x32, // string value = "2"
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactMapEncoding = {
    (byte) 0b00011011, // field id delta (0001) + type (1011) = Map
    (byte) 0x64, // map size (varint = 100)
    (byte) 0b01101000, // key type (0110) i64, value type (1000) string
    (byte) 0x00, // key value = 0
    (byte) 0x01, // value: string size = 1
    (byte) 0x30, // string content = "0"
    (byte) 0x02, // key value = 1 (zigzag encoded)
    (byte) 0x01, // value: string size = 1
    (byte) 0x31, // string content = "1"
    (byte) 0x04, // key value = 2 (zigzag encoded)
    (byte) 0x01, // value: string size = 1
    (byte) 0x32, // string content = "2"
    (byte) 0x00, // Stop
  };

  @Test
  public void testMapBinary() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kBinaryMapEncoding);
    TProtocol iprot = new TBinaryProtocol(buf);
    testTruncated(new MyMapStruct(), iprot);
  }

  @Test
  public void testMapCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactMapEncoding);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MyMapStruct(), iprot);
  }

  private static final byte[] kBinaryStringEncoding = {
    TType.STRING, // Field Type = string
    (byte) 0x00,
    (byte) 0x01, // Field id = 1
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0x00,
    (byte) 0xFF, // string length (255!)
    (byte) 0x48,
    (byte) 0x65,
    (byte) 0x6C,
    (byte) 0x6C,
    (byte) 0x6F,
    (byte) 0x2C,
    (byte) 0x20,
    (byte) 0x57,
    (byte) 0x6F,
    (byte) 0x72,
    (byte) 0x6C,
    (byte) 0x64,
    (byte) 0x21, // string chars: "Hello, World!"
    (byte) 0x00, // Stop
  };

  private static final byte[] kCompactStringEncoding = {
    (byte) 0b00011000, // field id delta (0001) + type (1000) = Binary
    (byte) 0xFF,
    (byte) 0x0F, // string size (varint) = 0x0FFF (4095)
    (byte) 0x48,
    (byte) 0x65,
    (byte) 0x6C,
    (byte) 0x6C,
    (byte) 0x6F,
    (byte) 0x2C,
    (byte) 0x20,
    (byte) 0x57,
    (byte) 0x6F,
    (byte) 0x72,
    (byte) 0x6C,
    (byte) 0x64,
    (byte) 0x21, // string chars: "Hello, World!"
    (byte) 0x00, // Stop
  };

  @Test
  public void testStringBinary() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kBinaryStringEncoding);
    TProtocol iprot = new TBinaryProtocol(buf);
    testTruncated(new MyStringStruct(), iprot);
  }

  @Test
  public void testStringCompact() throws Exception {
    TMemoryInputTransport buf = new TMemoryInputTransport(kCompactStringEncoding);
    TProtocol iprot = new TCompactProtocol(buf);
    testTruncated(new MyStringStruct(), iprot);
  }
}
