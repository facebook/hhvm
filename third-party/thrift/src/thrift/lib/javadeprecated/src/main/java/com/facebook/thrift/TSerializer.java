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

import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TIOStreamTransport;
import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;

/** Generic utility for easily serializing objects into a byte array or Java String. */
public class TSerializer {

  /** Internal protocol used for serializing objects. */
  private final TProtocolFactory protocolFactory;

  /** Create a new TSerializer that uses the TBinaryProtocol by default. */
  public TSerializer() {
    this(new TBinaryProtocol.Factory());
  }

  /**
   * Create a new TSerializer. It will use the TProtocol specified by the factory that is passed in.
   *
   * @param protocolFactory Factory to create a protocol
   */
  public TSerializer(TProtocolFactory protocolFactory) {
    this.protocolFactory = protocolFactory;
  }

  /**
   * Serialize the Thrift object into a byte array. The process is simple, just clear the byte array
   * output, write the object into it, and grab the raw bytes.
   *
   * @param base The object to serialize
   * @return Serialized object in byte[] format
   */
  public byte[] serialize(TBase base) throws TException {
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(byteArrayOutputStream);
    TProtocol protocol = protocolFactory.getProtocol(transport);

    base.write(protocol);

    return byteArrayOutputStream.toByteArray();
  }

  /**
   * Serialize the Thrift object into a Java string, using a specified character set for encoding.
   *
   * @param base The object to serialize
   * @param charset Valid JVM charset
   * @return Serialized object as a String
   */
  public String toString(TBase base, String charset) throws TException {
    try {
      return new String(serialize(base), charset);
    } catch (UnsupportedEncodingException uex) {
      throw new TException("JVM DOES NOT SUPPORT ENCODING: " + charset);
    }
  }

  /**
   * Serialize the Thrift object into a Java string, using the default JVM charset encoding.
   *
   * @param base The object to serialize
   * @return Serialized object as a String
   */
  public String toString(TBase base) throws TException {
    return new String(serialize(base));
  }
}
