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

include "thrift/annotation/python.thrift"

include "c.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

typedef c.C1 C1

@python.Adapter{
  name = "two.AdapterTwo",
  typeHint = "typeshed_two.AdapterTwoType[]",
}
typedef c.C2 C2

@python.Adapter{
  name = "one.AdapterOne",
  typeHint = "typeshed_one.AdapterOneType[]",
}
struct B {}
