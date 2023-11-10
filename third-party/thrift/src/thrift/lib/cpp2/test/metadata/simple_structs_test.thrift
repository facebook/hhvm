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

namespace cpp2 metadata.test.simple_structs

include "thrift/annotation/cpp.thrift"

struct Nat {
  1: string data = "";
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Nat next;
}

typedef Nat NatTypedef

@NatTypedef{data = "Abbr_typedef"}
typedef list<string> Abbreviations

struct Map {
  1: map<i64, string> value;
}

struct Country {
  1: string name;
  3: string capital;
  10: double population;
  @NatTypedef{data = "Abbr_field"}
  4: Abbreviations abbreviations;
}

@Nat{data = "struct"}
struct City {
  @Map{value = {0: "0", 1: "1"}}
  1: string name;
  @NatTypedef{
    data = "2",
    next = NatTypedef{data = "1", next = NatTypedef{data = "0"}},
  }
  2: string country;
  3: double population;
}

service SimpleStructsTestService {
  Country getCountry(1: City city);
}
