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

package "meta.com/thrift/test/digest/single_empty_struct"

namespace cpp2 apache.thrift.type_system.test.digest_single_empty_struct
namespace rust digest_fixture_single_empty_struct
namespace py3 apache.thrift.type_system.test

@thrift.Uri{value = "meta.com/test/Empty"}
struct Empty {}
