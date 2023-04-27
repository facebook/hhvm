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
import static org.hamcrest.Matchers.is;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.java.test.StringAndList;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TCompactProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolException;
import com.facebook.thrift.transport.TIOStreamTransport;
import com.facebook.thrift.transport.TMemoryInputTransport;
import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;
import org.junit.Before;
import org.junit.Test;

public class ProtocolLimitTest {

  private static final String SMALL_STRING;
  private static final String BIG_STRING;
  private static final List<Integer> SMALL_INTS;
  private static final List<Integer> BIG_INTS;
  private static final int MAX_VALUE = 256;

  static {
    SMALL_INTS =
        new ArrayList<Integer>() {
          {
            add(1);
            add(2);
            add(3);
          }
        };
    SMALL_STRING = "BLABLA";
    BIG_INTS = new ArrayList<>();
    StringBuilder builder = new StringBuilder();
    for (int i = 0; i < MAX_VALUE + 1; i++) {
      builder.append('X');
      BIG_INTS.add(i);
    }
    BIG_STRING = builder.toString();
  }

  private ByteArrayOutputStream bos;
  private TIOStreamTransport out;
  private TProtocol binaryOutProto;
  private TProtocol compactOutProto;

  @Before
  public void setup() {
    bos = new ByteArrayOutputStream();
    out = new TIOStreamTransport(bos);
    binaryOutProto =
        new TBinaryProtocol.Factory(false, true, MAX_VALUE, MAX_VALUE).getProtocol(out);
    compactOutProto = new TCompactProtocol.Factory(MAX_VALUE, MAX_VALUE).getProtocol(out);
  }

  @Test
  public void binaryProtocol_works_whenStringAndContainerAreBelowLimit() throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(SMALL_STRING).setMyInts(SMALL_INTS).build();
    struct.write(binaryOutProto);

    TMemoryInputTransport in = new TMemoryInputTransport(bos.toByteArray());
    TProtocol iproto =
        new TBinaryProtocol.Factory(false, true, MAX_VALUE, MAX_VALUE).getProtocol(in);
    StringAndList readStruct = new StringAndList();
    readStruct.read(iproto);

    assertThat(readStruct, is(equalTo(struct)));
  }

  private void checkBinaryReadingOverLimit(StringAndList struct) throws Exception {
    struct.write(binaryOutProto);

    TMemoryInputTransport in = new TMemoryInputTransport(bos.toByteArray());
    TProtocol iproto =
        new TBinaryProtocol.Factory(false, true, MAX_VALUE, MAX_VALUE).getProtocol(in);
    StringAndList readStruct = new StringAndList();
    readStruct.read(iproto);
  }

  @Test(expected = TProtocolException.class)
  public void binaryProtocol_throwsTProtocolException_whenStringExceedsLimit() throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(BIG_STRING).setMyInts(SMALL_INTS).build();
    checkBinaryReadingOverLimit(struct);
  }

  @Test(expected = TProtocolException.class)
  public void binaryProtocol_throwsTProtocolException_whenContainerExceedsLimit() throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(SMALL_STRING).setMyInts(BIG_INTS).build();
    checkBinaryReadingOverLimit(struct);
  }

  @Test
  public void compactProtocol_works_whenStringAndContainerAreBelowLimit() throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(SMALL_STRING).setMyInts(SMALL_INTS).build();
    struct.write(compactOutProto);

    TMemoryInputTransport in = new TMemoryInputTransport(bos.toByteArray());
    TProtocol iproto = new TCompactProtocol.Factory(MAX_VALUE, MAX_VALUE).getProtocol(in);
    StringAndList readStruct = new StringAndList();
    readStruct.read(iproto);

    assertThat(readStruct, is(equalTo(struct)));
  }

  private void checkCompactReadingOverLimit(StringAndList struct) throws Exception {
    struct.write(compactOutProto);

    TMemoryInputTransport in = new TMemoryInputTransport(bos.toByteArray());
    TProtocol iproto = new TCompactProtocol.Factory(MAX_VALUE, MAX_VALUE).getProtocol(in);
    StringAndList readStruct = new StringAndList();
    readStruct.read(iproto);
  }

  @Test(expected = TProtocolException.class)
  public void compactProtocol_throwsTProtocolException_whenStringExceedsLimit() throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(BIG_STRING).setMyInts(SMALL_INTS).build();
    checkCompactReadingOverLimit(struct);
  }

  @Test(expected = TProtocolException.class)
  public void compactProtocol_throwsTProtocolException_whenContainerExceedsLimit()
      throws Exception {
    StringAndList struct =
        new StringAndList.Builder().setMyString(SMALL_STRING).setMyInts(BIG_INTS).build();
    checkCompactReadingOverLimit(struct);
  }
}
