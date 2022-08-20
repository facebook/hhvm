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

package com.facebook.thrift.meta_data;

import com.facebook.thrift.TBase;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This class is used to store meta data about thrift fields. Every field in a a struct should have
 * a corresponding instance of this class describing it.
 */
@SuppressWarnings("serial")
public class FieldMetaData implements java.io.Serializable {
  public final String fieldName;
  public final byte requirementType;
  public final FieldValueMetaData valueMetaData;
  private static Map<Class<? extends TBase>, Map<Integer, FieldMetaData>> structMap;

  static {
    structMap = new ConcurrentHashMap<Class<? extends TBase>, Map<Integer, FieldMetaData>>();
  }

  public FieldMetaData(String name, byte req, FieldValueMetaData vMetaData) {
    this.fieldName = name;
    this.requirementType = req;
    this.valueMetaData = vMetaData;
  }

  public static void addStructMetaDataMap(
      Class<? extends TBase> sClass, Map<Integer, FieldMetaData> map) {
    structMap.put(sClass, map);
  }

  /**
   * Returns a map with metadata (i.e. instances of FieldMetaData) that describe the fields of the
   * given class.
   *
   * @param sClass The TBase class for which the metadata map is requested
   */
  public static Map<Integer, FieldMetaData> getStructMetaDataMap(Class<? extends TBase> sClass) {
    if (!structMap.containsKey(sClass)) { // Load class if it hasn't been loaded
      try {
        sClass.newInstance();
      } catch (InstantiationException e) {
        throw new RuntimeException(
            "InstantiationException for TBase class: "
                + sClass.getName()
                + ", message: "
                + e.getMessage());
      } catch (IllegalAccessException e) {
        throw new RuntimeException(
            "IllegalAccessException for TBase class: "
                + sClass.getName()
                + ", message: "
                + e.getMessage());
      }
    }
    return structMap.get(sClass);
  }
}
