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

package com.facebook.thrift.adapter.test;

import com.facebook.thrift.adapter.common.EnumTypeAdapter;
import com.facebook.thrift.test.common.TestEnumOpen;
import com.facebook.thrift.util.EnumUtil;

public class OpenEnumToStringTypeAdapter implements EnumTypeAdapter<TestEnumOpen, String> {
  @Override
  public String fromThrift(TestEnumOpen testEnum) {
    return testEnum == null ? null : String.valueOf(EnumUtil.getValue(testEnum));
  }

  @Override
  public TestEnumOpen toThrift(String s) {
    return s == null ? null : TestEnumOpen.fromInteger(Integer.valueOf(s));
  }
}
