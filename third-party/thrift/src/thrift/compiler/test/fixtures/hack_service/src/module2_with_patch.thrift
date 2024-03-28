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

namespace hack hack_ns2

enum Status {
  Unknown = 0,
}

struct FBStruct {
  1: string str_value;
}

service TestService extends module.FooHackService {
  i32 ping(1: string str_arg) throws (1: module.ExTypedef ex);
}
