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

package com.facebook.thrift.any;

import com.facebook.swift.codec.ThriftUnion;
import com.facebook.thrift.enums.BaseEnum;
import com.facebook.thrift.payload.ThriftSerializable;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.standard_type.TypeUri;
import com.facebook.thrift.standard_type.Void;
import com.facebook.thrift.type.Type;
import com.facebook.thrift.type.TypeRegistry;
import com.facebook.thrift.type.UniversalName;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import com.facebook.thrift.util.EnumUtil;
import com.facebook.thrift.util.SerializationProtocolUtil;
import com.facebook.thrift.util.SerializerUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import java.lang.annotation.Annotation;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;
import org.apache.thrift.TBaseException;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolUtil;
import org.apache.thrift.protocol.TSet;
import org.apache.thrift.protocol.TType;

/**
 * Serializer for thrift Any and SemiAny. Serialize given object with the protocol provided. Object
 * type is generated from object.
 */
public class ThriftAnySerializer {
  private TProtocol protocol;
  private TypeStruct typeStruct;

  private AnyBuilder builder;

  private ByteBuf buffer;

  public ThriftAnySerializer(AnyBuilder builder) {
    this.builder = builder;
  }

  private boolean isUnion(Class clazz) {
    try {
      Annotation a = clazz.getAnnotation(ThriftUnion.class);
      return a != null;
    } catch (Throwable t) {
      // ignore
    }

    return false;
  }

  private boolean isUnion(Object o) {
    return isUnion(o.getClass());
  }

  private TypeUri getTypeUri(Class clazz) {
    Type type = TypeRegistry.findByClass(clazz);
    if (type == null) {
      throw new ObjectNotRegisteredException(
          "Unable to find type for any to serialize object: " + clazz.getName());
    }

    UniversalName un = type.getUniversalName();

    if (builder.useUri) {
      return TypeUri.fromUri(un.getUri());
    }
    if (builder.useHashPrefix) {
      return TypeUri.fromTypeHashPrefixSha2256(un.getHashPrefixBytes(builder.numberOfBytes));
    }
    if (un.preferHash()) {
      return TypeUri.fromTypeHashPrefixSha2256(un.getHashPrefixBytes(builder.numberOfBytes));
    }
    return TypeUri.fromUri(un.getUri());
  }

  private TypeUri getTypeUri(Object o) {
    return getTypeUri(o.getClass());
  }

  private TypeName getTypeName(Class clazz) {
    if (Boolean.class.isAssignableFrom(clazz)) {
      return TypeName.fromBoolType(Void.UNUSED);
    }
    if (Byte.class.isAssignableFrom(clazz)) {
      return TypeName.fromByteType(Void.UNUSED);
    }
    if (Short.class.isAssignableFrom(clazz)) {
      return TypeName.fromI16Type(Void.UNUSED);
    }
    if (Integer.class.isAssignableFrom(clazz)) {
      return TypeName.fromI32Type(Void.UNUSED);
    }
    if (Long.class.isAssignableFrom(clazz)) {
      return TypeName.fromI64Type(Void.UNUSED);
    }
    if (Float.class.isAssignableFrom(clazz)) {
      return TypeName.fromFloatType(Void.UNUSED);
    }
    if (Double.class.isAssignableFrom(clazz)) {
      return TypeName.fromDoubleType(Void.UNUSED);
    }
    if (String.class.isAssignableFrom(clazz)) {
      return TypeName.fromStringType(Void.UNUSED);
    }
    if (byte[].class.isAssignableFrom(clazz)) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (ByteBuf.class.isAssignableFrom(clazz)) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (ByteBuffer.class.isAssignableFrom(clazz)) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (BaseEnum.class.isAssignableFrom(clazz)) {
      return TypeName.fromEnumType(getTypeUri(clazz));
    }
    if (TBaseException.class.isAssignableFrom(clazz)) {
      return TypeName.fromExceptionType(getTypeUri(clazz));
    }
    if (ThriftSerializable.class.isAssignableFrom(clazz) && isUnion(clazz)) {
      return TypeName.fromUnionType(getTypeUri(clazz));
    }
    if (ThriftSerializable.class.isAssignableFrom(clazz)) {
      return TypeName.fromStructType(getTypeUri(clazz));
    }
    if (List.class.isAssignableFrom(clazz)) {
      return TypeName.fromListType(Void.UNUSED);
    }
    if (Map.class.isAssignableFrom(clazz)) {
      return TypeName.fromMapType(Void.UNUSED);
    }
    if (Set.class.isAssignableFrom(clazz)) {
      return TypeName.fromSetType(Void.UNUSED);
    }

    throw new IllegalArgumentException(getMsg(clazz));
  }

