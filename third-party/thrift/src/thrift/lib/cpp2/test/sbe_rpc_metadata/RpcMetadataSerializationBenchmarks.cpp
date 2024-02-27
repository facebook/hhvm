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

// build in @mode/dbg to enable SBE Checks
#include <iterator>
#if __SBE_DEBUG_MODE == 1
#define SBE_ENABLE_PRECEDENCE_CHECKS
#else
#define SBE_NO_BOUNDS_CHECK
#endif

#include <iostream>
#include <memory>
#include <string>
#include <folly/Benchmark.h>
#include <folly/FixedString.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/cpp2/test/sbe_rpc_metadata/gen-cpp2/wrapper_types.h>
#include <thrift/lib/thrift/apache_thrift_sbe/RequestRpcMetadata.h>
#include <thrift/lib/thrift/apache_thrift_sbe/ResponseRpcMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::benchmark {

constexpr auto kSerfAppHeader = folly::makeFixedString("X-Serf-App");
constexpr auto kSerfAppPathHeader = folly::makeFixedString("X-Serf-AppPath");
constexpr auto kSerfAppArgsHeader = folly::makeFixedString("X-Serf-AppArgs");
constexpr auto kSerfAppAPIHeader = folly::makeFixedString("X-Serf-API");
constexpr auto kSerfOnBehalfOf = folly::makeFixedString("X-Serf-On-Behalf-Of");

const auto kEmptyString = std::string("");

auto thriftIOBufQueue =
    folly::IOBufQueue{folly::IOBufQueue::cacheChainLength()};

template <class ProtocolWriter, class T>
void serialize_with_protocol_writer(T&& t) {
  ProtocolWriter writer;
  writer.setOutput(&thriftIOBufQueue);
  std::forward<T>(t).write(&writer);
}

template <class T, class ProtocolReader>
T deserialize_with_protocol_reader(const std::unique_ptr<folly::IOBuf>& iobuf) {
  T t;
  ProtocolReader reader;
  reader.setInput(folly::io::Cursor(iobuf.get()));
  t.read(&reader);
  return t;
}

const std::string name = "test_name";
const std::string_view nameView = std::string_view(name);

std::unique_ptr<folly::IOBuf> baseRequestMetadata;
std::unique_ptr<folly::IOBuf> baseResponseMetadata;
std::unique_ptr<folly::IOBuf> requestMetadataHeaders;
std::unique_ptr<folly::IOBuf> responseMetadataHeaders;
std::unique_ptr<folly::IOBuf> messageBuffer;
std::unique_ptr<folly::IOBuf> tcompact_baseRequestMetadata;
std::unique_ptr<folly::IOBuf> tcompact_baseResponseMetadata;
std::unique_ptr<folly::IOBuf> tcompact_requestMetadataHeaders;
std::unique_ptr<folly::IOBuf> tcompact_responseMetadataHeaders;

