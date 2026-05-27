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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/MetadataAppAdapter.h>

#include <utility>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

// Hand-rolled mirror of the codegen-emitted
// ThriftMetadataService_getThriftServiceMetadata_presult — defining it
// inline avoids depending on the heavyweight gen-cpp2/ThriftMetadataService.tcc
// (which would pull in the legacy AsyncProcessor surface).
using GetMetadataPresult = apache::thrift::ThriftPresult<
    /*hasIsSet=*/true,
    apache::thrift::FieldData<
        /*fieldId=*/0,
        ::apache::thrift::type_class::structure,
        apache::thrift::metadata::ThriftServiceMetadataResponse*>>;

constexpr std::string_view kMethodName = "getThriftServiceMetadata";

} // namespace

MetadataAppAdapter::MetadataAppAdapter(
    std::shared_ptr<
        const apache::thrift::metadata::ThriftServiceMetadataResponse> response)
    : response_(std::move(response)) {
  CHECK(response_) << "MetadataAppAdapter requires a non-null response";
  addMethodHandler(
      kMethodName, &MetadataAppAdapter::handleGetThriftServiceMetadata);
}

template <typename Writer>
void MetadataAppAdapter::writeMetadataResponse(uint32_t streamId) noexcept {
  GetMetadataPresult presult;
  // Presult holds a pointer to the response; const_cast is safe — write() is
  // logically const, the codegen surface just isn't const-correct.
  presult.get<0>().value =
      const_cast<apache::thrift::metadata::ThriftServiceMetadataResponse*>(
          response_.get());
  presult.setIsSet(0, true);
  writeResponse(makeSuccessResponseMessage<Writer>(streamId, presult));
}

void MetadataAppAdapter::handleGetThriftServiceMetadata(
    ThriftServerAppAdapter* self,
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> /*requestData*/,
    apache::thrift::ProtocolId protocol,
    std::unique_ptr<ThriftRequestContext> /*requestContext*/) noexcept {
  auto* impl = static_cast<MetadataAppAdapter*>(self);
  switch (protocol) {
    case apache::thrift::ProtocolId::COMPACT:
      impl->writeMetadataResponse<apache::thrift::CompactProtocolWriter>(
          streamId);
      break;
    case apache::thrift::ProtocolId::BINARY:
      impl->writeMetadataResponse<apache::thrift::BinaryProtocolWriter>(
          streamId);
      break;
    default:
      impl->writeResponse(makeFrameworkErrorMessage(
          streamId,
          apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
          "Unsupported protocol id for getThriftServiceMetadata"));
      break;
  }
}

} // namespace apache::thrift::fast_thrift::thrift
