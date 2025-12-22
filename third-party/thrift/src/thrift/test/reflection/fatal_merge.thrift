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

#
# exp = the expected new value of dst after merging
# nil = the expected new value of src after merging with rvalue-ref
#

cpp_include "thrift/test/reflection/fatal_merge_types.h"

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

struct Basic {
  1: string b;
  2: optional string b_opt;
  3: required string b_req;
}

struct BasicExample {
  1: Basic src;
  2: Basic dst;
  3: Basic exp;
  4: Basic nil;
}

const BasicExample kBasicExample = {
  "src": {"b": "hello", "b_opt": "hello_opt", "b_req": "hello_req"},
  "dst": {},
  "exp": {"b": "hello", "b_opt": "hello_opt", "b_req": "hello_req"},
  "nil": {"b_opt": ""},
};

const BasicExample kBasicOptionalExample = {
  "src": {"b": "hello", "b_req": "hello_req"},
  "dst": {"b_opt": "hello_opt"},
  "exp": {"b": "hello", "b_opt": "hello_opt", "b_req": "hello_req"},
  "nil": {},
};

struct BasicList {
  1: list<Basic> l;
}

struct BasicListExample {
  1: BasicList src;
  2: BasicList dst;
  3: BasicList exp;
  4: BasicList nil;
}

const BasicListExample kBasicListExample = {
  "src": {"l": [{"b": "hello"}]},
  "dst": {"l": []},
  "exp": {"l": [{"b": "hello"}]},
  "nil": {"l": [{}]},
};

struct BasicSet {
  1: set<Basic> l;
}

struct BasicSetExample {
  1: BasicSet src;
  2: BasicSet dst;
  3: BasicSet exp;
  4: BasicSet nil;
}

const BasicSetExample kBasicSetExample = {
  "src": {"l": [{"b": "hello"}]},
  "dst": {"l": []},
  "exp": {"l": [{"b": "hello"}]},
  "nil": {"l": [{"b": "hello"}]},
};

struct BasicMap {
  1: map<string, Basic> l;
}

struct BasicMapExample {
  1: BasicMap src;
  2: BasicMap dst;
  3: BasicMap exp;
  4: BasicMap nil;
}

const BasicMapExample kBasicMapExample = {
  "src": {"l": {"foo": {"b": "hello"}}},
  "dst": {"l": {}},
  "exp": {"l": {"foo": {"b": "hello"}}},
  "nil": {"l": {"foo": {}}},
};

struct NestedMap {
  1: map<string, map<string, Basic>> l;
}

struct NestedMapExample {
  1: NestedMap src;
  2: NestedMap dst;
  3: NestedMap exp;
  4: NestedMap nil;
}

const NestedMapExample kNestedMapExample = {
  "src": {"l": {"outer_foo": {"inner_foo": {"b": "hello"}}}},
  "dst": {},
  "exp": {"l": {"outer_foo": {"inner_foo": {"b": "hello"}}}},
  "nil": {"l": {"outer_foo": {"inner_foo": {}}}},
};

struct Nested {
  1: Basic a;
  2: Basic b;
  3: string c;
  4: string d;
}

struct NestedExample {
  1: Nested src;
  2: Nested dst;
  3: Nested exp;
  4: Nested nil;
}

const NestedExample kNestedExample = {
  "src": {"b": {"b": "hello"}, "d": "bar"},
  "dst": {"a": {"b": "world"}, "c": "foo"},
  "exp": {"a": {"b": ""}, "b": {"b": "hello"}, "c": "", "d": "bar"}, # shouldn't this be "foo"?
  "nil": {},
};

struct NestedRefUnique {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional Basic a;
  @cpp.Ref{type = cpp.RefType.Unique}
  2: optional Basic b;
  3: string c;
  4: string d;
}

struct NestedRefUniqueExample {
  1: NestedRefUnique src;
  2: NestedRefUnique dst;
  3: NestedRefUnique exp;
  4: NestedRefUnique nil;
}