  private TypeName getTypeName(Object o) {
    if (o instanceof Boolean) {
      return TypeName.fromBoolType(Void.UNUSED);
    }
    if (o instanceof Byte) {
      return TypeName.fromByteType(Void.UNUSED);
    }
    if (o instanceof Short) {
      return TypeName.fromI16Type(Void.UNUSED);
    }
    if (o instanceof Integer) {
      return TypeName.fromI32Type(Void.UNUSED);
    }
    if (o instanceof Long) {
      return TypeName.fromI64Type(Void.UNUSED);
    }
    if (o instanceof Float) {
      return TypeName.fromFloatType(Void.UNUSED);
    }
    if (o instanceof Double) {
      return TypeName.fromDoubleType(Void.UNUSED);
    }
    if (o instanceof String) {
      return TypeName.fromStringType(Void.UNUSED);
    }
    if (o instanceof byte[]) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (o instanceof ByteBuf) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (o instanceof ByteBuffer) {
      return TypeName.fromBinaryType(Void.UNUSED);
    }
    if (o instanceof BaseEnum) {
      return TypeName.fromEnumType(getTypeUri(o));
    }
    if (o instanceof TBaseException) {
      return TypeName.fromExceptionType(getTypeUri(o));
    }
    if (o instanceof ThriftSerializable && isUnion(o)) {
      return TypeName.fromUnionType(getTypeUri(o));
    }
    if (o instanceof ThriftSerializable) {
      return TypeName.fromStructType(getTypeUri(o));
    }

    throw new IllegalArgumentException(getMsg(o));
  }

  private void writeObject(Object o) {
    if (o instanceof ThriftSerializable) {
      ((ThriftSerializable) o).write0(protocol);
    } else if (o instanceof Writer) {
      ((Writer) o).write(protocol);
    } else {
      throwError(o);
    }
  }

  private String getMsg(Class clazz) {
    return "Unsupported type for thrift Any: " + clazz.getName();
  }

  private String getMsg(Object o) {
    return getMsg(o.getClass());
  }

  private void throwError(Object o) {
    throw new IllegalArgumentException(getMsg(o));
  }

  private int typeInx;

  private boolean isContainer(Object o) {
    return o instanceof List || o instanceof Map || o instanceof Set;
  }

  private Class getNextClass() {
    if (typeInx >= builder.elementTypeList.size()) {
      throw new IllegalArgumentException("Type class is not provided for nested value");
    }
    return (Class) builder.elementTypeList.get(typeInx++);
  }

  private TypeStruct getTypeStruct(Class clazz) {
    TypeStruct.Builder st = new TypeStruct.Builder().setName(getTypeName(clazz));

    if (typeInx < builder.elementTypeList.size()) {
      List<TypeStruct> paramList = new ArrayList<>();
      st.setParams(paramList);

      if (Map.class.isAssignableFrom(clazz)) {
        paramList.add(getTypeStruct(getNextClass()));
        paramList.add(getTypeStruct(getNextClass()));
      } else if (List.class.isAssignableFrom(clazz)) {
        paramList.add(getTypeStruct(getNextClass()));
      } else if (Set.class.isAssignableFrom(clazz)) {
        paramList.add(getTypeStruct(getNextClass()));
      }
    }

    return st.build();
  }

