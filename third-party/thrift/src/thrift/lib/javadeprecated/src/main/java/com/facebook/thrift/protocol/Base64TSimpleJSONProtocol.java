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
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.Base64;

/** JSON protocol implementation for thrift that uses Base64 for Binary. */
final class Base64TSimpleJSONProtocol extends AbstractTSimpleJSONProtocol {

  public Base64TSimpleJSONProtocol(TTransport trans) {
    super(trans);
  }

  @Override
  public void writeBinary(byte[] bin) throws TException {
    byte[] encode = Base64.getEncoder().encode(bin);
    CharBuffer decode = StandardCharsets.UTF_8.decode(ByteBuffer.wrap(encode));
    writeString(decode.toString());
  }

  @Override
  public byte[] readBinary() throws TException {
    return Base64.getDecoder().decode(readString());
  }
}
