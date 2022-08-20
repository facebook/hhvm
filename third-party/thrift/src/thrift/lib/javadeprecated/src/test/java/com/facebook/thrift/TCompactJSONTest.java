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
import static org.junit.Assert.assertThat;

import com.facebook.thrift.java.test.StructWithAllTypes;
import com.facebook.thrift.java.test.StructWithMaps;
import com.facebook.thrift.protocol.TCompactJSONProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.transport.TIOStreamTransport;
import com.facebook.thrift.transport.TMemoryInputTransport;
import com.facebook.thrift.utils.StandardCharsets;
import java.io.ByteArrayOutputStream;
import java.util.HashMap;
import org.junit.Test;

public class TCompactJSONTest {

  @Test
  public void testCompactJSONProtocol() throws Exception {
    final ByteArrayOutputStream bos = new ByteArrayOutputStream();
    final TIOStreamTransport out = new TIOStreamTransport(bos);
    final TProtocol oproto = new TCompactJSONProtocol(out);

    final StructWithAllTypes struct = new StructWithAllTypes.Builder().setBb(true).build();
    struct.write(oproto);

    final String json = new String(bos.toByteArray(), StandardCharsets.UTF_8);

    TMemoryInputTransport in = new TMemoryInputTransport(json.getBytes(StandardCharsets.UTF_8));
    TProtocol iproto = new TCompactJSONProtocol(in);

    StructWithAllTypes read0 = new StructWithAllTypes();
    read0.read(iproto);
    assertThat(read0, equalTo(struct));

    // Test SimpleJSON compatibility (i.e. boolean serialized as true/false)
    String oldJson = json.replaceAll("\"bb\":1", "\"bb\":true");

    in = new TMemoryInputTransport(oldJson.getBytes(StandardCharsets.UTF_8));
    iproto = new TCompactJSONProtocol(in);

    StructWithAllTypes read1 = new StructWithAllTypes();
    read1.read(iproto);
    assertThat(read1, equalTo(struct));
  }

  @Test
  public void testCompactJSONWithMaps() throws Exception {
    final ByteArrayOutputStream bos = new ByteArrayOutputStream();
    final TIOStreamTransport out = new TIOStreamTransport(bos);
    final TProtocol oproto = new TCompactJSONProtocol(out);

    final StructWithMaps struct =
        new StructWithMaps.Builder()
            .setStringstrings(
                new HashMap<String, String>() {
                  {
                    put("1", "one");
                  }
                })
            .setBoolstrings(
                new HashMap<Boolean, String>() {
                  {
                    put(true, "VRAI");
                  }
                })
            .setStringbools(
                new HashMap<String, Boolean>() {
                  {
                    put("FAUX", false);
                  }
                })
            .setIntstrings(
                new HashMap<Integer, String>() {
                  {
                    put(2, "two");
                  }
                })
            .setStringints(
                new HashMap<String, Integer>() {
                  {
                    put("three", 3);
                  }
                })
            .build();
    struct.write(oproto);

    String json = new String(bos.toByteArray(), StandardCharsets.UTF_8);
    TMemoryInputTransport in = new TMemoryInputTransport(json.getBytes(StandardCharsets.UTF_8));
    TProtocol iproto = new TCompactJSONProtocol(in);

    StructWithMaps read0 = new StructWithMaps();
    read0.read(iproto);
    assertThat(read0, equalTo(struct));

    // Test TSimpleJSON compatibility (i.e. boolean serialized as 0/1)
    json = json.replace("\"boolstrings\":{\"1\":\"VRAI\"}", "\"boolstrings\":{true:\"VRAI\"}");
    json = json.replace("\"stringbools\":{\"FAUX\":0}", "\"stringbools\":{\"FAUX\":false}");
    in = new TMemoryInputTransport(json.getBytes(StandardCharsets.UTF_8));
    iproto = new TCompactJSONProtocol(in);

    StructWithMaps read1 = new StructWithMaps();
    read1.read(iproto);
    assertThat(read1, equalTo(struct));
  }
}
