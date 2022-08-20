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

package com.facebook.thrift.type;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.reflect.ClassPath;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public final class TypeRegistry {
  private static final Logger LOGGER = LoggerFactory.getLogger(TypeRegistry.class);

  /** Sorted map for universal name hash to Type */
  private static final ConcurrentSkipListMap<ByteBuf, Type> hashMap = new ConcurrentSkipListMap<>();

  /** Local cache for sorted map. */
  private static final Map<ByteBuf, Type> cache = new ConcurrentHashMap<>();

  /** Hash map for class name to Type */
  private static final Map<Class, Type> classMap = new ConcurrentHashMap<>();

  /** Sorted map for pre registered universal name hash to class name */
  private static final ConcurrentSkipListMap<ByteBuf, String> hashList =
      new ConcurrentSkipListMap<>();

  private static final String TYPE_LIST_PREFIX = "__fbthrift_TypeList_";

  private TypeRegistry() {}

  static {
    initialize();
  }

  /**
   * Check if the prefix is already in the pre-loaded hash list. If it is in the list, mapped
   * classname is loaded. This will register the type to the TypeRegistry.
   *
   * @param prefix prefix as byte array
   * @return True if prefix is already in pre-loaded hash list, false if otherwise.
   * @throws AmbiguousUniversalNameException if the prefix return more than one entry.
   */
  private static boolean registered(ByteBuf prefix) {
    ByteBuf key = hashList.ceilingKey(prefix);
    if (key == null || !startsWith(key, prefix)) {
      return false;
    }

    ByteBuf nextKey = hashList.higherKey(key);
    if (nextKey == null || !startsWith(nextKey, prefix)) {
      String className = hashList.get(key);
      if (className != null) {
        try {
          Class.forName(className);
        } catch (Exception e) {
          throw new RuntimeException(e);
        }
      }
      return true;
    }

    throw new AmbiguousUniversalNameException(ByteBufUtil.hexDump(prefix));
  }

  /**
   * Find all _TypeList_<hash-code> classes from classpath and pre-load all type mapping into
   * hashList - TreeMap
   */
  private static void initialize() {
    try {
      ClassPath.from(ClassLoader.getSystemClassLoader()).getAllClasses().stream()
          .filter(c -> c.getName().contains(TYPE_LIST_PREFIX))
          .forEach(
              c -> {
                try {
                  Class clazz = Class.forName(c.getName());
                  Object o = clazz.getDeclaredConstructor().newInstance();
                  if (o instanceof TypeList) {
                    TypeList typeList = (TypeList) o;
                    for (TypeList.TypeMapping m : typeList.getTypes()) {
                      UniversalName un = new UniversalName(m.getUri());
                      hashList.put(un.getHashBytes(), m.getClassName());
                    }
                  }
                } catch (Exception e) {
                  LOGGER.warn("Can not load class: " + c.getName(), e);
                }
              });
    } catch (Exception e) {
      throw new RuntimeException("Can not read classes", e);
    }
  }

  /**
   * Validate given universal name and the class. They should not be associated with another object.
   * If universal name or class is already associated with another entry, IllegalArgumentException
   * is thrown.
   *
   * @param type
   */
  private static void validate(Type type) {
    Class clazz = type.getClazz();
    UniversalName universalName = type.getUniversalName();

    if (exist(universalName) && hashMap.get(universalName.getHashBytes()).getClazz() != clazz) {
      String msg =
          "Universal name already registered with another class "
              + hashMap.get(universalName.getHashBytes()).getClazz().getCanonicalName();
      LOGGER.error(msg);
      throw new IllegalArgumentException(msg);
    }
    if (exist(clazz)
        && !classMap
            .get(clazz)
            .getUniversalName()
            .getHashBytes()
            .equals(universalName.getHashBytes())) {
      String msg =
          "Class name already registered with another universal name "
              + classMap.get(clazz).getUniversalName().getUri();
      LOGGER.error(msg);
      throw new IllegalArgumentException(msg);
    }
  }

  /**
   * Validate and add the type.
   *
   * @param type Type info containing universal name, class name and other metadata.
   */
  private static synchronized void addType(Type type) {
    validate(type);

    hashMap.put(type.getUniversalName().getHashBytes(), type);
    classMap.put(type.getClazz(), type);

    cache.clear();
  }

  /**
   * Add the given type to the registry. If universal name or class is already associated with
   * another entry, IllegalArgumentException is thrown.
   *
   * @param type Type info containing universal name, class name and other metadata.
   * @return Hash bytes as hexadecimal value.
   */
  public static String add(Type type) {
    LOGGER.debug("Adding type to the registry: {}", type);
    addType(type);

    return type.getUniversalName().getHash();
  }

  private static boolean startsWith(ByteBuf bytes, ByteBuf prefix) {
    for (int i = 0; i < Math.min(prefix.capacity(), bytes.capacity()); i++) {
      if (prefix.getByte(i) != bytes.getByte(i)) {
        return false;
      }
    }
    return prefix.capacity() <= bytes.capacity();
  }

  /**
   * Find Type by given hash prefix. For example, if a Type is registered with a hash
   * "af420267bd92e0cb", it can be returned by providing any hash prefix, like "af42", "af4202" etc
   * if they return exactly one Type.
   *
   * @param prefix prefix as byte array
   * @return Type
   * @throws AmbiguousUniversalNameException if the prefix return more than one Type.
   * @throws IllegalArgumentException if the prefix is empty or null
   */
  public static Type findByHashPrefix(ByteBuf prefix) {
    if (!Objects.requireNonNull(prefix).isReadable()) {
      throw new IllegalArgumentException("Prefix must be at least one byte, it is empty or null");
    }

    if (cache.containsKey(prefix)) {
      return cache.get(prefix);
    }

    ByteBuf key = hashMap.ceilingKey(prefix);
    if (key == null || !startsWith(key, prefix)) {
      if (registered(prefix)) {
        return findByHashPrefix(prefix);
      }
      return null;
    }

    ByteBuf nextKey = hashMap.higherKey(key);
    if (nextKey == null || !startsWith(nextKey, prefix)) {
      Type type = hashMap.get(key);
      cache.put(prefix, type);
      return type;
    }

    throw new AmbiguousUniversalNameException(ByteBufUtil.hexDump(prefix));
  }

  /**
   * Find Type by given hash prefix. Same as the {@link #findByHashPrefix(ByteBuf) findByHashPrefix}
   * but taking prefix as hexadecimal String.
   *
   * @param hexPrefix as hexadecimal string
   * @return Type
   * @throws AmbiguousUniversalNameException if the prefix return more than one Type.
   * @throws IllegalArgumentException if the prefix is empty or null
   */
  public static Type findByHashPrefix(String hexPrefix) {
    return findByHashPrefix(Unpooled.wrappedBuffer(ByteBufUtil.decodeHexDump(hexPrefix)));
  }

  /**
   * Find Type by given class name.
   *
   * @param clazz
   * @return Type
   */
  public static Type findByClass(Class clazz) {
    return classMap.get(clazz);
  }

  /**
   * Find Type by given universal name.
   *
   * @param universalName Universal name
   * @return Type
   */
  public static Type findByUniversalName(UniversalName universalName) {
    return hashMap.get(universalName.getHashBytes());
  }

  /**
   * Check if the class is already in the registry.
   *
   * @param clazz Name of the class
   * @return True if already in the registry, false otherwise
   */
  public static boolean exist(Class clazz) {
    return classMap.containsKey(clazz);
  }

  /**
   * Check if the universal name is already in the registry.
   *
   * @param universalName Universal name
   * @return True if already in the registry, false otherwise
   */
  public static boolean exist(UniversalName universalName) {
    return hashMap.containsKey(universalName.getHashBytes());
  }

  @VisibleForTesting
  public static int size() {
    return hashMap.size();
  }

  @VisibleForTesting
  public static void clear() {
    LOGGER.info("Clearing type registry");
    hashMap.clear();
    classMap.clear();
  }

  @VisibleForTesting
  public static void print() {
    for (ByteBuf key : hashMap.keySet()) {
      System.out.println(
          ByteBufUtil.hexDump(key) + " " + hashMap.get(key).getUniversalName().getUri());
    }
  }
}
