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

@thrift.AllowLegacyMissingUris
package;

namespace py3 thrift.lib.python.schema.tests

struct LegacyStruct {
  1: i32 value;
  2: optional string name;
  3: LegacyEnum kind;
}

union LegacyUnion {
  1: i32 int_field;
  2: string str_field;
}

exception LegacyException {
  1: string message;
  2: i32 code;
}

enum LegacyEnum {
  UNKNOWN = 0,
  VALUE_A = 1,
  VALUE_B = 2,
}

interaction LegacyInteraction {
  LegacyStruct ping();
}

service LegacyService {
  LegacyStruct getLegacy(1: i32 id);
  LegacyInteraction startLegacy();
}
