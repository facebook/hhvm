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

import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TIOStreamTransport;
import com.facebook.thrift.utils.ByteBufferUtils;
import java.io.ByteArrayInputStream;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

/** Generic utility methods to create deserialization protocols. */
public class TDeserializeUtils {
  /**
   * Get a protocol for deserializing a Thrift object from a byte array.
   *
   * @param bytes The array to read from
   */
  public static TProtocol protocolFromByteArray(TProtocolFactory protocolFactory, byte[] bytes)
      throws TException {
    return protocolFactory.getProtocol(new TIOStreamTransport(new ByteArrayInputStream(bytes)));
  }

  /**
   * Get a protocol for deserializing a Thrift object from a byte buffer.
   *
   * @param buffer The byte buffer to read from.
   */
  public static TProtocol protocolFromByteBuffer(
      TProtocolFactory protocolFactory, ByteBuffer buffer) throws TException {
    byte[] byteData = ByteBufferUtils.toBytes(buffer);
    return protocolFromByteArray(protocolFactory, byteData);
  }

  /**
   * Get a protocol for deserializing a Thrift object from a Java string, using a specified
   * character set for decoding.
   *
   * @param data The string to read from
   * @param charset Valid JVM charset
   */
  public static TProtocol protocolFromString(
      TProtocolFactory protocolFactory, String data, String charset) throws TException {
    try {
      return protocolFromByteArray(protocolFactory, data.getBytes(charset));
    } catch (UnsupportedEncodingException uex) {
      throw new TException("JVM DOES NOT SUPPORT ENCODING: " + charset);
    }
  }

  /**
   * Get a protocol for deserializing a Thrift object from a Java string, using the default JVM
   * charset encoding.
   *
   * @param data The string to read from
   */
  public static TProtocol protocolFromString(TProtocolFactory protocolFactory, String data)
      throws TException {
    return protocolFromByteArray(protocolFactory, data.getBytes());
  }
}
