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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/test/metadata/gen-cpp2/nested_structs_test_metadata.h> // @manual=:nested_structs_test_thrift-cpp2-metadata
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/simple_structs_test_metadata.h> // @manual=:simple_structs_test_thrift-cpp2-metadata

namespace apache::thrift::detail::md {

TEST(Foo, base) {
  metadata::ThriftMetadata md;
  const metadata::ThriftStruct& ts =
      StructMetadata<::metadata::test::nested_structs::Foo>::gen(md);
  EXPECT_EQ(md.structs_ref()->size(), 1);
  EXPECT_EQ(&ts, &md.structs_ref()->at("nested_structs_test.Foo"));
}

TEST(City, base) {
  metadata::ThriftMetadata md;
  const metadata::ThriftStruct& ts =
      StructMetadata<::metadata::test::nested_structs::City>::gen(md);
  EXPECT_EQ(md.structs_ref()->size(), 2);
  EXPECT_EQ(&ts, &md.structs_ref()->at("nested_structs_test.City"));
}

TEST(City, structured_metadata) {
  const metadata::ThriftStruct& metadata =
      get_struct_metadata<::metadata::test::simple_structs::City>();

  const metadata::ThriftConstStruct& struct_annotation =
      metadata.structured_annotations()->at(0);
  EXPECT_EQ(
      struct_annotation.type_ref()->name_ref(), "simple_structs_test.Nat");
  const metadata::ThriftConstValue& value =
      struct_annotation.fields_ref()->at("data");
  EXPECT_EQ(value.cv_string_ref(), "struct");

  const metadata::ThriftField& field = metadata.fields_ref()->at(0);
  const auto field_annotation = field.structured_annotations()->at(0);
  EXPECT_EQ(field_annotation.type_ref()->name_ref(), "simple_structs_test.Map");
  auto field_value = field_annotation.fields_ref()->at("value");
  for (auto kv : *field_value.cv_map_ref()) {
    if (kv.key_ref()->cv_integer_ref() == 0) {
      EXPECT_EQ(kv.value_ref()->cv_string_ref(), "0");
    } else {
      EXPECT_EQ(kv.key_ref()->cv_integer_ref(), 1);
      EXPECT_EQ(kv.value_ref()->cv_string_ref(), "1");
    }
  }
}

TEST(Country, AccessMetadataTwice) {
  // Previously we had bug that some data are moved out when generating
  // metadata. In which case you will get empty fields when generating metadata
  // the second time. This unit-test makes sure this won't happen again.
  using ::metadata::test::simple_structs::Country;
  metadata::ThriftMetadata md1, md2;
  auto s1 = StructMetadata<Country>::gen(md1);
  auto s2 = StructMetadata<Country>::gen(md2);
  EXPECT_EQ(s1, s2);
}

} // namespace apache::thrift::detail::md
