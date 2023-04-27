/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package com.facebook.thrift.protocol;

import com.facebook.thrift.TException;
import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.utils.StandardCharsets;

/** JSON protocol implementation for thrift that saves binary as a UFT8 string without encoding. */
final class DefaultTSimpleJSONProtocol extends AbstractTSimpleJSONProtocol {

  /**
   * Constructor
   *
   * @param trans
   */
  public DefaultTSimpleJSONProtocol(TTransport trans) {
    super(trans);
  }

  @Override
  public void writeBinary(byte[] bin) throws TException {
    String data = new String(bin, StandardCharsets.UTF_8);
    writeString(data);
  }

  @Override
  public byte[] readBinary() throws TException {
    return readString().getBytes(StandardCharsets.UTF_8);
  }
}
