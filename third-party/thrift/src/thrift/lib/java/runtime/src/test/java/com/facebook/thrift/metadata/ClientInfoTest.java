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

package com.facebook.thrift.metadata;

import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;

public class ClientInfoTest {

  @Before
  public void init() {
    ClientInfo.reset();
  }

  @Test
  public void testEmpty() {
    assertTrue(ClientInfo.getClientMetadata().contains("java"));
  }

  @Test
  public void testClientInfo() {
    ClientInfo.addClient(ClientInfo.Client.ABSTRACT);
    assertTrue(ClientInfo.getClientMetadata().contains("abstract.java"));

    ClientInfo.addTransport(ClientInfo.Transport.HEADER);
    assertTrue(ClientInfo.getClientMetadata().contains("abstract.header.java"));

    ClientInfo.addClient(ClientInfo.Client.ABSTRACT);
    assertTrue(ClientInfo.getClientMetadata().contains("abstract.header.java"));

    ClientInfo.addRuntime(ClientInfo.Runtime.SWIFT);
    assertTrue(ClientInfo.getClientMetadata().contains("swift.abstract.header.java"));

    ClientInfo.addRuntime(ClientInfo.Runtime.JAVA2);
    assertTrue(ClientInfo.getClientMetadata().contains("java2.swift.abstract.header.java"));
  }

  @Test
  public void testClientInfoWrapper() {
    ClientInfo.addClient(ClientInfo.Client.WRAPPER);
    assertTrue(ClientInfo.getClientMetadata().contains("wrapper.java"));

    ClientInfo.addTransport(ClientInfo.Transport.HEADER);
    assertTrue(ClientInfo.getClientMetadata().contains("wrapper.header.java"));

    ClientInfo.addRuntime(ClientInfo.Runtime.SWIFT);
    assertTrue(ClientInfo.getClientMetadata().contains("swift.wrapper.header.java"));
  }
}
