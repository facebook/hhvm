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

include "thrift/annotation/hack.thrift"

// Mangled services without `namespace php` are a no-op: the package
// namespace already disambiguates services across thrift files, so there is
// nothing for `mangledsvcs` to do. `@hack.NamePrefix` (if present) is still
// applied to the generated class names just as it would be without
// `mangledsvcs`.
@hack.NamePrefix{prefix = "Beeble_", apply_to_services = true}
package "facebook.com/thrift/test/fixtures/hack_mangledsvcs_name_prefix"

namespace hack ""

service Brox {
}
