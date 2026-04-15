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

import com.facebook.thrift.enums.BaseEnum;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.payload.Writer;
import io.netty.buffer.ByteBuf;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.apache.thrift.TBaseException;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolUtil;
import org.apache.thrift.protocol.TSet;
import org.apache.thrift.protocol.TStruct;
import org.apache.thrift.protocol.TType;

public final class Writers {
  private static final TStruct EMPTY_STRUCT = new TStruct();

  private Writers() {}

  public static Writer emptyStruct() {
    return protocol -> {
      protocol.writeStructBegin(EMPTY_STRUCT);
      protocol.writeFieldStop();
      protocol.writeStructEnd();
    };
  }

  public static Writer binaryWriter(byte[] b) {
    return protocol -> protocol.writeBinary(ByteBuffer.wrap(b));
  }

  public static Writer binaryWriter(ByteBuf b) {
    return protocol -> protocol.writeBinary(b.nioBuffer());
  }

  public static Writer binaryWriter(ByteBuffer b) {
    return protocol -> protocol.writeBinary(b);
  }

  public static Writer booleanWriter(boolean b) {
    return protocol -> protocol.writeBool(b);
  }

  public static Writer stringWriter(String s) {
    return protocol -> protocol.writeString(s);
  }

  public static Writer byteWriter(byte b) {
    return protocol -> protocol.writeByte(b);
  }

  public static Writer i16Writer(short s) {
    return protocol -> protocol.writeI16(s);
  }

  public static Writer i32Writer(int i) {
    return protocol -> protocol.writeI32(i);
  }

  public static Writer i64Writer(long l) {
    return protocol -> protocol.writeI64(l);
  }

  public static Writer floatWriter(float f) {
    return protocol -> protocol.writeFloat(f);
  }

  public static Writer doubleWriter(double d) {
    return protocol -> protocol.writeDouble(d);
  }

  public static <T> Writer listWriter(List<T> list, Class<T> type) {
    return protocol -> serializeObject(protocol, list, type, null);
  }

  public static <K, V> Writer mapWriter(Map<K, V> list, Class<K> type, Class<V> valueType) {
    return protocol -> serializeObject(protocol, list, type, valueType);
  }

  public static <T> Writer setWriter(Set<T> list, Class<T> type) {
    return protocol -> serializeObject(protocol, list, type, null);
  }

  private static <T, E> void serializeObject(
      TProtocol protocol, Object o, Class<T> clazz, Class<E> secondaryClazz) {
    if (o instanceof List) {
      List<?> list = (List) o;
      protocol.writeListBegin(new TList(getType(clazz), list.size()));
      for (Object item : list) {
        write(protocol, item);
      }
      protocol.writeListEnd();
    } else if (o instanceof Map) {
      Map map = (Map) o;
      protocol.writeMapBegin(new TMap(getType(clazz), getType(secondaryClazz), map.size()));
      for (Object key : map.keySet()) {
        write(protocol, key);
        write(protocol, map.get(key));
      }
      protocol.writeMapEnd();
    } else if (o instanceof Set) {
      Set set = (Set) o;
      protocol.writeSetBegin(new TSet(getType(clazz), set.size()));
      for (Object item : set) {
        write(protocol, item);
      }
      protocol.writeSetEnd();
    } else {
      throwError(o);
    }
  }

  private static void writeObject(TProtocol protocol, Object o) {
    if (o instanceof ThriftSerializable) {
      ((ThriftSerializable) o).write0(protocol);
    } else if (o instanceof Writer) {
      ((Writer) o).write(protocol);
    } else {
      throwError(o);
    }
  }

  private static void throwError(Object o) {
    throw new IllegalArgumentException(
        "Unsupported type for thrift type: " + (o == null ? "null" : o.getClass().getName()));
  }

  private static void write(TProtocol protocol, Object o) {
    if (o instanceof Boolean) {
      protocol.writeBool((Boolean) o);
    } else if (o instanceof Byte) {
      protocol.writeByte((Byte) o);
    } else if (o instanceof Short) {
      protocol.writeI16((Short) o);
    } else if (o instanceof Integer) {
      protocol.writeI32((Integer) o);
    } else if (o instanceof Long) {
      protocol.writeI64((Long) o);
    } else if (o instanceof Float) {
      protocol.writeFloat((Float) o);
    } else if (o instanceof Double) {
      protocol.writeDouble((Double) o);
    } else if (o instanceof String) {
      protocol.writeString((String) o);
    } else if (o instanceof byte[]) {
      TProtocolUtil.writeBinary(protocol, (byte[]) o);
    } else if (o instanceof ByteBuf) {
      TProtocolUtil.writeBinary(protocol, (ByteBuf) o);
    } else if (o instanceof ByteBuffer) {
      TProtocolUtil.writeBinary(protocol, (ByteBuffer) o);
    } else if (o instanceof BaseEnum) {
      protocol.writeI32(EnumUtil.getValue((BaseEnum) o));
    } else if (o instanceof TBaseException) {
      writeObject(protocol, o);
    } else if (o instanceof ThriftSerializable) {
      writeObject(protocol, o);
    } else {
      // Recursive object types List, Map, Set not currently supported due to lack of type
      // information
      throwError(o);
    }
  }

  private static byte getType(Class clazz) {
    if (Boolean.class.isAssignableFrom(clazz)) {
      return TType.BOOL;
    }
    if (Byte.class.isAssignableFrom(clazz)) {
      return TType.BYTE;
    }
    if (Short.class.isAssignableFrom(clazz)) {
      return TType.I16;
    }
    if (Integer.class.isAssignableFrom(clazz)) {
      return TType.I32;
    }
    if (Long.class.isAssignableFrom(clazz)) {
      return TType.I64;
    }
    if (Float.class.isAssignableFrom(clazz)) {
      return TType.FLOAT;
    }
    if (Double.class.isAssignableFrom(clazz)) {
      return TType.DOUBLE;
    }
    if (String.class.isAssignableFrom(clazz)) {
      return TType.STRING;
    }
    if (byte[].class.isAssignableFrom(clazz)) {
      return TType.STRING;
    }
    if (ByteBuf.class.isAssignableFrom(clazz)) {
      return TType.STRING;
    }
    if (ByteBuffer.class.isAssignableFrom(clazz)) {
      return TType.STRING;
    }
    if (BaseEnum.class.isAssignableFrom(clazz)) {
      return TType.ENUM;
    }
    if (TBaseException.class.isAssignableFrom(clazz)) {
      return TType.STRUCT;
    }
    if (ThriftSerializable.class.isAssignableFrom(clazz)) {
      return TType.STRUCT;
    }
    if (List.class.isAssignableFrom(clazz)) {
      return TType.LIST;
    }
    if (Map.class.isAssignableFrom(clazz)) {
      return TType.MAP;
    }
    if (Set.class.isAssignableFrom(clazz)) {
      return TType.SET;
    }

    throw new IllegalArgumentException("Unsupported type for thrift type: " + clazz.getName());
  }
}
