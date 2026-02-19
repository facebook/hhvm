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

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.mockito.Mockito.*;

import com.facebook.thrift.enums.BaseEnum;
import com.facebook.thrift.enums.ThriftEnum;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.junit.jupiter.MockitoExtension;

@ExtendWith(MockitoExtension.class)
public class EnumUtilTest {

  @Test
  public void testJavaEnum() {
    BaseEnum javaEnum = mock(BaseEnum.class);
    when(javaEnum.isClosedEnum()).thenReturn(true);
    when(javaEnum.getValue()).thenReturn(3);
    assertEquals(3, EnumUtil.getValue(javaEnum));

    when(javaEnum.getValue()).thenReturn(0);
    assertEquals(0, EnumUtil.getValue(javaEnum));
  }

  @Test
  public void testNullEnum() {
    assertEquals(0, EnumUtil.getValue(null));
  }

  @Test
  public void testOpenEnum() {
    ThriftEnum openEnum = mock(ThriftEnum.class);
    when(openEnum.isClosedEnum()).thenReturn(false);
    when(openEnum.getValue()).thenReturn(3);
    assertEquals(3, EnumUtil.getValue(openEnum));

    when(openEnum.getUnrecognizedValue()).thenReturn(10);
    when(openEnum.isValueUnrecognized()).thenReturn(true);
    lenient().when(openEnum.getValue()).thenReturn(5);
    assertEquals(10, EnumUtil.getValue(openEnum));
  }
}
