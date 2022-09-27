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

#include <thrift/test/reflection/gen-cpp2/reflection_fatal.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>

#include <folly/portability/GTest.h>

namespace test_cpp2 {
namespace cpp_reflection {

FATAL_S(cpp_s, "cpp");
FATAL_S(cpp_ns, "test_cpp1::cpp_reflection");
FATAL_S(cpp2_s, "cpp2");
FATAL_S(cpp2_ns, "test_cpp2::cpp_reflection");
FATAL_S(d_s, "d");
FATAL_S(d_2ns, "test_d.cpp_reflection");
FATAL_S(java_s, "java");
FATAL_S(java_ns, "test_java.cpp_reflection");
FATAL_S(java_swift_s, "java.swift");
FATAL_S(java_swift_ns, "test_swift.cpp_reflection");
FATAL_S(php_s, "php");
FATAL_S(php_ns, "test_php_cpp_reflection");
FATAL_S(python_s, "py3");
FATAL_S(python_ns, "test_py.cpp_reflection");

FATAL_S(enum1s, "enum1");
FATAL_S(enum2s, "enum2");
FATAL_S(enum3s, "enum3");
FATAL_S(enum_with_renamed_values, "enum_with_renamed_value");

FATAL_S(union1s, "union1");
FATAL_S(union2s, "union2");
FATAL_S(union3s, "union3");
FATAL_S(unionAs, "unionA");
FATAL_S(union_with_renamed_fields, "union_with_renamed_field");

FATAL_S(structAs, "structA");
FATAL_S(structBs, "structB");
FATAL_S(structCs, "structC");
FATAL_S(struct1s, "struct1");
FATAL_S(struct2s, "struct2");
FATAL_S(struct3s, "struct3");
FATAL_S(struct4s, "struct4");
FATAL_S(struct5s, "struct5");
FATAL_S(struct_binarys, "struct_binary");
FATAL_S(dep_A_structs, "dep_A_struct");
FATAL_S(annotateds, "annotated");
FATAL_S(my_structAs, "my_structA");
FATAL_S(hasRefUnique_name, "hasRefUnique");
FATAL_S(hasRefShared_name, "hasRefShared");
FATAL_S(hasRefSharedConst_name, "hasRefSharedConst");
FATAL_S(hasRefUniqueSimple_name, "hasRefUniqueSimple");
FATAL_S(hasRefSharedSimple_name, "hasRefSharedSimple");
FATAL_S(hasRefSharedConstSimple_name, "hasRefSharedConstSimple");
FATAL_S(hasBox_name, "hasBox");
FATAL_S(hasBoxSimple_name, "hasBoxSimple");
FATAL_S(StructWithIOBufs, "StructWithIOBuf");
FATAL_S(struct_with_renamed_fields, "struct_with_renamed_field");
FATAL_S(IntStructs, "IntStruct");
FATAL_S(StructWithAdaptedFields, "StructWithAdaptedField");
FATAL_S(StructWithVectorBools, "StructWithVectorBool");

FATAL_S(constant1s, "constant1");
FATAL_S(constant2s, "constant2");
FATAL_S(constant3s, "constant3");

FATAL_S(service1s, "service1");
FATAL_S(service2s, "service2");
FATAL_S(service3s, "service3");

FATAL_S(enum_with_special_namess, "enum_with_special_names");
FATAL_S(union_with_special_namess, "union_with_special_names");
FATAL_S(struct_with_special_namess, "struct_with_special_names");
FATAL_S(service_with_special_namess, "service_with_special_names");
FATAL_S(constant_with_special_names, "constant_with_special_name");

FATAL_S(variantHasRefUniques, "variantHasRefUnique");

TEST(fatal, tags) {
  EXPECT_SAME<cpp_s, reflection_tags::languages::cpp>();
  EXPECT_SAME<cpp2_s, reflection_tags::languages::cpp2>();
  EXPECT_SAME<d_s, reflection_tags::languages::d>();
  EXPECT_SAME<java_s, reflection_tags::languages::java>();
  EXPECT_SAME<java_swift_s, reflection_tags::languages::java_swift>();
  EXPECT_SAME<php_s, reflection_tags::languages::php>();
  EXPECT_SAME<python_s, reflection_tags::languages::py3>();

  EXPECT_SAME<enum1s, reflection_tags::enums::enum1>();
  EXPECT_SAME<enum2s, reflection_tags::enums::enum2>();
  EXPECT_SAME<enum3s, reflection_tags::enums::enum3>();
  EXPECT_SAME<
      enum_with_special_namess,
      reflection_tags::enums::enum_with_special_names>();

  EXPECT_SAME<union1s, reflection_tags::unions::union1>();
  EXPECT_SAME<union2s, reflection_tags::unions::union2>();
  EXPECT_SAME<union3s, reflection_tags::unions::union3>();
  EXPECT_SAME<
      union_with_special_namess,
      reflection_tags::unions::union_with_special_names>();

  EXPECT_SAME<structAs, reflection_tags::structs::structA>();
  EXPECT_SAME<structBs, reflection_tags::structs::structB>();
  EXPECT_SAME<struct1s, reflection_tags::structs::struct1>();
  EXPECT_SAME<struct2s, reflection_tags::structs::struct2>();
  EXPECT_SAME<struct3s, reflection_tags::structs::struct3>();
  EXPECT_SAME<struct_binarys, reflection_tags::structs::struct_binary>();
  EXPECT_SAME<annotateds, reflection_tags::structs::annotated>();
  EXPECT_SAME<
      struct_with_special_namess,
      reflection_tags::structs::struct_with_special_names>();
  EXPECT_SAME<my_structAs, reflection_tags::structs::my_structA>();

  EXPECT_SAME<constant1s, reflection_tags::constants::constant1>();
  EXPECT_SAME<constant2s, reflection_tags::constants::constant2>();
  EXPECT_SAME<constant3s, reflection_tags::constants::constant3>();
  EXPECT_SAME<
      constant_with_special_names,
      reflection_tags::constants::constant_with_special_name>();

  EXPECT_SAME<service1s, reflection_tags::services::service1>();
  EXPECT_SAME<service2s, reflection_tags::services::service2>();
  EXPECT_SAME<service3s, reflection_tags::services::service3>();
  EXPECT_SAME<
      service_with_special_namess,
      reflection_tags::services::service_with_special_names>();
}

TEST(fatal, metadata) {
  using info = apache::thrift::reflect_module<reflection_tags::module>;

  EXPECT_SAME<
      info,
      apache::thrift::try_reflect_module<reflection_tags::module, void>>();
  EXPECT_SAME<void, apache::thrift::try_reflect_module<int, void>>();

  EXPECT_SAME<
      fatal::list<
          fatal::pair<cpp_s, cpp_ns>,
          fatal::pair<cpp2_s, cpp2_ns>,
          fatal::pair<d_s, d_2ns>,
          fatal::pair<java_s, java_ns>,
          fatal::pair<java_swift_s, java_swift_ns>,
          fatal::pair<php_s, php_ns>,
          fatal::pair<python_s, python_ns>>,
      info::namespaces>();

  EXPECT_SAME<
      fatal::list<
          fatal::pair<enum1, enum1s>,
          fatal::pair<enum2, enum2s>,
          fatal::pair<enum3, enum3s>,
          fatal::pair<enum_with_renamed_value, enum_with_renamed_values>,
          fatal::pair<enum_with_special_names, enum_with_special_namess>>,
      info::enums>();

  EXPECT_SAME<
      fatal::list<
          fatal::pair<union1, union1s>,
          fatal::pair<union2, union2s>,
          fatal::pair<union3, union3s>,
          fatal::pair<unionA, unionAs>,
          fatal::pair<union_with_special_names, union_with_special_namess>,
          fatal::pair<variantHasRefUnique, variantHasRefUniques>,
          fatal::pair<union_with_renamed_field, union_with_renamed_fields>>,
      info::unions>();

  EXPECT_SAME<
      fatal::list<
          fatal::pair<structA, structAs>,
          fatal::pair<structB, structBs>,
          fatal::pair<structC, structCs>,
          fatal::pair<struct1, struct1s>,
          fatal::pair<struct2, struct2s>,
          fatal::pair<struct3, struct3s>,
          fatal::pair<struct4, struct4s>,
          fatal::pair<struct5, struct5s>,
          fatal::pair<struct_binary, struct_binarys>,
          fatal::pair<dep_A_struct, dep_A_structs>,
          fatal::pair<annotated, annotateds>,
          fatal::pair<struct_with_special_names, struct_with_special_namess>,
          fatal::pair<hasRefUnique, hasRefUnique_name>,
          fatal::pair<hasRefUniqueSimple, hasRefUniqueSimple_name>,
          fatal::pair<hasRefShared, hasRefShared_name>,
          fatal::pair<hasRefSharedSimple, hasRefSharedSimple_name>,
          fatal::pair<hasRefSharedConst, hasRefSharedConst_name>,
          fatal::pair<hasRefSharedConstSimple, hasRefSharedConstSimple_name>,
          fatal::pair<hasBox, hasBox_name>,
          fatal::pair<hasBoxSimple, hasBoxSimple_name>,
          fatal::pair<StructWithIOBuf, StructWithIOBufs>,
          fatal::pair<struct_with_renamed_field, struct_with_renamed_fields>,
          fatal::pair<IntStruct, IntStructs>,
          fatal::pair<StructWithAdaptedField, StructWithAdaptedFields>,
          fatal::pair<StructWithVectorBool, StructWithVectorBools>,
          fatal::pair<my_structA, my_structAs>>,

      info::structs>();

  EXPECT_SAME<
      fatal::list<service1s, service2s, service3s, service_with_special_namess>,
      info::services>();
}

} // namespace cpp_reflection
} // namespace test_cpp2

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_all.h>