const NestedRefUniqueExample kNestedRefUniqueExample = {
  "src": {"b": {"b": "hello"}, "d": "bar"},
  "dst": {"a": {"b": "world"}, "c": "foo"},
  "exp": {
    # "a": {"b": ""}, # why not this?
    "b": {"b": "hello"},
    "c": "", # shouldn't this be "foo"?
    "d": "bar",
  },
  "nil": {},
};

struct NestedRefShared {
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  1: optional Basic a;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  2: optional Basic b;
  3: string c;
  4: string d;
}

struct NestedRefSharedExample {
  1: NestedRefShared src;
  2: NestedRefShared dst;
  3: NestedRefShared exp;
  4: NestedRefShared nil;
}

const NestedRefSharedExample kNestedRefSharedExample = {
  "src": {"b": {"b": "hello"}, "d": "bar"},
  "dst": {"a": {"b": "world"}, "c": "foo"},
  "exp": {
    # should "a" be {"b": ""}?
    "b": {"b": "hello"},
    "c": "", # shouldn't this be "foo"?
    "d": "bar",
  },
  "nil": {},
};

struct NestedRefSharedConst {
  @cpp.Ref{type = cpp.RefType.Shared}
  1: optional Basic a;
  @cpp.Ref{type = cpp.RefType.Shared}
  2: optional Basic b;
  3: string c;
  4: string d;
}

struct NestedRefSharedConstExample {
  1: NestedRefSharedConst src;
  2: NestedRefSharedConst dst;
  3: NestedRefSharedConst exp;
  4: NestedRefSharedConst nil;
}

const NestedRefSharedConstExample kNestedRefSharedConstExample = {
  "src": {"b": {"b": "hello"}, "d": "bar"},
  "dst": {"a": {"b": "world"}, "c": "foo"},
  "exp": {
    # should "a" be {"b": ""}?
    "b": {"b": "hello"},
    "c": "", # shouldn't this be "foo"?
    "d": "bar",
  },
  "nil": {},
};

struct NestedBox {
  @thrift.Box
  1: optional Basic a;
  @thrift.Box
  2: optional Basic b;
  @thrift.Box
  3: optional string c;
  @thrift.Box
  4: optional string d;
}

struct NestedBoxExample {
  1: NestedBox src;
  2: NestedBox dst;
  3: NestedBox exp;
  4: NestedBox nil;
}

const NestedBoxExample kNestedBoxExample = {
  "src": {"b": {"b": "hello"}, "d": "bar"},
  "dst": {"a": {"b": "world"}, "c": "foo"},
  "exp": {"a": {"b": "world"}, "b": {"b": "hello"}, "c": "foo", "d": "bar"},
  "nil": {},
};

union BasicUnion {
  1: i32 a;
  2: string b;
}

struct BasicUnionExample {
  1: BasicUnion src;
  2: BasicUnion dst;
  3: BasicUnion exp;
  4: BasicUnion nil;
}

const BasicUnionExample kBasicUnionExample1 = {
  "src": {"a": 1},
  "dst": {},
  "exp": {"a": 1},
  "nil": {},
};

const BasicUnionExample kBasicUnionExample2 = {
  "src": {},
  "dst": {"a": 1},
  "exp": {},
  "nil": {},
};

const BasicUnionExample kBasicUnionExample3 = {
  "src": {"a": 2},
  "dst": {"a": 1},
  "exp": {"a": 2},
  "nil": {},
};

const BasicUnionExample kBasicUnionExample4 = {
  "src": {"a": 1},
  "dst": {"b": "wat"},
  "exp": {"a": 1},
  "nil": {},
};

struct MapUnion {
  1: map<i64, BasicUnion> l;
}

struct MapUnionExample {
  1: MapUnion src;
  2: MapUnion dst;
  3: MapUnion exp;
  4: MapUnion nil;
}

const MapUnionExample kMapUnionExample = {
  "src": {"l": {1: {"a": 1}}},
  "dst": {"l": {}},
  "exp": {"l": {1: {"a": 1}}},
  "nil": {"l": {1: {}}},
};
