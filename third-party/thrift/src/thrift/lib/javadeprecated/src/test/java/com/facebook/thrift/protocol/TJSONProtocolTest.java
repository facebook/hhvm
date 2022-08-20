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

package com.facebook.thrift.protocol;

import com.facebook.thrift.java.test.MySimpleUnion;
import com.facebook.thrift.transport.TIOStreamTransport;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import org.junit.Assert;
import org.junit.Test;

public class TJSONProtocolTest {
  @Test
  public void serializedUnion_withTJSONProtocol_shouldBeDeserializable() throws Exception {
    MySimpleUnion union = MySimpleUnion.caseThree("akdhjba");

    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(bos);
    TJSONProtocol protocol1 = new TJSONProtocol(transport);
    union.write(protocol1);

    ByteArrayInputStream bis = new ByteArrayInputStream(bos.toByteArray());
    TIOStreamTransport transport1 = new TIOStreamTransport(bis);
    TJSONProtocol protocol2 = new TJSONProtocol(transport1);
    MySimpleUnion read = new MySimpleUnion();
    read.read(protocol2);
    Assert.assertEquals(union, read);
  }
}
