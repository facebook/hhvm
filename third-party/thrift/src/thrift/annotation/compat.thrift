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

include "thrift/annotation/scope.thrift"

package "facebook.com/thrift/annotation"

namespace java com.facebook.thrift.annotation_deprecated
namespace android com.facebook.thrift.annotation_deprecated
namespace py.asyncio facebook_thrift_asyncio.annotation.compat
namespace go thrift.annotation.compat
namespace py thrift.annotation.compat

/**
 * Enumeration for coding error actions.
 * Used for handling malformed UTF-8 bytes.
 */
enum CodingErrorAction {
  /**
   * Default behavior of the languages.
   * Pythons default is report, java is replacing malformed
   * bytes, cpp skips validation and ignores malformed bytes.
   */
  Legacy = 0,
  /**
   * A decode error is thrown when the bytes are malformed.
   * Default behavior in Thrift v1.
   */
  Report = 1,
}

/**
 * Enables compatibility on string and binary types. When target data
 * type is string, UTF-8 byte sequence is validated. If it is invalid,
 * one of the CodingErrorAction is taken.
 */
@scope.Program
@scope.Structured
@scope.Field
@scope.Typedef
struct Strings {
  1: CodingErrorAction onInvalidUtf8;
}

@Strings{onInvalidUtf8 = CodingErrorAction.Report}
@scope.Transitive
struct Utf8 {}

@Strings{onInvalidUtf8 = CodingErrorAction.Legacy}
@scope.Transitive
struct LegacyString {}

/**
 * Enumeration for enum types.
 */
enum EnumType {
  /**
   * Default enum type of the languages in Thrift v0.
   * C++ default is open, Java is closed.
   */
  Legacy = 0,
  /**
   * Enum is defined as open enum type.
   * A new enum value UNRECOGNIZED is added to the enum for the values outside the defined range.
   * Unrecognized values are stored as its underlying integer value and can be retrieved.
   * Default behavior in Thrift v1.
   */
  Open = 1,
}

/**
 * Enables compatibility on enums.
 */
@scope.Program
@scope.Enum
struct Enums {
  1: EnumType type;
}
