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

// SBE requires that fields are serialized and deserialized in the order in the
// SBE XML file. This flag is used to  ensure that the order is correct. If the
// order is incorrect, the SBE with throw an exception. You should only use this
// flag when you are testing.
#define SBE_ENABLE_PRECEDENCE_CHECKS

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/thrift/apache_thrift_sbe/CompressionAlgorithm.h>
#include <thrift/lib/thrift/apache_thrift_sbe/CompressionConfig.h>
#include <thrift/lib/thrift/apache_thrift_sbe/FdMetadata.h>
#include <thrift/lib/thrift/apache_thrift_sbe/InteractionCreate.h>
#include <thrift/lib/thrift/apache_thrift_sbe/InteractionRequest.h>
#include <thrift/lib/thrift/apache_thrift_sbe/InterationTerminate.h>
#include <thrift/lib/thrift/apache_thrift_sbe/MessageHeader.h>
#include <thrift/lib/thrift/apache_thrift_sbe/RequestRpcMetadata.h>
#include <thrift/lib/thrift/apache_thrift_sbe/RequestRpcMetadataOptional.h>
#include <thrift/lib/thrift/apache_thrift_sbe/ResponseRpcMetadata.h>
#include <thrift/lib/thrift/apache_thrift_sbe/RpcPriority.h>
#include <thrift/lib/thrift/apache_thrift_sbe/VarDataEncoding.h>
#include <thrift/lib/thrift/apache_thrift_sbe/VarStringEncoding.h>

using namespace ::testing;
using namespace apache::thrift;
using namespace apache::thrift::sbe;

const auto kEmptyString = std::string("");

TEST(RpcMetadataSerializationTest, BasicRoundTripRequestRpcMetadata) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(64);
  auto metadata = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  metadata->otherMetadataCount(0);
  metadata->putName(std::string("test_name"));
  metadata->putInteractionMetadata(kEmptyString);
  metadata->putOptionalMetdata(kEmptyString);
  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);

  EXPECT_EQ(other->protocol(), sbe::ProtocolId::COMPACT);
  EXPECT_EQ(other->kind(), sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  auto om1 = other->otherMetadata();

  EXPECT_EQ(om1.count(), 0);
  EXPECT_EQ(om1.hasNext(), false);

  auto name = other->getNameAsStringView();
  EXPECT_EQ(name, "test_name");

  EXPECT_EQ(other->skipInteractionMetadata(), 0);
  EXPECT_EQ(other->skipOptionalMetdata(), 0);
}

TEST(RpcMetadataSerializationTest, BasicRoundTripResponseRpcMetadata) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(128);
  auto metadata = MessageWrapper<ResponseRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->streamId(100);
  metadata->otherMetadataCount(0);
  metadata->putExceptionMetadata(kEmptyString);
  metadata->putOptionalMetadata(kEmptyString);
  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<ResponseRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);
  EXPECT_EQ(other->streamId(), 100);

  auto om1 = other->otherMetadata();
  EXPECT_EQ(om1.count(), 0);
  EXPECT_EQ(om1.hasNext(), false);
  EXPECT_EQ(other->skipExceptionMetadata(), 0);
  EXPECT_EQ(other->skipOptionalMetadata(), 0);
}

TEST(RpcMetadataSerializationTest, RequestRpcMetadataWithHeaders) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(128);
  auto metadata = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  auto om1 = metadata->otherMetadataCount(3);
  for (int i = 0; i < 3; i++) {
    om1.next();
    om1.putOtherMetadataKey(std::to_string(i));
    om1.putOtherMetadataValue(std::string("test_value"));
  }

  metadata->putName(std::string("test_name"));
  metadata->putInteractionMetadata(kEmptyString);
  metadata->putOptionalMetdata(kEmptyString);
  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);
  EXPECT_EQ(other->protocol(), sbe::ProtocolId::COMPACT);
  EXPECT_EQ(other->kind(), sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  auto om2 = other->otherMetadata();
  for (int i = 0; i < 3; i++) {
    om2.next();
    auto key = om2.getOtherMetadataKeyAsString();
    auto value = om2.getOtherMetadataValueAsString();
    EXPECT_EQ(key, std::to_string(i));
    EXPECT_EQ(value, "test_value");
  }

  EXPECT_EQ(other->getNameAsString(), "test_name");
  EXPECT_EQ(other->skipInteractionMetadata(), 0);
  EXPECT_EQ(other->skipOptionalMetdata(), 0);
}

