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

enum MyEnum {
  MyValue1 = 0,
  MyValue2 = 1,
  DOMAIN = 2 (cpp.name = 'REALM'),
} (cpp.name = "YourEnum")

@cpp.StrongType
typedef i16 MyId

struct MyStructNestedAnnotation {
  1: string name;
}

@cpp.Adapter{name = '::StaticCast'}
union MyUnion {} (cpp.name = "YourUnion")
@cpp.Adapter{name = '::StaticCast'}
safe exception MyException {} (cpp.name = "YourException")

# We intentionally keep field IDs out of order to check whether this case is handled correctly
@cpp.Adapter{name = '::StaticCast'}
struct MyStruct {
  # glibc has macros with this name, Thrift should be able to prevent collisions
  2: i64 major (cpp.name = 'majorVer');
  # package is a reserved keyword in Java, Thrift should be able to handle this
  1: string package (java.swift.name = '_package');
  # should generate valid code even with double quotes in an annotation
  3: string annotation_with_quote (go.tag = 'tag:"somevalue"');
  4: string class_ (java.swift.name = 'class_');
  5: string annotation_with_trailing_comma (custom = 'test');
  6: string empty_annotations ();
  7: MyEnum my_enum;
  8: list<string> (cpp.type = "std::deque<std::string>") cpp_type_annotation;
  9: MyUnion my_union;
  10: MyId my_id;
} (
  cpp.name = "YourStruct",
  android.generate_builder,
  cpp.internal.deprecated._data.method,
  thrift.uri = "facebook.com/thrift/compiler/test/fixtures/basic-annotations/src/module/MyStruct",
  hack.attributes = "\SomeClass(\AnotherClass::class)",
)

const MyStruct myStruct = {
  "major": 42,
  "package": "package",
  "my_enum": MyEnum.DOMAIN,
};

service MyService {
  void ping() throws (1: MyException myExcept);
  string getRandomData();
  bool hasDataById(1: i64 id);
  string getDataById(1: i64 id);
  void putDataById(
    1: i64 id,
    @MyStructNestedAnnotation{name = "argument"}
    2: string data,
  );
  oneway void lobDataById(1: i64 id, 2: string data (cpp.name = "dataStr"));
  void doNothing() (cpp.name = 'cppDoNothing', go.name = 'GoDoNothing');
}

service MyServicePrioParent {
  void ping() (priority = 'IMPORTANT');
  void pong() (priority = 'HIGH_IMPORTANT');
} (priority = 'HIGH')

service MyServicePrioChild extends MyServicePrioParent {
  void pang() (priority = 'BEST_EFFORT');
}

struct SecretStruct {
  1: i64 id;
  2: string password (java.sensitive);
}

interaction BadInteraction {
  void foo();
} (cpp.name = "GoodInteraction")

service BadService {
  performs BadInteraction;
  i32 bar();
} (cpp.name = "GoodService")