void setup() {
  auto requestMetadata =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  auto responseMetadata =
      sbe::MessageWrapper<sbe::ResponseRpcMetadata, sbe::MessageHeader>();
  {
    baseRequestMetadata = folly::IOBuf::create(64);
    requestMetadata.wrapForEncode(*baseRequestMetadata);
    requestMetadata->protocol(sbe::ProtocolId::COMPACT);
    requestMetadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
    requestMetadata->otherMetadataCount(0);
    requestMetadata->putName(name);
    requestMetadata->putInteractionMetadata(kEmptyString);
    requestMetadata->putOptionalMetdata(kEmptyString);
    requestMetadata.completeEncoding(*baseRequestMetadata);
  }
  {
    baseResponseMetadata = folly::IOBuf::create(64);
    responseMetadata.wrapForEncode(*baseResponseMetadata);
    responseMetadata->streamId(100);
    responseMetadata->otherMetadataCount(0);
    responseMetadata->putExceptionMetadata(kEmptyString);
    responseMetadata->putOptionalMetadata(kEmptyString);
    responseMetadata.completeEncoding(*baseResponseMetadata);
  }
  {
    requestMetadataHeaders = folly::IOBuf::create(256);
    requestMetadata.wrapForEncode(*requestMetadataHeaders);
    requestMetadata->protocol(sbe::ProtocolId::COMPACT);
    requestMetadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
    auto om = requestMetadata->otherMetadataCount(5);

    om.next();
    om.putOtherMetadataKey(kSerfAppHeader.c_str(), kSerfAppHeader.size());
    om.putOtherMetadataValue(std::string("test_value"));

    om.next();
    om.putOtherMetadataKey(
        kSerfAppPathHeader.c_str(), kSerfAppPathHeader.size());
    om.putOtherMetadataValue(std::string("test_app_path_header"));

    om.next();
    om.putOtherMetadataKey(
        kSerfAppArgsHeader.c_str(), kSerfAppArgsHeader.size());
    om.putOtherMetadataValue(std::string("test_app"));

    om.next();
    om.putOtherMetadataKey(kSerfAppAPIHeader.c_str(), kSerfAppAPIHeader.size());
    om.putOtherMetadataValue(std::string("test_api"));

    om.next();
    om.putOtherMetadataKey(kSerfOnBehalfOf.c_str(), kSerfOnBehalfOf.size());
    om.putOtherMetadataValue(std::string("12345"));

    requestMetadata->putName(name);
    requestMetadata->putInteractionMetadata(kEmptyString);
    requestMetadata->putOptionalMetdata(kEmptyString);
    requestMetadata.completeEncoding(*requestMetadataHeaders);
  }
  {
    responseMetadataHeaders = folly::IOBuf::create(256);
    responseMetadata.wrapForEncode(*responseMetadataHeaders);
    responseMetadata->streamId(100);
    auto om = responseMetadata->otherMetadataCount(5);

    om.next();
    om.putOtherMetadataKey(kSerfAppHeader.c_str(), kSerfAppHeader.size());
    om.putOtherMetadataValue(std::string("test_value"));

    om.next();
    om.putOtherMetadataKey(
        kSerfAppPathHeader.c_str(), kSerfAppPathHeader.size());
    om.putOtherMetadataValue(std::string("test_app_path_header"));

    om.next();
    om.putOtherMetadataKey(
        kSerfAppArgsHeader.c_str(), kSerfAppArgsHeader.size());
    om.putOtherMetadataValue(std::string("test_app"));

    om.next();
    om.putOtherMetadataKey(kSerfAppAPIHeader.c_str(), kSerfAppAPIHeader.size());
    om.putOtherMetadataValue(std::string("test_api"));

    om.next();
    om.putOtherMetadataKey(kSerfOnBehalfOf.c_str(), kSerfOnBehalfOf.size());
    om.putOtherMetadataValue(std::string("12345"));

    responseMetadata->putExceptionMetadata(kEmptyString);
    responseMetadata->putOptionalMetadata(kEmptyString);
    responseMetadata.completeEncoding(*responseMetadataHeaders);
  }
  messageBuffer = folly::IOBuf::create(256);

  {
    ManagedStringView managedName = ManagedStringView::from_static(nameView);
    apache::thrift::RequestRpcMetadata request;
    request.protocol() = apache::thrift::ProtocolId::COMPACT;
    request.name() = ManagedStringViewWithConversions(std::move(managedName));
    request.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
    serialize_with_protocol_writer<CompactProtocolWriter>(request);
    tcompact_baseRequestMetadata = thriftIOBufQueue.move();
  }

  {
    apache::thrift::ResponseRpcMetadata response;
    response.streamId() = 100;
    serialize_with_protocol_writer<CompactProtocolWriter>(response);
    tcompact_baseResponseMetadata = thriftIOBufQueue.move();
  }

  {
    ManagedStringView managedName = ManagedStringView::from_static(nameView);
    apache::thrift::RequestRpcMetadata request;
    request.protocol() = apache::thrift::ProtocolId::COMPACT;
    request.name() = ManagedStringViewWithConversions(std::move(managedName));
    request.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
    request.otherMetadata().ensure();
    auto& otherMetadata = can_throw(*request.otherMetadata());
    otherMetadata[kSerfAppHeader] = "test_app";
    otherMetadata[kSerfAppPathHeader] = "test_app_path_header";
    otherMetadata[kSerfAppArgsHeader] = "test_app";
    otherMetadata[kSerfAppAPIHeader] = "test_api";
    otherMetadata[kSerfOnBehalfOf] = "12345";
    serialize_with_protocol_writer<CompactProtocolWriter>(request);
    tcompact_requestMetadataHeaders = thriftIOBufQueue.move();
  }

  {
    apache::thrift::ResponseRpcMetadata response;
    response.streamId() = 100;
    response.otherMetadata().ensure();
    auto& otherMetadata = can_throw(*response.otherMetadata());
    otherMetadata[kSerfAppHeader] = "test_app";
    otherMetadata[kSerfAppPathHeader] = "test_app_path_header";
    otherMetadata[kSerfAppArgsHeader] = "test_app";
    otherMetadata[kSerfAppAPIHeader] = "test_api";
    otherMetadata[kSerfOnBehalfOf] = "12345";
    serialize_with_protocol_writer<CompactProtocolWriter>(response);
    tcompact_responseMetadataHeaders = thriftIOBufQueue.move();
  }

  thriftIOBufQueue.preallocate(1024, 1024);
}

