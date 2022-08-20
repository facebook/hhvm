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

namespace cpp2 metadata.test.struct_union

typedef list<string> StringList
typedef list<list<string>> StringListList

struct Dog {
  1: string name;
}

struct Cat {
  1: string name;
}

union Pet {
  1: Dog dog;
  2: Cat cat;
}

union ComplexUnion {
  1: Pet pet;
  2: StringList string_list;
  3: StringListList string_list_list;
  4: list<string> raw_string_list;
}

service StructUnionTestService {
  ComplexUnion getData(1: Dog dog, 2: Cat cat, 4: Pet pet);
}
