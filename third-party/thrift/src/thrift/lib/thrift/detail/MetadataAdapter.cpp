// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "thrift/lib/thrift/detail/MetadataAdapter.h"
#include "thrift/lib/thrift/gen-cpp2/metadata_types.h"

namespace apache::thrift::metadata {

const ThriftConstStruct* findStructuredAnnotation(
    const std::vector<ThriftConstStruct>& annotations, std::string_view name) {
  for (const auto& i : annotations) {
    if (i.type()->name() == name) {
      return &i;
    }
  }
  return nullptr;
}
const ThriftConstStruct& findStructuredAnnotationOrThrow(
    const std::vector<ThriftConstStruct>& annotations, std::string_view name) {
  if (const auto* p = findStructuredAnnotation(annotations, name)) {
    return *p;
  }

  throw std::out_of_range("Can not find annotation: " + std::string(name));
}

} // namespace apache::thrift::metadata
