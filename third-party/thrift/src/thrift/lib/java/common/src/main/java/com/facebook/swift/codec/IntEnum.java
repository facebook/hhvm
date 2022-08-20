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

package com.facebook.swift.codec;

/**
 * Enumeration that is really just an int, as the enum is too large for java to handle. java only
 * allows 64k of code in the constants pool of a class. if you have more than a few thousand enums
 * in a thrift file, java will no longer be able to compile that file. Enums marked with
 * java.swift.skip_enum_name_map = 1 will not try to generate a java enum as it would not be
 * compilable.
 */
public abstract class IntEnum {
  /** Get as an int value */
  public abstract int getValue();
}
