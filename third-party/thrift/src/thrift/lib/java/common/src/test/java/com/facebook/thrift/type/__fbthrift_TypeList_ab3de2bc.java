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

import java.util.ArrayList;
import java.util.List;

public class __fbthrift_TypeList_ab3de2bc implements TypeList {

  private static List<TypeMapping> list = new ArrayList<>();

  static {
    // 0577e3815faebd73391359ec7cd3fa6e
    list.add(
        new TypeList.TypeMapping(
            "facebook.com/thrift/test/48/StringBuffer", "java.lang.StringBuffer"));
    // 05776f3a7ee3ecc10ac174c4b3895610
    list.add(
        new TypeList.TypeMapping(
            "facebook.com/thrift/test/288/StringBuilder", "java.lang.StringBuilder"));
  }

  @Override
  public List<TypeList.TypeMapping> getTypes() {
    return list;
  }
}
