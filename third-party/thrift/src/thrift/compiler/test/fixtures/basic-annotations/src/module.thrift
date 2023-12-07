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

namespace java test.fixtures.basicannotations
namespace java.swift test.fixtures.basicannotations

include "thrift/annotation/cpp.thrift"
include "thrift/annotation/go.thrift"
include "thrift/annotation/hack.thrift"
include "thrift/annotation/thrift.thrift"

@cpp.Name{value = "YourEnum"}
enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
  @cpp.Name{value = "REALM"}
  DOMAIN = 2,
}

struct MyStructNestedAnnotation {
  1: string name;
}

@cpp.Adapter{name = '::StaticCast'}
@cpp.Name{value = "YourUnion"}
union MyUnion {}
@cpp.Name{value = "YourException"}
@cpp.Adapter{name = '::StaticCast'}
safe exception MyException {}

# We intentionally keep field IDs out of order to check whether this case is handled correctly
@cpp.Name{value = "YourStruct"}
@hack.Attributes{attributes = ["\SomeClass(\AnotherClass::class)"]}
@cpp.Adapter{name = '::StaticCast'}
struct MyStruct {
  # glibc has macros with this name, Thrift should be able to prevent collisions
  @cpp.Name{value = "majorVer"}
  @go.Name{name = "MajorVer"}
  2: i64 major;
  # abstract is a reserved keyword in Java, Thrift should be able to handle this
  @go.Name{name = "AbstractName"}
  @go.Tag{tag = 'tag:"some_abstract"'}
  1: string abstract (java.swift.name = '_abstract');
  # should generate valid code even with double quotes in an annotation
  @go.Tag{tag = 'tag:"somevalue"'}
  3: string annotation_with_quote;
  4: string class_ (java.swift.name = 'class_');
  5: string annotation_with_trailing_comma (custom = 'test');
  6: string empty_annotations ();
  7: MyEnum my_enum;
  8: list_string_6884 cpp_type_annotation;
  9: MyUnion my_union;
} (
  android.generate_builder,
  thrift.uri = "facebook.com/thrift/compiler/test/fixtures/basic-annotations/src/module/MyStruct",
)

@go.Name{name = "IncredibleStruct"}
typedef MyStruct AwesomeStruct
@go.Name{name = "BrilliantStruct"}
typedef MyStruct FantasticStruct

const MyStruct myStruct = {
  "major": 42,
  "abstract": "abstract",
  "my_enum": MyEnum.DOMAIN,
};

service MyService {
  void ping() throws (1: MyException myExcept);
  string getRandomData();
  bool hasDataById(1: i64 id);
  @go.Name{name = "GoGetDataById"}
  string getDataById(1: i64 id);
  void putDataById(
    1: i64 id,
    @MyStructNestedAnnotation{name = "argument"}
    2: string data,
  );
  oneway void lobDataById(1: i64 id, 2: string data (cpp.name = "dataStr"));
  @cpp.Name{value = "cppDoNothing"}
  @go.Name{name = "GoDoNothing"}
  void doNothing();
}

service MyServicePrioParent {
  @thrift.Priority{level = thrift.RpcPriority.IMPORTANT}
  void ping();
  @thrift.Priority{level = thrift.RpcPriority.HIGH_IMPORTANT}
  void pong();
}

service MyServicePrioChild extends MyServicePrioParent {
  @thrift.Priority{level = thrift.RpcPriority.BEST_EFFORT}
  void pang();
}

struct SecretStruct {
  1: i64 id;
  @thrift.DeprecatedUnvalidatedAnnotations{items = {"java.sensitive": "1"}}
  2: string password;
}

@cpp.Name{value = "GoodInteraction"}
interaction BadInteraction {
  void foo();
}

@cpp.Name{value = "GoodService"}
service BadService {
  performs BadInteraction;
  i32 bar();
}

service FooBarBazService {
  @go.Name{name = "FooStructured"}
  void foo();
  @go.Name{name = "BarNonStructured"}
  void bar();
  void baz();
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{name = "std::deque<std::string>"}
typedef list<string> list_string_6884
