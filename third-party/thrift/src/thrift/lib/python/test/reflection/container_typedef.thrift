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

include "thrift/lib/python/test/reflection/included_container_typedef.thrift"

package "thrift.com/python/test/reflection/container_typedef"

namespace py3 python_test.reflection

// A named container typedef defined in *this* (the service's own) file. This
// path already worked and guards against a regression from the fix.
typedef list<i32> ReproSameFileList

service ReflectionReproService {
  // Cross-file named container typedef as arg + return: the reflection bug.
  included_container_typedef.ReproDepMap echo_map(
    1: included_container_typedef.ReproDepMap m,
  );

  // Same-file named container typedef: must keep working after the fix.
  ReproSameFileList echo_same(1: ReproSameFileList l);

  // Cross-file Py3Hidden named container typedef, mirroring acl.IdentitySet.
  included_container_typedef.ReproHiddenSet echo_hidden(
    1: included_container_typedef.ReproHiddenSet s,
  );
}
