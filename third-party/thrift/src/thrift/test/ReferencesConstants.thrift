/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 apache.thrift.test

struct Empty {}

struct StructWithRef {
  1: Empty def_field (cpp.ref);
  2: optional Empty opt_field (cpp.ref);
  3: required Empty req_field (cpp.ref);
}

const StructWithRef kStructWithRef = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeUnique {
  1: Empty def_field (cpp.ref_type = "unique");
  2: optional Empty opt_field (cpp.ref_type = "unique");
  3: required Empty req_field (cpp.ref_type = "unique");
}

const StructWithRefTypeUnique kStructWithRefTypeUnique = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeShared {
  1: Empty def_field (cpp.ref_type = "shared");
  2: optional Empty opt_field (cpp.ref_type = "shared");
  3: required Empty req_field (cpp.ref_type = "shared");
}

const StructWithRefTypeShared kStructWithRefTypeShared = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeSharedConst {
  1: Empty def_field (cpp.ref_type = "shared_const");
  2: optional Empty opt_field (cpp.ref_type = "shared_const");
  3: required Empty req_field (cpp.ref_type = "shared_const");
}

const StructWithRefTypeSharedConst kStructWithRefTypeSharedConst = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};
