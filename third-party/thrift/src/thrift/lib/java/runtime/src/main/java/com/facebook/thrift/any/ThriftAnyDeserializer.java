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

import com.facebook.thrift.standard_type.TypeName;
import com.facebook.thrift.standard_type.TypeUri;
import com.facebook.thrift.type.Type;
import com.facebook.thrift.type.TypeRegistry;
import com.facebook.thrift.type.UniversalName;
import com.facebook.thrift.type_swift.ProtocolUnion;
import com.facebook.thrift.type_swift.TypeStruct;
import com.facebook.thrift.util.SerializationProtocolUtil;
import com.facebook.thrift.util.SerializerUtil;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.BiFunction;
import org.apache.thrift.protocol.TList;
import org.apache.thrift.protocol.TMap;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolUtil;
import org.apache.thrift.protocol.TSet;

/** Deserializer for thrift Any and SemiAny. */
public class ThriftAnyDeserializer {

  private static final Map<String, Type> uriCache = new ConcurrentHashMap<>();

  private TProtocol protocol;

  private ByteBuf byteBuf;
  private ProtocolUnion protocolUnion;
  private TypeStruct typeStruct;

  public ThriftAnyDeserializer(
      ByteBuf byteBuf, ProtocolUnion protocolUnion, TypeStruct typeStruct) {
    this.byteBuf = byteBuf;
    this.protocolUnion = protocolUnion;
    this.typeStruct = typeStruct;
  }

  private Object read(TypeStruct typeStruct) {
    TypeName name = typeStruct.getName();
    if (name.isSetBoolType()) {
      return protocol.readBool();
    } else if (name.isSetByteType()) {
      return protocol.readByte();
    } else if (name.isSetI16Type()) {
      return protocol.readI16();
    } else if (name.isSetI32Type()) {
      return protocol.readI32();
    } else if (name.isSetI64Type()) {
      return protocol.readI64();
    } else if (name.isSetFloatType()) {
      return protocol.readFloat();
    } else if (name.isSetDoubleType()) {
      return protocol.readDouble();
    } else if (name.isSetStringType()) {
      return protocol.readString();
    } else if (name.isSetBinaryType()) {
      return TProtocolUtil.readBinaryAsByteBuf(protocol);
    } else if (name.isSetEnumType()) {
      return readObject(typeStruct);
    } else if (name.isSetExceptionType()) {
      return readObject(typeStruct);
    } else if (name.isSetStructType()) {
      return readObject(typeStruct);
    } else if (name.isSetUnionType()) {
      return readObject(typeStruct);
    } else if (name.isSetListType()) {
      return deserializeObject(typeStruct);
    } else if (name.isSetMapType()) {
      return deserializeObject(typeStruct);
    } else {
      return deserializeObject(typeStruct);
    }
  };

  private Type findByType(String uri) {
    Type type = uriCache.get(uri);
    if (type != null) {
      return type;
    }
    type = TypeRegistry.findByUniversalName(new UniversalName(uri));
    if (type != null) {
      uriCache.put(uri, type);
    }
    return type;
  }

  private Type getType(TypeUri typeUri) {
    Type type;
    if (typeUri.isSetUri()) {
      type = TypeRegistry.findByUri(typeUri.getUri());
    } else {
      type = TypeRegistry.findByHashPrefix(typeUri.getTypeHashPrefixSha2256());
    }

    if (type == null) {
      throw new ObjectNotRegisteredException(
          "Unable to find type for any to deserialize object. "
              + "type: "
              + (typeUri.isSetUri() ? typeUri.getUri() : "null")
              + " hash: "
              + (typeUri.isSetTypeHashPrefixSha2256()
                  ? ByteBufUtil.hexDump(typeUri.getTypeHashPrefixSha2256())
                  : "null"));
    }

    return type;
  }

  private Object readObject(TypeStruct typeStruct) {
    Type type = null;
    TypeName name = typeStruct.getName();
    if (name.isSetStructType()) {
      type = getType(name.getStructType());
    } else if (name.isSetExceptionType()) {
      type = getType(name.getExceptionType());
    } else if (name.isSetUnionType()) {
      type = getType(name.getUnionType());
    } else if (name.isSetEnumType()) {
      type = getType(name.getEnumType());
    }

    return type.getReader().read(protocol);
  }

  private List deserializeList(TypeStruct typeStruct) {
    TList tList = protocol.readListBegin();
    List list = new ArrayList(Math.max(0, tList.size));
    int i = 0;

    while (true) {
      if (tList.size < 0) {
        if (!protocol.peekList()) {
          break;
        }
      } else if (i >= tList.size) {
        break;
      }

      list.add(read(typeStruct.getParams().get(0)));
      i++;
    }
    protocol.readListEnd();
    return list;
  }

  private Map deserializeMap(TypeStruct typeStruct) {
    TMap tMap = protocol.readMapBegin();
    Map map = new HashMap(Math.max(0, tMap.size));
    int i = 0;

    while (true) {
      if (tMap.size < 0) {
        if (!protocol.peekMap()) {
          break;
        }
      } else if (i >= tMap.size) {
        break;
      }

      map.put(read(typeStruct.getParams().get(0)), read(typeStruct.getParams().get(1)));
      i++;
    }
    protocol.readMapEnd();
    return map;
  }

  private Set deserializeSet(TypeStruct typeStruct) {
    TSet tSet = protocol.readSetBegin();
    Set set = new HashSet(Math.max(0, tSet.size));
    int i = 0;

    while (true) {
      if (tSet.size < 0) {
        if (!protocol.peekSet()) {
          break;
        }
      } else if (i >= tSet.size) {
        break;
      }

      set.add(read(typeStruct.getParams().get(0)));
      i++;
    }
    protocol.readSetEnd();
    return set;
  }

  private Object deserializeObject(TypeStruct typeStruct) {
    if (typeStruct == null) {
      return null;
    }

    if (typeStruct.getName().isSetListType()) {
      return deserializeList(typeStruct);
    } else if (typeStruct.getName().isSetMapType()) {
      return deserializeMap(typeStruct);
    } else if (typeStruct.getName().isSetSetType()) {
      return deserializeSet(typeStruct);
    }

    return read(typeStruct);
  }

  private Object deserializeObject(TypeStruct typeStruct, ByteBuf data, String customProtocolUri) {
    BiFunction<TypeStruct, ByteBuf, Object> func =
        AbstractAny.deserializerUri.get(customProtocolUri);
    if (func == null) {
      throw new IllegalArgumentException(
          "Custom protocol deserializer is not registered, uri: " + customProtocolUri);
    }
    return func.apply(typeStruct, data);
  }

  private Object deserializeObject(TypeStruct typeStruct, ByteBuf data, long customProtocolId) {
    BiFunction<TypeStruct, ByteBuf, Object> func = AbstractAny.deserializerId.get(customProtocolId);
    if (func == null) {
      throw new IllegalArgumentException(
          "Custom protocol deserializer is not registered, id: " + customProtocolId);
    }
    return func.apply(typeStruct, data);
  }

  public Object deserialize() {
    if (protocolUnion.isSetStandard()) {
      this.protocol =
          SerializerUtil.toByteBufProtocol(
              SerializationProtocolUtil.getProtocol(protocolUnion.getStandard()), byteBuf);
      return deserializeObject(typeStruct);
    }
    if (protocolUnion.isSetCustom()) {
      return deserializeObject(typeStruct, byteBuf, protocolUnion.getCustom());
    }
    return deserializeObject(typeStruct, byteBuf, protocolUnion.getId());
  }
}