TEST(RpcMetadataSerializationTest, ResponseRpcMetadataWithHeaders) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(128);
  auto metadata = MessageWrapper<ResponseRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->streamId(100);

  auto om1 = metadata->otherMetadataCount(3);
  for (int i = 0; i < 3; i++) {
    om1.next();
    om1.putOtherMetadataKey(std::to_string(i));
    om1.putOtherMetadataValue(std::string("test_value"));
  }

  metadata->putExceptionMetadata(kEmptyString);
  metadata->putOptionalMetadata(kEmptyString);
  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<ResponseRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);
  auto streamId = other->streamId();
  EXPECT_EQ(streamId, 100);

  auto om2 = other->otherMetadata();
  for (int i = 0; i < 3; i++) {
    om2.next();
    auto key = om2.getOtherMetadataKeyAsString();
    auto value = om2.getOtherMetadataValueAsString();
    EXPECT_EQ(key, std::to_string(i));
    EXPECT_EQ(value, "test_value");
  }
}

TEST(RpcMetadataSerializationTest, RequestRpcMetadataWithOptionalMetadata) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(256);
  auto loadMetric = std::string("load_metric");
  auto tenantId = std::string("tenant_id");
  auto serviceTraceMetadata = std::string("service_trace_metadata");
  auto loggingContext = std::string("logging_context");

  // clang-format off
  const size_t encodingLength =
    // Message Header Length
    sbe::MessageHeader::encodedLength() +

    // Encoding Length for fixed fields
    sbe::RequestRpcMetadataOptional::clientTimeoutMsEncodingLength() +
    sbe::RequestRpcMetadataOptional::queueTimeoutMsEncodingLength() +
    sbe::RequestRpcMetadataOptional::priorityEncodingLength() +
    sbe::RequestRpcMetadataOptional::compressionEncodingLength() +
    sbe::CompressionConfig::encodedLength() +
    sbe::FdMetadata::encodedLength() +

    // Encoding Length for dynamic fields
    sbe::VarDataEncoding::lengthEncodingLength() +
    loadMetric.size() +
    sbe::VarStringEncoding::lengthEncodingLength() +
    tenantId.size() +
    sbe::VarStringEncoding::lengthEncodingLength() +
    serviceTraceMetadata.size() +
    sbe::VarDataEncoding::lengthEncodingLength() +
    loggingContext.size();

  // clang-format on

  auto metadata = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  metadata->otherMetadataCount(0);
  metadata->putName(std::string("test_name"));
  metadata->putInteractionMetadata(kEmptyString);

  const auto pos = metadata->sbePosition();
  auto optionalMetadata =
      MessageWrapper<RequestRpcMetadataOptional, MessageHeader>();
  metadata.wrapSubMessageForEncode(
      optionalMetadata, encodingLength, [](auto& message) {
        return message.getOptionalMetdataAsStringView();
      });

  auto expectedPos =
      // clang-format off
      // previous position
      pos +
      // encoded length of RequestRpcMetadataOptional
      encodingLength +
      // size of binary field length in RpcMetadata
      sbe::VarDataEncoding::lengthEncodingLength();

  // clang-format on
  EXPECT_EQ(expectedPos, metadata->sbePosition());

  optionalMetadata->clientTimeoutMs(100)
      .queueTimeoutMs(200)
      .priority(sbe::RpcPriority::NORMAL)
      .compression(sbe::CompressionAlgorithm::ZSTD);

  auto cc = optionalMetadata->compressionConfig();
  cc.codecConfig().zlibCompressionCodecConfig(true);
  cc.compressionSizeLimit(100);

  optionalMetadata->fdMetadata().fdSeqNum(100).numFds(10);

  optionalMetadata->putLoadMetric(loadMetric)
      .putTenantId(tenantId)
      .putServiceTraceMeta(serviceTraceMetadata)
      .putLoggingContext(loggingContext);

  optionalMetadata->checkEncodingIsComplete();

  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);

  EXPECT_EQ(other->protocol(), sbe::ProtocolId::COMPACT);
  EXPECT_EQ(other->kind(), sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  auto om1 = other->otherMetadata();

  EXPECT_EQ(om1.count(), 0);
  EXPECT_EQ(om1.hasNext(), false);

  auto name = other->getNameAsStringView();
  EXPECT_EQ(name, "test_name");

  EXPECT_EQ(0, other->skipInteractionMetadata());

  auto otherOptional =
      MessageWrapper<RequestRpcMetadataOptional, MessageHeader>();
  other.wrapSubMessageForDecode(otherOptional, [](auto& message) {
    return message.getOptionalMetdataAsStringView();
  });

  EXPECT_EQ(otherOptional->clientTimeoutMs(), 100);
  EXPECT_EQ(otherOptional->queueTimeoutMs(), 200);
  EXPECT_EQ(otherOptional->priority(), sbe::RpcPriority::NORMAL);
  EXPECT_EQ(otherOptional->compression(), sbe::CompressionAlgorithm::ZSTD);

  auto otherCc = otherOptional->compressionConfig();
  EXPECT_EQ(otherCc.codecConfig().zlibCompressionCodecConfig(), true);

  auto fd = otherOptional->fdMetadata();
  EXPECT_EQ(fd.fdSeqNum(), 100);
  EXPECT_EQ(fd.numFds(), 10);

  auto loadMetric2 = otherOptional->getLoadMetricAsString();
  EXPECT_EQ(loadMetric2, loadMetric);

  auto tenantId2 = otherOptional->getTenantIdAsString();
  EXPECT_EQ(tenantId2, tenantId);

  auto serviceTrace2 = otherOptional->getServiceTraceMetaAsString();
  EXPECT_EQ(serviceTrace2, serviceTraceMetadata);
}

