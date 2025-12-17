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

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.Experimental
package "facebook.com/thrift/test/fixtures/field_interceptor"

struct InterceptedFields {
  @cpp.FieldInterceptor{name = "my::FieldInterceptor", noinline = true}
  1: i32 access_field;

  @cpp.FieldInterceptor{name = "my::FieldInterceptor"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  @cpp.AllowLegacyNonOptionalRef
  2: i32 access_shared_field;

  @cpp.FieldInterceptor{name = "my::FieldInterceptor"}
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: optional i32 access_optional_shared_field;

  @cpp.FieldInterceptor{name = "my::FieldInterceptor"}
  @cpp.Ref{type = cpp.RefType.Shared}
  @cpp.AllowLegacyNonOptionalRef
  4: i32 access_shared_const_field;

  @cpp.FieldInterceptor{name = "my::FieldInterceptor"}
  @cpp.Ref{type = cpp.RefType.Shared}
  5: optional i32 access_optional_shared_const_field;

  @cpp.FieldInterceptor{name = "my::FieldInterceptor"}
  @thrift.Box
  6: optional i32 access_optional_boxed_field;
}