  private void write(Object o, int depth) {
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
      writeObject(o);
    } else if (o instanceof ThriftSerializable && isUnion(o)) {
      writeObject(o);
    } else if (o instanceof ThriftSerializable) {
      writeObject(o);
    } else if (o instanceof List) {
      serializeObject(o, depth);
    } else if (o instanceof Map) {
      serializeObject(o, depth);
    } else if (o instanceof Set) {
      serializeObject(o, depth);
    } else {
      throwError(o);
    }
  }

  private byte getType(Class clazz) {
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
    if (ThriftSerializable.class.isAssignableFrom(clazz) && isUnion(clazz)) {
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

    throw new IllegalArgumentException(getMsg(clazz));
  }

  private byte getType(int depth) {
    return getType((Class) builder.elementTypeList.get(depth));
  }

  private void serializeObject(Object o, int depth) {
    if (o instanceof List) {
      List<?> list = (List) o;
      protocol.writeListBegin(new TList(getType(depth), list.size()));
      for (Object item : list) {
        write(item, depth + 1);
      }
      protocol.writeListEnd();
    } else if (o instanceof Map) {
      Map map = (Map) o;
      protocol.writeMapBegin(new TMap(getType(depth), getType(depth + 1), map.size()));
      for (Object key : map.keySet()) {
        write(key, depth + 2);
        write(map.get(key), depth + 2);
      }
      protocol.writeMapEnd();
    } else if (o instanceof Set) {
      Set set = (Set) o;
      protocol.writeSetBegin(new TSet(getType(depth), set.size()));
      for (Object item : set) {
        write(item, depth + 1);
      }
      protocol.writeSetEnd();
    } else {
      write(o, depth + 1);
    }
  }

  public ProtocolUnion getProtocolUnion() {
    if (builder.standardProtocol != null) {
      return ProtocolUnion.fromStandard(builder.standardProtocol);
    }
    if (builder.customProtocolUri != null) {
      return ProtocolUnion.fromCustom(builder.customProtocolUri);
    }
    if (builder.customProtocolId != null) {
      return ProtocolUnion.fromId(builder.customProtocolId);
    }

    return null;
    //    throw new IllegalArgumentException(
    //        "StandardProtocol, custom protocol uri or id must be provided");
  }

  private void serializeObject(Object o, String customProtocolUri) {
    Function<Object, ByteBuf> func = AbstractAny.serializerUri.get(customProtocolUri);
    if (func == null) {
      throw new IllegalArgumentException(
          "Custom protocol serializer is not registered, uri: " + customProtocolUri);
    }
    this.buffer = func.apply(o);
  }

  private void serializeObject(Object o, long customProtocolId) {
    Function<Object, ByteBuf> func = AbstractAny.serializerId.get(customProtocolId);
    if (func == null) {
      throw new IllegalArgumentException(
          "Custom protocol serializer is not registered, id: " + customProtocolId);
    }
    this.buffer = func.apply(o);
  }

  public void serialize(Object o) {
    if (o != null) {
      if (builder.typeStruct == null) {
        if (isContainer(o)) {
          typeStruct = getTypeStruct(o.getClass());
        } else {
          typeStruct = new TypeStruct.Builder().setName(getTypeName(o)).build();
        }
      } else {
        typeStruct = builder.typeStruct;
      }

      if (builder.standardProtocol != null) {
        // buffer = RpcResources.getUnpooledByteBufAllocator().buffer(1024, 1 << 24);
        buffer = RpcResources.getByteBufAllocator().buffer(1024, 1 << 24);
        protocol =
            SerializerUtil.toByteBufProtocol(
                SerializationProtocolUtil.getProtocol(builder.standardProtocol), buffer);
        serializeObject(o, 0);
      } else if (builder.customProtocolUri != null) {
        serializeObject(o, builder.customProtocolUri);
      } else if (builder.customProtocolId != null) {
        serializeObject(o, builder.customProtocolId);
      }
    }
  }

  public TypeStruct getTypeStruct() {
    return this.typeStruct;
  }

  public ByteBuf getBuffer() {
    return this.buffer;
  }
}
