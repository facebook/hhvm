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
include "thrift/annotation/python.thrift"

package "thrift.com/python/test"

@python.Name{name = "RenamedEmpty"}
struct Empty {}

struct Struct {
  1: i32 first;
  @python.Name{name = "renamed_second"}
  2: i32 second;
}

exception ExceptionWithMessage {
  @thrift.ExceptionMessage
  @python.Name{name = "message"}
  1: string msg;
}