namespace test_cpp2 {
namespace cpp_reflection {

TEST(fatal, reflect_module_tag) {
  using tag = reflection_tags::module;

  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<enum1>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<enum2>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<enum3>>();
  EXPECT_SAME<
      tag,
      apache::thrift::reflect_module_tag<enum_with_special_names>>();

  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<union1>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<union2>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<union3>>();
  EXPECT_SAME<
      tag,
      apache::thrift::reflect_module_tag<union_with_special_names>>();

  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<structA>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<structB>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<struct1>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<struct2>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<struct3>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<struct_binary>>();
  EXPECT_SAME<tag, apache::thrift::reflect_module_tag<annotated>>();
  EXPECT_SAME<
      tag,
      apache::thrift::reflect_module_tag<struct_with_special_names>>();
}

TEST(fatal, try_reflect_module_tag) {
  using tag = reflection_tags::module;

  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<enum1, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<enum2, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<enum3, void>>();
  EXPECT_SAME<
      tag,
      apache::thrift::try_reflect_module_tag<enum_with_special_names, void>>();

  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<union1, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<union2, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<union3, void>>();
  EXPECT_SAME<
      tag,
      apache::thrift::try_reflect_module_tag<union_with_special_names, void>>();

  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<structA, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<structB, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<struct1, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<struct2, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<struct3, void>>();
  EXPECT_SAME<
      tag,
      apache::thrift::try_reflect_module_tag<struct_binary, void>>();
  EXPECT_SAME<tag, apache::thrift::try_reflect_module_tag<annotated, void>>();
  EXPECT_SAME<
      tag,
      apache::thrift::
          try_reflect_module_tag<struct_with_special_names, void>>();

  EXPECT_SAME<void, apache::thrift::try_reflect_module_tag<int, void>>();
  EXPECT_SAME<void, apache::thrift::try_reflect_module_tag<void, void>>();
  EXPECT_SAME<
      void,
      apache::thrift::try_reflect_module_tag<std::string, void>>();
}

TEST(reflection, is_reflectable_module) {
  EXPECT_SAME<
      std::true_type,
      apache::thrift::is_reflectable_module<reflection_tags::module>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<enum1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<enum2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<enum3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<union1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<union2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<union3>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<struct1>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<struct2>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<struct3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<void>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_module<int>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::string>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::vector<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::vector<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::vector<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::set<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::set<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::unordered_set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::unordered_set<std::string>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::map<std::string, struct1>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<std::map<struct1, struct2>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<
          std::unordered_map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_module<
          std::unordered_map<std::string, struct1>>>();
}

TEST(reflection, is_reflectable_struct) {
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<reflection_tags::module>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<enum1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<enum2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<enum3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<union1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<union2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<union3>>();

  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_struct<struct1>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_struct<struct2>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_struct<struct3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<void>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_struct<int>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::string>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::vector<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::vector<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::vector<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::set<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::set<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::unordered_set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::unordered_set<std::string>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::map<std::string, struct1>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<std::map<struct1, struct2>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<
          std::unordered_map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_struct<
          std::unordered_map<std::string, struct1>>>();
}

TEST(reflection, is_reflectable_enum) {
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<reflection_tags::module>>();

  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_enum<enum1>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_enum<enum2>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_enum<enum3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<union1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<union2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<union3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<struct1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<struct2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<struct3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<void>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_enum<int>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::string>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::vector<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::vector<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::vector<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::set<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::set<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::unordered_set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::unordered_set<std::string>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::map<std::string, struct1>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<std::map<struct1, struct2>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<
          std::unordered_map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_enum<
          std::unordered_map<std::string, struct1>>>();
}

TEST(reflection, is_reflectable_union) {
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<reflection_tags::module>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<enum1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<enum2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<enum3>>();

  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_union<union1>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_union<union2>>();
  EXPECT_SAME<std::true_type, apache::thrift::is_reflectable_union<union3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<struct1>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<struct2>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<struct3>>();

  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<void>>();
  EXPECT_SAME<std::false_type, apache::thrift::is_reflectable_union<int>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::string>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::vector<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::vector<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::vector<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::set<std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::set<struct1>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::unordered_set<int>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::unordered_set<std::string>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::map<std::string, struct1>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<std::map<struct1, struct2>>>();

  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<
          std::unordered_map<int, std::string>>>();
  EXPECT_SAME<
      std::false_type,
      apache::thrift::is_reflectable_union<
          std::unordered_map<std::string, struct1>>>();
}

TEST(reflection, reflect_type_class_of_thrift_class) {
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class<
          reflection_tags::module>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class<void>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class<int32_t>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class<
          std::vector<int32_t>>>();

  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class<enum1>>();

  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class<union1>>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class<union2>>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class<union3>>();

  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class<struct1>>();
  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class<struct2>>();
  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class<struct3>>();
}

TEST(reflection, reflect_type_class_of_thrift_class_enum) {
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class_enum<
          reflection_tags::module>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class_enum<void>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class_enum<int32_t>>();
  EXPECT_SAME<
      apache::thrift::type_class::unknown,
      apache::thrift::reflect_type_class_of_thrift_class_enum<
          std::vector<int32_t>>>();

  EXPECT_SAME<
      apache::thrift::type_class::enumeration,
      apache::thrift::reflect_type_class_of_thrift_class_enum<enum1>>();

  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class_enum<union1>>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class_enum<union2>>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      apache::thrift::reflect_type_class_of_thrift_class_enum<union3>>();

  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class_enum<struct1>>();
  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class_enum<struct2>>();
  EXPECT_SAME<
      apache::thrift::type_class::structure,
      apache::thrift::reflect_type_class_of_thrift_class_enum<struct3>>();
}

TEST(reflection, reflected_module_name) {
  using module = test_cpp2::cpp_reflection::reflection_tags::module;
  using module_meta = apache::thrift::reflect_module<module>;
  const auto module_name = fatal::to_instance<std::string, module_meta::name>();
  EXPECT_EQ("reflection", module_name);
}

} // namespace cpp_reflection
} // namespace test_cpp2
