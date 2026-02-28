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

include "module.thrift"
include "includes.thrift"
namespace java.swift test.fixtures.includes

/** This is a service-level docblock */
service MyService {
  /** This is a function-level docblock */
  void query(1: module.MyStruct s, 2: includes.Included i);
  void has_arg_docs(
    1: /** arg doc*/ module.MyStruct s,
    /** arg doc */ 2: includes.Included i,
  );
}

// Ensure that we can alias included types into the main namespace.
typedef includes.Included IncludesIncluded
// Typedef-of-typedef, cross-module.
typedef includes.TransitiveFoo IncludesTransitiveFoo
