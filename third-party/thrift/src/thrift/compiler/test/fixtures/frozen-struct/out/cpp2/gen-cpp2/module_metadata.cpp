/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/frozen-struct/src/module.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated @nocommit
 */
#include <thrift/lib/cpp2/gen/module_metadata_cpp.h>
#include "thrift/compiler/test/fixtures/frozen-struct/gen-cpp2/module_metadata.h"

// some of these functions can be so large that the compiler gives up optimizing
// them - and issues a warning which may be treated as an error!
//
// these functions are so rarely called that it is probably okay for them not to
// be optimized in practice
FOLLY_CLANG_DISABLE_WARNING("-Wignored-optimization-argument")

namespace apache {
namespace thrift {
namespace detail {
namespace md {
using ThriftMetadata = ::apache::thrift::metadata::ThriftMetadata;
using ThriftPrimitiveType = ::apache::thrift::metadata::ThriftPrimitiveType;
using ThriftType = ::apache::thrift::metadata::ThriftType;
using ThriftService = ::apache::thrift::metadata::ThriftService;
using ThriftServiceContext = ::apache::thrift::metadata::ThriftServiceContext;
using ThriftFunctionGenerator = void (*)(ThriftMetadata&, ThriftService&);

void EnumMetadata<::some::ns::EnumB>::gen(ThriftMetadata& metadata) {
  auto res = metadata.enums()->emplace("module.EnumB", ::apache::thrift::metadata::ThriftEnum{});
  if (!res.second) {
    return;
  }
  ::apache::thrift::metadata::ThriftEnum& enum_metadata = res.first->second;
  enum_metadata.name() = "module.EnumB";
  using EnumTraits = TEnumTraits<::some::ns::EnumB>;
  for (std::size_t i = 0; i != EnumTraits::size; ++i) {
    enum_metadata.elements()->emplace(static_cast<int32_t>(EnumTraits::values[i]), EnumTraits::names[i]);
  }
}

const ::apache::thrift::metadata::ThriftStruct&
StructMetadata<::some::ns::ModuleA>::gen(ThriftMetadata& metadata) {
  auto res = metadata.structs()->emplace("module.ModuleA", ::apache::thrift::metadata::ThriftStruct{});
  if (!res.second) {
    return res.first->second;
  }
  ::apache::thrift::metadata::ThriftStruct& module_ModuleA = res.first->second;
  module_ModuleA.name() = "module.ModuleA";
  module_ModuleA.is_union() = false;
  static const auto* const
  module_ModuleA_fields = new std::array<EncodedThriftField, 6>{ {
    { 1, "i32Field", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ }},    { 2, "strField", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_STRING_TYPE), std::vector<ThriftConstStruct>{ }},    { 3, "listField", false, std::make_unique<List>(std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I16_TYPE)), std::vector<ThriftConstStruct>{ }},    { 4, "mapField", false, std::make_unique<Map>(std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_STRING_TYPE), std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE)), std::vector<ThriftConstStruct>{ }},    { 5, "inclAField", false, std::make_unique<Struct<::some::ns::IncludedA>>("include1.IncludedA"), std::vector<ThriftConstStruct>{ }},    { 6, "inclBField", false, std::make_unique<Struct<::some::ns::IncludedB>>("include2.IncludedB"), std::vector<ThriftConstStruct>{ }},  }};
  for (const auto& f : *module_ModuleA_fields) {
    ::apache::thrift::metadata::ThriftField field;
    field.id() = f.id;
    field.name() = f.name;
    field.is_optional() = f.is_optional;
    f.metadata_type_interface->writeAndGenType(*field.type(), metadata);
    field.structured_annotations() = f.structured_annotations;
    module_ModuleA.fields()->push_back(std::move(field));
  }
  return res.first->second;
}
const ::apache::thrift::metadata::ThriftStruct&
StructMetadata<::some::ns::ModuleB>::gen(ThriftMetadata& metadata) {
  auto res = metadata.structs()->emplace("module.ModuleB", ::apache::thrift::metadata::ThriftStruct{});
  if (!res.second) {
    return res.first->second;
  }
  ::apache::thrift::metadata::ThriftStruct& module_ModuleB = res.first->second;
  module_ModuleB.name() = "module.ModuleB";
  module_ModuleB.is_union() = false;
  static const auto* const
  module_ModuleB_fields = new std::array<EncodedThriftField, 2>{ {
    { 1, "i32Field", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ }},    { 2, "inclEnumB", false, std::make_unique<Enum<::some::ns::EnumB>>("module.EnumB"), std::vector<ThriftConstStruct>{ }},  }};
  for (const auto& f : *module_ModuleB_fields) {
    ::apache::thrift::metadata::ThriftField field;
    field.id() = f.id;
    field.name() = f.name;
    field.is_optional() = f.is_optional;
    f.metadata_type_interface->writeAndGenType(*field.type(), metadata);
    field.structured_annotations() = f.structured_annotations;
    module_ModuleB.fields()->push_back(std::move(field));
  }
  return res.first->second;
}
const ::apache::thrift::metadata::ThriftStruct&
StructMetadata<::some::ns::detail::DirectlyAdapted>::gen(ThriftMetadata& metadata) {
  auto res = metadata.structs()->emplace("module.DirectlyAdapted", ::apache::thrift::metadata::ThriftStruct{});
  if (!res.second) {
    return res.first->second;
  }
  ::apache::thrift::metadata::ThriftStruct& module_DirectlyAdapted = res.first->second;
  module_DirectlyAdapted.name() = "module.DirectlyAdapted";
  module_DirectlyAdapted.is_union() = false;
  static const auto* const
  module_DirectlyAdapted_fields = new std::array<EncodedThriftField, 1>{ {
    { 1, "field", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ }},  }};
  for (const auto& f : *module_DirectlyAdapted_fields) {
    ::apache::thrift::metadata::ThriftField field;
    field.id() = f.id;
    field.name() = f.name;
    field.is_optional() = f.is_optional;
    f.metadata_type_interface->writeAndGenType(*field.type(), metadata);
    field.structured_annotations() = f.structured_annotations;
    module_DirectlyAdapted.fields()->push_back(std::move(field));
  }
  module_DirectlyAdapted.structured_annotations()->push_back(*cvStruct("cpp.Adapter", { {"name", cvString("::my::Adapter") } }).cv_struct_ref());
  return res.first->second;
}
const ::apache::thrift::metadata::ThriftStruct&
StructMetadata<::some::ns::CppRef>::gen(ThriftMetadata& metadata) {
  auto res = metadata.structs()->emplace("module.CppRef", ::apache::thrift::metadata::ThriftStruct{});
  if (!res.second) {
    return res.first->second;
  }
  ::apache::thrift::metadata::ThriftStruct& module_CppRef = res.first->second;
  module_CppRef.name() = "module.CppRef";
  module_CppRef.is_union() = false;
  static const auto* const
  module_CppRef_fields = new std::array<EncodedThriftField, 5>{ {
    { 1, "shared_field", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ *cvStruct("cpp.Ref", { {"type", cvInteger(2) } }).cv_struct_ref(), *cvStruct("cpp.AllowLegacyNonOptionalRef", {  }).cv_struct_ref(), }},    { 2, "shared_const_field", false, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ *cvStruct("cpp.Ref", { {"type", cvInteger(1) } }).cv_struct_ref(), *cvStruct("cpp.AllowLegacyNonOptionalRef", {  }).cv_struct_ref(), }},    { 3, "opt_shared_field", true, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ *cvStruct("cpp.Ref", { {"type", cvInteger(2) } }).cv_struct_ref(), }},    { 4, "opt_shared_const_field", true, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ *cvStruct("cpp.Ref", { {"type", cvInteger(1) } }).cv_struct_ref(), }},    { 5, "boxed_field", true, std::make_unique<Primitive>(ThriftPrimitiveType::THRIFT_I32_TYPE), std::vector<ThriftConstStruct>{ *cvStruct("thrift.Box", {  }).cv_struct_ref(), }},  }};
  for (const auto& f : *module_CppRef_fields) {
    ::apache::thrift::metadata::ThriftField field;
    field.id() = f.id;
    field.name() = f.name;
    field.is_optional() = f.is_optional;
    f.metadata_type_interface->writeAndGenType(*field.type(), metadata);
    field.structured_annotations() = f.structured_annotations;
    module_CppRef.fields()->push_back(std::move(field));
  }
  return res.first->second;
}

} // namespace md
} // namespace detail
} // namespace thrift
} // namespace apache