void do_TCompactRpcMetadataRequestSerializationBaseline() {
  ManagedStringView managedName = ManagedStringView::from_static(nameView);
  apache::thrift::RequestRpcMetadata request;
  request.protocol() = apache::thrift::ProtocolId::COMPACT;
  request.name() = ManagedStringViewWithConversions(std::move(managedName));
  request.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  serialize_with_protocol_writer<CompactProtocolWriter>(request);
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK(TCompactRpcMetadataRequestSerializationBaseline) {
  do_TCompactRpcMetadataRequestSerializationBaseline();
}

void do_SBERpcMetadataRequestSerializationBaseline() {
  messageBuffer->clear();
  auto metadata =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  metadata.wrapForEncode(*messageBuffer);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  metadata->otherMetadataCount(0);
  metadata->putName(name);
  metadata->putInteractionMetadata(kEmptyString);
  metadata->putOptionalMetdata(kEmptyString);
  metadata.completeEncoding(*messageBuffer);
}

BENCHMARK_RELATIVE(SBERpcMetadataRequestSerializationBaseline) {
  do_SBERpcMetadataRequestSerializationBaseline();
}

void do_TCompactRpcMetadataRequestDeserBaseline() {
  deserialize_with_protocol_reader<
      apache::thrift::RequestRpcMetadata,
      CompactProtocolReader>(tcompact_baseRequestMetadata);
}

BENCHMARK(TCompactRpcMetadataRequestDeserBaseline) {
  do_TCompactRpcMetadataRequestDeserBaseline();
}

void do_SBERpcMetadataRequestDeserBaseline() {
  auto other =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  other.wrapForDecode(*baseRequestMetadata);
  folly::doNotOptimizeAway(other->protocol());
  folly::doNotOptimizeAway(other->kind());

#if __SBE_DEBUG_MODE == 1
  auto n = other->otherMetadata().hasNext();
  assert(n == false);
#else
  folly::doNotOptimizeAway(other->otherMetadata().hasNext());
#endif

  folly::doNotOptimizeAway(other->getNameAsStringView());
  folly::doNotOptimizeAway(other->getInteractionMetadataAsStringView());
  folly::doNotOptimizeAway(other->getOptionalMetdataAsStringView());
}

BENCHMARK_RELATIVE(SBERpcMetadataRequestDeserBaseline) {
  do_SBERpcMetadataRequestDeserBaseline();
}

void do_TCompactRpcMetadataResponseSerializationBaseline() {
  apache::thrift::ResponseRpcMetadata response;
  response.streamId() = 100;
  serialize_with_protocol_writer<CompactProtocolWriter>(response);
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK(TCompactRpcMetadataResponseSerializationBaseline) {
  do_TCompactRpcMetadataResponseSerializationBaseline();
}

void do_SBERpcMetadataResponseSerializationBaseline() {
  messageBuffer->clear();
  auto metadata =
      sbe::MessageWrapper<sbe::ResponseRpcMetadata, sbe::MessageHeader>();
  metadata.wrapForEncode(*messageBuffer);
  metadata->streamId(100);
  metadata->otherMetadataCount(0);
  metadata->putExceptionMetadata(kEmptyString);
  metadata->putOptionalMetadata(kEmptyString);
#if __SBE_DEBUG_MODE == 1
  metadata.completeEncoding();
#endif
}

BENCHMARK_RELATIVE(SBERpcMetadataResponseSerializationBaseline) {
  do_SBERpcMetadataResponseSerializationBaseline();
}

void do_TCompactRpcMetadataResponseDeserBaseline() {
  deserialize_with_protocol_reader<
      apache::thrift::ResponseRpcMetadata,
      CompactProtocolReader>(tcompact_baseResponseMetadata);
}

BENCHMARK(TCompactRpcMetadataResponseDeserBaseline) {
  do_TCompactRpcMetadataResponseDeserBaseline();
}

void do_SBERpcMetadataResponseDeserBaseline() {
  auto other =
      sbe::MessageWrapper<sbe::ResponseRpcMetadata, sbe::MessageHeader>();
  other.wrapForDecode(*baseResponseMetadata);
  folly::doNotOptimizeAway(other->streamId());

#if __SBE_DEBUG_MODE == 1
  auto n = other->otherMetadata().hasNext();
  assert(n == false);
#else
  folly::doNotOptimizeAway(other->otherMetadata().hasNext());
#endif

  folly::doNotOptimizeAway(other->skipExceptionMetadata());
  folly::doNotOptimizeAway(other->skipOptionalMetadata());
}

BENCHMARK_RELATIVE(SBERpcMetadataResponseDeserBaseline) {
  do_SBERpcMetadataResponseDeserBaseline();
}

BENCHMARK(TCompactClientRoundTripBaseline) {
  do_TCompactRpcMetadataRequestSerializationBaseline();
  do_TCompactRpcMetadataResponseDeserBaseline();
}

BENCHMARK_RELATIVE(SBEClientRoundTripBaseline) {
  do_SBERpcMetadataRequestSerializationBaseline();
  do_SBERpcMetadataResponseDeserBaseline();
}

BENCHMARK(TCompactServerRoundTripBaseline) {
  do_TCompactRpcMetadataRequestDeserBaseline();
  do_TCompactRpcMetadataResponseSerializationBaseline();
}

BENCHMARK_RELATIVE(SBEServerRoundTripBaseline) {
  do_SBERpcMetadataRequestDeserBaseline();
  do_SBERpcMetadataResponseSerializationBaseline();
}

void do_TCompactRpcMetadataRequestSerializationWithHeaders() {
  ManagedStringView managedName = ManagedStringView::from_static(nameView);
  apache::thrift::RequestRpcMetadata request;
  request.protocol() = apache::thrift::ProtocolId::COMPACT;
  request.name() = ManagedStringViewWithConversions(std::move(managedName));
  request.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  request.otherMetadata().ensure();
  auto& otherMetadata = can_throw(*request.otherMetadata());
  otherMetadata[kSerfAppHeader] = "test_app";
  otherMetadata[kSerfAppPathHeader] = "test_app_path_header";
  otherMetadata[kSerfAppArgsHeader] = "test_app";
  otherMetadata[kSerfAppAPIHeader] = "test_api";
  otherMetadata[kSerfOnBehalfOf] = "12345";
  serialize_with_protocol_writer<CompactProtocolWriter>(request);
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK(TCompactRpcMetadataRequestSerializationHeaders) {
  do_TCompactRpcMetadataRequestSerializationWithHeaders();
}

void do_SBERpcMetadataRequestSerializationWithHeaders() {
  messageBuffer->clear();
  auto metadata =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  metadata.wrapForEncode(*messageBuffer);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  auto om = metadata->otherMetadataCount(5);

  om.next();
  om.putOtherMetadataKey(kSerfAppHeader.c_str(), kSerfAppHeader.size());
  om.putOtherMetadataValue(std::string("test_value"));

  om.next();
  om.putOtherMetadataKey(kSerfAppPathHeader.c_str(), kSerfAppPathHeader.size());
  om.putOtherMetadataValue(std::string("test_app_path_header"));

  om.next();
  om.putOtherMetadataKey(kSerfAppArgsHeader.c_str(), kSerfAppArgsHeader.size());
  om.putOtherMetadataValue(std::string("test_app"));

  om.next();
  om.putOtherMetadataKey(kSerfAppAPIHeader.c_str(), kSerfAppAPIHeader.size());
  om.putOtherMetadataValue(std::string("test_api"));

  om.next();
  om.putOtherMetadataKey(kSerfOnBehalfOf.c_str(), kSerfOnBehalfOf.size());
  om.putOtherMetadataValue(std::string("12345"));

  metadata->putName(name);
  metadata->putInteractionMetadata(kEmptyString);
  metadata->putOptionalMetdata(kEmptyString);
#if __SBE_DEBUG_MODE == 1
  metadata.completeEncoding();
#endif
}

BENCHMARK_RELATIVE(SBERpcMetadataRequestSerializationWithHeaders) {
  do_SBERpcMetadataRequestSerializationWithHeaders();
}

void do_TCompactRpcMetadataRequestDeserializationWithHeaders() {
  deserialize_with_protocol_reader<
      apache::thrift::RequestRpcMetadata,
      CompactProtocolReader>(tcompact_requestMetadataHeaders);
}

BENCHMARK(TCompactRpcMetadataRequestDeserializationHeaders) {
  do_TCompactRpcMetadataRequestDeserializationWithHeaders();
}

void do_SBERpcMetadataRequestDeserializationWithHeaders() {
  auto other =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  other.wrapForDecode(*requestMetadataHeaders);
  folly::doNotOptimizeAway(other->protocol());
  folly::doNotOptimizeAway(other->kind());

  auto om = other->otherMetadata();
  while (om.hasNext()) {
    om.next();
    folly::doNotOptimizeAway(om.getOtherMetadataKeyAsStringView());
    folly::doNotOptimizeAway(om.getOtherMetadataValueAsStringView());
  }

  folly::doNotOptimizeAway(other->getNameAsStringView());
  folly::doNotOptimizeAway(other->getInteractionMetadataAsStringView());
  folly::doNotOptimizeAway(other->getOptionalMetdataAsStringView());
}

BENCHMARK_RELATIVE(SBERpcMetadataRequestDeserializationWithHeaders) {
  do_SBERpcMetadataRequestDeserializationWithHeaders();
}

void do_TCompactRpcMetadataResponseSerializationHeaders() {
  apache::thrift::ResponseRpcMetadata response;
  response.streamId() = 100;
  response.otherMetadata().ensure();
  auto& otherMetadata = can_throw(*response.otherMetadata());
  otherMetadata[kSerfAppHeader] = "test_app";
  otherMetadata[kSerfAppPathHeader] = "test_app_path_header";
  otherMetadata[kSerfAppArgsHeader] = "test_app";
  otherMetadata[kSerfAppAPIHeader] = "test_api";
  otherMetadata[kSerfOnBehalfOf] = "12345";
  serialize_with_protocol_writer<CompactProtocolWriter>(response);
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK(TCompactRpcMetadataResponseSerializationHeaders) {
  do_TCompactRpcMetadataResponseSerializationHeaders();
}

void do_SBERpcMetadataResponseSerializationHeaders() {
  folly::BenchmarkSuspender susp;
  messageBuffer->clear();
  susp.dismiss();
  auto metadata =
      sbe::MessageWrapper<sbe::ResponseRpcMetadata, sbe::MessageHeader>();
  metadata.wrapForEncode(*messageBuffer);

  metadata->streamId(100);
  auto om = metadata->otherMetadataCount(5);

  om.next();
  om.putOtherMetadataKey(kSerfAppHeader.c_str(), kSerfAppHeader.size());
  om.putOtherMetadataValue(std::string("test_value"));

  om.next();
  om.putOtherMetadataKey(kSerfAppPathHeader.c_str(), kSerfAppPathHeader.size());
  om.putOtherMetadataValue(std::string("test_app_path_header"));

  om.next();
  om.putOtherMetadataKey(kSerfAppArgsHeader.c_str(), kSerfAppArgsHeader.size());
  om.putOtherMetadataValue(std::string("test_app"));

  om.next();
  om.putOtherMetadataKey(kSerfAppAPIHeader.c_str(), kSerfAppAPIHeader.size());
  om.putOtherMetadataValue(std::string("test_api"));

  om.next();
  om.putOtherMetadataKey(kSerfOnBehalfOf.c_str(), kSerfOnBehalfOf.size());
  om.putOtherMetadataValue(std::string("12345"));
  metadata->putExceptionMetadata(kEmptyString);
  metadata->putOptionalMetadata(kEmptyString);
#if __SBE_DEBUG_MODE == 1
  metadata.completeEncoding();
#endif
}

BENCHMARK_RELATIVE(SBERpcMetadataResponseSerializationHeaders) {
  do_SBERpcMetadataResponseSerializationHeaders();
}

void do_TCompactRpcMetadataResponseDeserializationHeaders() {
  deserialize_with_protocol_reader<
      apache::thrift::ResponseRpcMetadata,
      CompactProtocolReader>(tcompact_responseMetadataHeaders);
}

BENCHMARK(TCompactRpcMetadataResponseDeserializationHeaders) {
  do_TCompactRpcMetadataResponseDeserializationHeaders();
}

void do_SBERpcMetadataResponseDeserializationHeaders() {
  auto other =
      sbe::MessageWrapper<sbe::ResponseRpcMetadata, sbe::MessageHeader>();
  other.wrapForDecode(*responseMetadataHeaders);

  folly::doNotOptimizeAway(other->streamId());

  auto om = other->otherMetadata();
  while (om.hasNext()) {
    om.next();
    folly::doNotOptimizeAway(om.getOtherMetadataKeyAsStringView());
    folly::doNotOptimizeAway(om.getOtherMetadataValueAsStringView());
  }

  folly::doNotOptimizeAway(other->getExceptionMetadataAsStringView());
  folly::doNotOptimizeAway(other->getOptionalMetadataAsStringView());
}

BENCHMARK_RELATIVE(SBERpcMetadataResponseDeserializationHeaders) {
  do_SBERpcMetadataResponseDeserializationHeaders();
}

BENCHMARK(TCompactClientRoundTripHeader) {
  do_TCompactRpcMetadataRequestSerializationWithHeaders();
  do_TCompactRpcMetadataResponseDeserializationHeaders();
}

BENCHMARK_RELATIVE(SBEClientRoundTripHeader) {
  do_SBERpcMetadataRequestSerializationWithHeaders();
  do_SBERpcMetadataResponseDeserializationHeaders();
}

BENCHMARK(TCompactServerRoundTripHeader) {
  do_TCompactRpcMetadataRequestDeserializationWithHeaders();
  do_TCompactRpcMetadataResponseSerializationHeaders();
}

BENCHMARK_RELATIVE(SBEServerRoundTripHeader) {
  do_SBERpcMetadataRequestDeserializationWithHeaders();
  do_SBERpcMetadataResponseSerializationHeaders();
}

BENCHMARK_DRAW_LINE();

BENCHMARK(TestWrapping_SbeBaseline) {
  do_SBERpcMetadataRequestSerializationBaseline();
}

BENCHMARK(TestWrapping_SbeWithAllocation) {
  auto buffer = folly::IOBuf::create(256);
  auto metadata =
      sbe::MessageWrapper<sbe::RequestRpcMetadata, sbe::MessageHeader>();
  metadata.wrapForEncode(*messageBuffer);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  metadata->otherMetadataCount(0);
  metadata->putName(name);
  metadata->putInteractionMetadata(kEmptyString);
  metadata->putOptionalMetdata(kEmptyString);
  metadata.completeEncoding(*buffer);
}

BENCHMARK_RELATIVE(TestWrapping_TCompactBaseline) {
  do_TCompactRpcMetadataRequestSerializationBaseline();
}

BENCHMARK_RELATIVE(TestWrapping_CloneOne) {
  do_SBERpcMetadataRequestSerializationBaseline();
  messageBuffer->cloneOne();
}

BENCHMARK_RELATIVE(TestWrapping_WrapNoSerialize) {
  do_SBERpcMetadataRequestSerializationBaseline();
  benchmarks::Wrapper wrapper{};
  wrapper.data() = messageBuffer->cloneOne();
}

BENCHMARK_RELATIVE(TestWrapping_CompactWriter) {
  CompactProtocolWriter writer;
  writer.setOutput(&thriftIOBufQueue);
  writer.writeBinary(baseRequestMetadata->cloneOne());
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK_RELATIVE(TestWrapping_BinaryWriter) {
  BinaryProtocolWriter writer;
  writer.setOutput(&thriftIOBufQueue);
  writer.writeBinary(baseRequestMetadata->cloneOne());
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

template <typename ProtocolWriter>
void do_ThriftWrapperSerialize() {
  do_SBERpcMetadataRequestSerializationBaseline();
  benchmarks::Wrapper wrapper{};
  wrapper.data() = messageBuffer->cloneOne();
  serialize_with_protocol_writer<ProtocolWriter>(wrapper);
  thriftIOBufQueue.clearAndTryReuseLargestBuffer();
}

BENCHMARK_RELATIVE(TestWrapping_TCompact) {
  do_ThriftWrapperSerialize<CompactProtocolWriter>();
}

BENCHMARK_RELATIVE(TestWrapping_TBinary) {
  do_ThriftWrapperSerialize<BinaryProtocolWriter>();
}

} // namespace apache::thrift::benchmark

int main(int argc, char** argv) {
#if __SBE_DEBUG_MODE == 1
  std::cout
      << "Benchmark built in SBE Debug Mode. This means predence, and bounds checking are enabled."
      << std::endl;
#else
  std::cout
      << "Benchmark built in SBE Release Mode. This means predence, and bounds checking are *disabled*."
      << std::endl;
#endif

  apache::thrift::benchmark::setup();
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
