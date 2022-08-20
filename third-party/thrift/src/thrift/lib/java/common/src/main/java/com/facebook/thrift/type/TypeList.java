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

import java.util.List;

/**
 * This interface defines a list which is mapping thrift uri to thrift generated classes.
 * Implementation of this interface is done dynamically by the thrift compiler using a mustache
 * template. Implementations use this syntax as the class name. __fbthrift_TypeList_<8 hex char>.
 * Each thrift module may have only one implementation. (If there is any thrift uri defined). To
 * prevent name conflict, sha256 hash code of the uri list is added as name suffix.
 *
 * <p>This list is pre-loaded into TypeRegistry without loading the mapped class through any
 * classloader.
 */
public interface TypeList {

  class TypeMapping {
    private String uri;
    private String className;

    public TypeMapping(String uri, String className) {
      this.uri = uri;
      this.className = className;
    }

    public String getUri() {
      return uri;
    }

    public String getClassName() {
      return className;
    }
  }

  List<TypeMapping> getTypes();
}
