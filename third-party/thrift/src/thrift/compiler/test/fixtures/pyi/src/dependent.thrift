/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 simple.dependent
namespace py simple.dependent
namespace py.asyncio simple.dependent_asyncio
namespace py3 simple.dependent

struct Item {
  1: string key;
  2: optional binary value;
  3: ItemEnum enum_value;
}

enum ItemEnum {
  OPTION_ONE = 1,
  OPTION_TWO = 2,
}
