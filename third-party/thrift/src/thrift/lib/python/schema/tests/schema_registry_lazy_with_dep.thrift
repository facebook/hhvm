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

include "thrift/lib/python/schema/tests/schema_registry_dep.thrift"

package "thrift.com/python/schema/registry_lazy_with_dep"

namespace py3 thrift.lib.python.schema.tests

struct LazyWithDepStruct {
  1: string name;
  2: schema_registry_dep.DepStruct dep;
}
