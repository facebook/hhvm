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

package com.facebook.thrift.util;

import com.facebook.thrift.payload.Reader;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TSet;
import reactor.core.Exceptions;

@SuppressWarnings("unused")
public final class Readers {
  private Readers() {
    throw new UnsupportedOperationException("Instantiate a utility class");
  }

  public static final Reader<Void> VOID_READER = iprot -> null;

  private static final Reader<byte[]> BINARY_READER =
      iprot -> {
        try {
          ByteBuffer buf = iprot.readBinary();
          if (buf.hasArray()) {
            int offset = buf.arrayOffset();
            int remaining = buf.remaining();

            // NOTE: in most TProtocol implementations, getBinary() is simply implemented as
            // ByteBuf.wrap()
            if (offset == 0 && remaining == buf.capacity()) {
              return buf.array();
            } else {
              byte[] copy = new byte[remaining];
              buf.get(copy, offset, copy.length);
              return copy;
            }
          }

          byte[] copy = new byte[buf.remaining()];
          buf.get(copy, 0, copy.length);
          return copy;

        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<String> STRING_READER =
      iprot -> {
        try {
          return iprot.readString();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Boolean> BOOLEAN_READER =
      iprot -> {
        try {
          return iprot.readBool();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Byte> BYTE_READER =
      iprot -> {
        try {
          return iprot.readByte();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Short> I16_READER =
      iprot -> {
        try {
          return iprot.readI16();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Integer> I32_READER =
      iprot -> {
        try {
          return iprot.readI32();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Long> I64_READER =
      iprot -> {
        try {
          return iprot.readI64();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Float> FLOAT_READER =
      iprot -> {
        try {
          return iprot.readFloat();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  private static final Reader<Double> DOUBLE_READER =
      iprot -> {
        try {
          return iprot.readDouble();
        } catch (Throwable e) {
          throw Exceptions.propagate(e);
        }
      };

  public static Reader<Void> voidReader() {
    return VOID_READER;
  }

  public static Reader<byte[]> binaryReader() {
    return BINARY_READER;
  }

  public static Reader<String> stringReader() {
    return STRING_READER;
  }

  public static Reader<Boolean> booleanReader() {
    return BOOLEAN_READER;
  }

  public static Reader<Byte> byteReader() {
    return BYTE_READER;
  }

  public static Reader<Short> i16Reader() {
    return I16_READER;
  }

  public static Reader<Integer> i32Reader() {
    return I32_READER;
  }

  public static Reader<Long> i64Reader() {
    return I64_READER;
  }

  public static Reader<Float> floatReader() {
    return FLOAT_READER;
  }

  public static Reader<Double> doubleReader() {
    return DOUBLE_READER;
  }

  public static <T> Reader<T> wrap(Reader<T> reader) {
    return iprot -> {
      try {
        return reader.read(iprot);
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }

  public static <T> Reader<List<T>> listReader(Reader<T> listElementReader) {
    return iprot -> {
      try {
        TList list = iprot.readListBegin();
        List<T> result = new ArrayList<>(Math.max(0, list.size));
        for (int i = 0; (list.size < 0) ? iprot.peekList() : (i < list.size); i++) {
          T elem = listElementReader.read(iprot);
          result.add(elem);
        }
        iprot.readListEnd();
        return result;
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }

  public static <K, V> Reader<Map<K, V>> mapReader(Reader<K> keyReader, Reader<V> valueReader) {
    return iprot -> {
      try {
        TMap map = iprot.readMapBegin();
        Map<K, V> result = new HashMap<>(Math.max(0, map.size));
        for (int i = 0; (map.size < 0) ? iprot.peekMap() : (i < map.size); i++) {
          K _key1 = keyReader.read(iprot);
          V _value1 = valueReader.read(iprot);
          result.put(_key1, _value1);
        }
        iprot.readMapEnd();
        return result;
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }

  public static <T> Reader<Set<T>> setReader(Reader<T> setElementReader) {
    return iprot -> {
      try {
        TSet set = iprot.readSetBegin();
        Set<T> result = new HashSet<>(Math.max(0, set.size));
        for (int i = 0; (set.size < 0) ? iprot.peekSet() : (i < set.size); i++) {
          T elem = setElementReader.read(iprot);
          result.add(elem);
        }
        iprot.readSetEnd();
        return result;
      } catch (Throwable e) {
        throw Exceptions.propagate(e);
      }
    };
  }
}
