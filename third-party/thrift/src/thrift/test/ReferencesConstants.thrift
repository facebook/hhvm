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

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Empty {}

struct StructWithRef {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required Empty req_field;
}

const StructWithRef kStructWithRef = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeUnique {
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Unique}
  @cpp.AllowLegacyNonOptionalRef
  3: required Empty req_field;
}

const StructWithRefTypeUnique kStructWithRefTypeUnique = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeShared {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  3: required Empty req_field;
}

const StructWithRefTypeShared kStructWithRefTypeShared = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};

struct StructWithRefTypeSharedConst {
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  1: Empty def_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: optional Empty opt_field;
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  3: required Empty req_field;
}

const StructWithRefTypeSharedConst kStructWithRefTypeSharedConst = {
  "def_field": {},
  "opt_field": {},
  "req_field": {},
};
