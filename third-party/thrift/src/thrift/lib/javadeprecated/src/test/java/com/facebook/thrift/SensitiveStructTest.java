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

package com.facebook.thrift;

import static org.junit.Assert.assertFalse;

import com.facebook.thrift.java.test.MySensitiveStruct;
import org.junit.Test;

public class SensitiveStructTest {
  private static final String password = "toto1234";
  private static final String errorMsg = "toString should not contain sensitive fields content!";

  @Test
  public void testJavaDeprecated() throws Exception {
    MySensitiveStruct struct = new MySensitiveStruct(123L, password);
    assertFalse(errorMsg, struct.toString().contains(password));
  }

  @Test
  public void testAndroid() throws Exception {
    com.facebook.thrift.android.test.MySensitiveStruct struct =
        new com.facebook.thrift.android.test.MySensitiveStruct(123L, password);
    assertFalse(errorMsg, struct.toString().contains(password));
  }

  @Test
  public void testJavaSwift() throws Exception {
    com.facebook.thrift.javaswift.test.MySensitiveStruct struct =
        new com.facebook.thrift.javaswift.test.MySensitiveStruct.Builder()
            .setId(123L)
            .setPassword(password)
            .build();
    assertFalse(errorMsg, struct.toString().contains(password));
  }
}
