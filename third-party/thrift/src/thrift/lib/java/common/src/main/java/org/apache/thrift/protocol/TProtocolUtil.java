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

package org.apache.thrift.protocol;

import com.facebook.thrift.protocol.ByteBufTBinaryProtocol;
import com.facebook.thrift.protocol.ByteBufTCompactProtocol;
import com.facebook.thrift.protocol.ByteBufTProtocol;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import io.netty.util.ReferenceCountUtil;
import java.nio.ByteBuffer;
import java.util.Collections;
import org.apache.thrift.TException;

/** Utility class with static methods for interacting with protocol data streams. */
public class TProtocolUtil {

  /** The maximum recursive depth the skip() function will traverse before throwing a TException. */
  private static int maxSkipDepth = Integer.MAX_VALUE;

  /**
   * Specifies the maximum recursive depth that the skip function will traverse before throwing a
   * TException. This is a global setting, so any call to skip in this JVM will enforce this value.
   *
   * @param depth the maximum recursive depth. A value of 2 would allow the skip function to skip a
   *     structure or collection with basic children, but it would not permit skipping a struct that
   *     had a field containing a child struct. A value of 1 would only allow skipping of simple
   *     types and empty structs/collections.
   */
  public static void setMaxSkipDepth(int depth) {
    maxSkipDepth = depth;
  }

  /**
   * Skips over the next data element from the provided input TProtocol object.
   *
   * @param prot the protocol object to read from
   * @param type the next value will be interpreted as this TType value.
   */
  public static void skip(TProtocol prot, byte type) throws TException {
    skip(prot, type, maxSkipDepth);
  }

  /**
   * Skips over the next data element from the provided input TProtocol object.
   *
   * @param prot the protocol object to read from
   * @param type the next value will be interpreted as this TType value.
   * @param maxDepth this function will only skip complex objects to this recursive depth, to
   *     prevent Java stack overflow.
   */
  public static void skip(TProtocol prot, byte type, int maxDepth) throws TException {
    if (maxDepth <= 0) {
      throw new TException("Maximum skip depth exceeded");
    }
    switch (type) {
      case TType.BOOL:
        prot.readBool();
        break;

      case TType.BYTE:
        prot.readByte();
        break;

      case TType.I16:
        prot.readI16();
        break;

      case TType.I32:
        prot.readI32();
        break;

      case TType.I64:
        prot.readI64();
        break;

      case TType.DOUBLE:
        prot.readDouble();
        break;

      case TType.STRING:
        prot.skipBinary();
        break;

      case TType.FLOAT:
        prot.readFloat();
        break;

      case TType.STRUCT:
        prot.readStructBegin(
            Collections.emptyMap(), Collections.emptyMap(), Collections.emptyMap());
        while (true) {
          TField field = prot.readFieldBegin();
          if (field.type == TType.STOP) {
            break;
          }
          skip(prot, field.type, maxDepth - 1);
          prot.readFieldEnd();
        }
        prot.readStructEnd();
        break;

      case TType.MAP:
        TMap map = prot.readMapBegin();
        for (int i = 0; (map.size < 0) ? prot.peekMap() : (i < map.size); i++) {
          skip(prot, map.keyType, maxDepth - 1);
          skip(prot, map.valueType, maxDepth - 1);
        }
        prot.readMapEnd();
        break;

      case TType.SET:
        TSet set = prot.readSetBegin();
        for (int i = 0; (set.size < 0) ? prot.peekSet() : (i < set.size); i++) {
          skip(prot, set.elemType, maxDepth - 1);
        }
        prot.readSetEnd();
        break;

      case TType.LIST:
        TList list = prot.readListBegin();
        for (int i = 0; (list.size < 0) ? prot.peekList() : (i < list.size); i++) {
          skip(prot, list.elemType, maxDepth - 1);
        }
        prot.readListEnd();
        break;

      default:
        break;
    }
  }

  public static ByteBuf readBinaryAsByteBuf(TProtocol protocol) {
    if (protocol instanceof ByteBufTBinaryProtocol) {
      return ((ByteBufTProtocol) protocol).readBinaryAsSlice();
    } else if (protocol instanceof ByteBufTCompactProtocol) {
      return ((ByteBufTProtocol) protocol).readBinaryAsSlice();
    }
    return Unpooled.wrappedBuffer(protocol.readBinary());
  }

  public static void writeBinary(TProtocol protocol, ByteBuffer buf) {
    protocol.writeBinary(buf);
  }

  public static void writeBinary(TProtocol protocol, ByteBuf buf) {
    try {
      if (protocol instanceof ByteBufTBinaryProtocol) {
        ((ByteBufTProtocol) protocol).writeBinaryAsByteBuf(buf);
      } else if (protocol instanceof ByteBufTCompactProtocol) {
        ((ByteBufTProtocol) protocol).writeBinaryAsByteBuf(buf);
      } else {
        protocol.writeBinary(ByteBuffer.wrap(ByteBufUtil.getBytes(buf)));
      }
    } finally {
      if (buf.refCnt() > 0) {
        ReferenceCountUtil.safeRelease(buf);
      }
    }
  }

  public static void writeBinary(TProtocol protocol, byte[] bytes) {
    protocol.writeBinary(ByteBuffer.wrap(bytes));
  }
}