TEST(RpcMetadataSerializationTest, RequestRpcMetadataWithInteractionMetadata) {
  auto interactionId = 1234;
  auto interactionName = std::string("the_interaction_name");
  // clang-format off
  const std::uint32_t encodingLength =
      sbe::MessageHeader::encodedLength() +
      sbe::InteractionCreate::interactionIdEncodingLength() +
      sbe::VarDataEncoding::lengthEncodingLength() +
      interactionName.size();
  // clang-format on

  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::create(128);
  auto metadata = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  metadata.wrapForEncode(*buf);
  metadata->protocol(sbe::ProtocolId::COMPACT);
  metadata->kind(sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  metadata->otherMetadataCount(0);
  metadata->putName(std::string("test_name"));

  const auto pos = metadata->sbePosition();

  auto interaction = MessageWrapper<sbe::InteractionCreate, MessageHeader>();
  metadata.wrapSubMessageForEncode(
      interaction, encodingLength, [](auto& message) {
        return message.getInteractionMetadataAsStringView();
      });

  auto expectedPos =
      // clang-format off
      // previous position
      pos +
      // encoded length of InteractionCreate
      encodingLength +
      // size of binary field length in RpcMetadata
      sbe::VarDataEncoding::lengthEncodingLength();

  // clang-format on
  EXPECT_EQ(expectedPos, metadata->sbePosition());

  interaction->interactionId(interactionId);
  interaction->putInteractionName(interactionName);
  interaction->checkEncodingIsComplete();

  metadata->putOptionalMetdata(kEmptyString);
  metadata.completeEncoding(*buf);

  auto other = MessageWrapper<RequestRpcMetadata, MessageHeader>();
  other.wrapForDecode(*buf);

  EXPECT_EQ(other->protocol(), sbe::ProtocolId::COMPACT);
  EXPECT_EQ(other->kind(), sbe::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);

  auto om1 = other->otherMetadata();

  EXPECT_EQ(om1.count(), 0);
  EXPECT_EQ(om1.hasNext(), false);

  auto name = other->getNameAsStringView();
  EXPECT_EQ(name, "test_name");

  auto subMessageOptional =
      other.getSubMessageTemplateIdAndBuffer<MessageHeader>([](auto& message) {
        return message.getInteractionMetadataAsStringView();
      });

  EXPECT_EQ(subMessageOptional.has_value(), true);
  auto templateId = subMessageOptional.value().first;
  auto interactionBuffer = subMessageOptional.value().second;

  // Verify that the schema is the InteractionCreate schema
  EXPECT_EQ(sbe::InteractionCreate::sbeTemplateId(), templateId);
  EXPECT_NE(sbe::InteractionRequest::sbeTemplateId(), templateId);
  EXPECT_NE(sbe::InterationTerminate::sbeTemplateId(), templateId);

  auto otherInteraction = MessageWrapper<InteractionCreate, MessageHeader>();
  otherInteraction.wrapForDecode(interactionBuffer);
  EXPECT_EQ(otherInteraction->interactionId(), interactionId);

  EXPECT_EQ(otherInteraction->getInteractionNameAsString(), interactionName);

  EXPECT_EQ(0, other->skipOptionalMetdata());
}
