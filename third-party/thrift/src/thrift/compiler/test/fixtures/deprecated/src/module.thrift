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

include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

namespace cpp2 apache.thrift.test
namespace py3 thrift.test
namespace java.swift test.fixtures.deprecated
namespace go thrift.test.deprecated
namespace hack test.fixtures.deprecated

/**
 * Example struct demonstrating @thrift.Deprecated annotation on fields.
 */
struct User {
  1: i64 id;

  /** @deprecated Use 'full_name' instead */
  @thrift.Deprecated{message = "Use 'full_name' instead"}
  2: string name;

  3: string full_name;

  /** @deprecated This field is no longer used */
  @thrift.Deprecated{message = "This field is no longer used"}
  4: optional string email;

  5: optional string email_address;
}

/**
 * Example union with deprecated field.
 */
union ExampleUnion {
  1: string stringValue;
  @thrift.Deprecated{message = "Use stringValue instead"}
  2: i32 intValue;
  3: double doubleValue;
}
