/**
 * Autogenerated by Thrift for thrift/compiler/test/fixtures/terse_write/src/terse_write.thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include "thrift/compiler/test/fixtures/terse_write/gen-py3/terse_write/metadata.h"

namespace facebook {
namespace thrift {
namespace test {
namespace terse_write {
::apache::thrift::metadata::ThriftMetadata terse_write_getThriftModuleMetadata() {
  ::apache::thrift::metadata::ThriftServiceMetadataResponse response;
  ::apache::thrift::metadata::ThriftMetadata& metadata = *response.metadata_ref();
  ::apache::thrift::detail::md::EnumMetadata<MyEnum>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<MyStruct>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<MyUnion>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<MyStructWithCustomDefault>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<StructLevelTerseStruct>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<FieldLevelTerseStruct>::gen(metadata);
  ::apache::thrift::detail::md::StructMetadata<AdaptedFields>::gen(metadata);
  ::apache::thrift::detail::md::ExceptionMetadata<TerseException>::gen(metadata);
  return metadata;
}
} // namespace facebook
} // namespace thrift
} // namespace test
} // namespace terse_write
