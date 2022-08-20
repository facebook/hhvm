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

#include <thrift/lib/cpp2/security/extensions/Types.h>

#include <folly/Optional.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

fizz::Extension encodeThriftExtension(const ThriftParametersExt& params) {
  fizz::Extension ext;
  ext.extension_type = fizz::ExtensionType::thrift_parameters;

  CompactProtocolWriter compactProtocolWriter;
  folly::IOBufQueue queue;
  compactProtocolWriter.setOutput(&queue);
  params.params.write(&compactProtocolWriter);
  ext.extension_data = queue.move();
  return ext;
}

folly::Optional<ThriftParametersExt> getThriftExtension(
    const std::vector<fizz::Extension>& extensions) {
  auto it = findExtension(extensions, fizz::ExtensionType::thrift_parameters);
  if (it == extensions.end()) {
    return folly::none;
  }

  CompactProtocolReader reader;
  reader.setInput(it->extension_data.get());
  ThriftParametersExt params;
  params.params.read(&reader);
  return params;
}
} // namespace thrift
} // namespace apache
