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

package "test.dev/fixtures/hack_object_key_containers"

namespace hack test.fixtures.hack_object_key_containers

include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

// A simple struct to use as a map key / set element.
struct MyStruct {
  1: i64 id;
  2: string name;
}

@hack.Wrapper{name = '\\MyStructWrapper'}
struct WrappedStruct {
  1: i64 id;
}

// A struct that becomes async-shapish in Hack because it uses a field wrapper.
struct AsyncShapeStruct {
  @hack.FieldWrapper{name = '\\MyFieldWrapper'}
  1: i64 id;
}

// A recursive union similar to protocol_detail.thrift's Value.
// Tests that object-key containers work with recursive types.
union Value {
  // Primitives.
  1: bool boolValue;
  2: i64 intValue;
  3: string stringValue;
  4: double doubleValue;

  // Struct value.
  5: MyStruct structValue;

  // Containers of values (recursive).
  6: list<Value> listValue;

  @thrift.AllowUnsafeNonSealedKeyType
  7: set<Value> setValue;

  @thrift.AllowUnsafeNonSealedKeyType
  8: map<Value, Value> mapValue;
}

// A struct that uses various non-arraykey container types.
struct ContainerOfObjects {
  // Set with struct elements.
  1: set<MyStruct> structSet;

  // Map with struct keys.
  2: map<MyStruct, string> structMap;

  // Set with float elements.
  3: set<float> floatSet;

  // Set with bool elements.
  4: set<bool> boolSet;

  // Map with bool keys.
  5: map<bool, string> boolMap;

  // Set with the recursive Value union.
  @thrift.AllowUnsafeNonSealedKeyType
  6: set<Value> valueSet;

  // Map with the recursive Value union as key.
  @thrift.AllowUnsafeNonSealedKeyType
  7: map<Value, i64> valueMap;

  // Mixed: normal fields alongside object-key fields.
  8: string normalField;
  9: map<string, i64> normalMap;

  // Object-key map with a wrapped value.
  10: map<MyStruct, WrappedStruct> wrappedStructValueMap;
}

// Adapted typedef: the underlying type is a struct (non-arraykey).
@hack.Adapter{name = '\\MyStructAdapter'}
typedef MyStruct AdaptedMyStruct

// Adapted typedef on an arraykey type: tests that the adapted type
// still gets object_key format since THackType may not be arraykey.
@hack.Adapter{name = '\\MyI64Adapter'}
typedef i64 AdaptedI64

// Struct exercising adapted types as map keys and set elements.
struct ContainerOfAdaptedObjects {
  1: map<AdaptedMyStruct, string> adaptedStructMap;
  2: set<AdaptedMyStruct> adaptedStructSet;
  3: map<AdaptedI64, string> adaptedI64Map;
  4: set<AdaptedI64> adaptedI64Set;
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  5: map<MyStruct, AdaptedMyStruct> adaptedStructValueMap;
}

// Struct exercising async and nullable shape methods for object-key containers.
struct ContainerOfAsyncShapeObjects {
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  1: set<AsyncShapeStruct> asyncStructSet;
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  2: map<AsyncShapeStruct, AsyncShapeStruct> asyncStructMap;
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  3: optional set<AsyncShapeStruct> optionalAsyncStructSet;
  // @lint-ignore THRIFTCHECKS bad-key-type
  @thrift.AllowUnsafeNonSealedKeyType
  4: optional map<AsyncShapeStruct, AsyncShapeStruct> optionalAsyncStructMap;
}
