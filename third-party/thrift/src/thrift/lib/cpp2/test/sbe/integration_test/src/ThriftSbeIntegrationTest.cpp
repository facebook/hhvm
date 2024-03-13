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

#include <gtest/gtest.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Collect.h>
#include <folly/experimental/coro/Task.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerNotFound.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerLookup.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/SingleCustomerLookup.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/CustomerLookupService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace facebook::sbe::test;

using apache::thrift::Client;
using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::sbe::MessageWrapper;

const auto kPath =
    "thrift/lib/cpp2/test/sbe/integration_test/resources/test_data.txt";

static void ensureSingleBufferParsing() {
  THRIFT_FLAG_SET_MOCK(rocket_allocating_strategy_parser, true);
}

TEST(IntegrationTest, TestSingleCustomerLookup) {
  ensureSingleBufferParsing();
  auto customerLookupHandler =
      facebook::sbe::test::createCustomerLookupHandler(kPath);
  ScopedServerInterfaceThread runner(customerLookupHandler);
  auto client = runner.newClient<Client<CustomerLookupService>>(
      folly::EventBaseManager::get()->getEventBase(), [&](auto socket) mutable {
        return apache::thrift::RocketClientChannel::newChannel(
            std::move(socket));
      });

  std::string customerId("c57Dc5cF465d279");
  auto req = folly::IOBuf::create(
      SingleCustomerLookup::computeLength(customerId.size()) +
      MessageHeader::encodedLength());
  auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
  lookup.wrapForEncode(*req);
  lookup->putCustomerId(customerId);
  lookup.completeEncoding(*req);

  DLOG(INFO) << "Looking up customer";
  auto res = folly::coro::blockingWait(client->co_lookupOne(*req));
  DLOG(INFO) << "Received response";

  auto messageHeader =
      apache::thrift::sbe::decodeMessageHeader<MessageHeader>(res);
  EXPECT_EQ(messageHeader.templateId(), CustomerResponse::sbeTemplateId());

  auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
  customer.wrapForDecode(res);
  EXPECT_EQ(
      customer->getCustomerIdAsStringView(), std::string{"c57Dc5cF465d279"});
}

TEST(IntegrationTest, TestMultipleCustomerLookup) {
  ensureSingleBufferParsing();
  auto customerLookupHandler =
      facebook::sbe::test::createCustomerLookupHandler(kPath);
  ScopedServerInterfaceThread runner(customerLookupHandler);
  auto client = runner.newClient<Client<CustomerLookupService>>(
      folly::EventBaseManager::get()->getEventBase(), [&](auto socket) mutable {
        return apache::thrift::RocketClientChannel::newChannel(
            std::move(socket));
      });

  auto req = folly::IOBuf::create(16000);
  auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
  lookup.wrapForEncode(*req);

  const std::uint16_t n = 500;
  auto customerId = lookup->customerIdsCount(n);
  {
    auto it = customerLookupHandler->getCustomers().begin();
    for (int i = 0; i < n; ++i, ++it) {
      customerId.next();
      customerId.putCustomerId(it->first);
    }
  }

  lookup.completeEncoding(*req);

  DLOG(INFO) << "Looking up customers";
  auto res = folly::coro::blockingWait(client->co_lookupMany(*req));
  DLOG(INFO) << "Received response";

  auto messageHeader =
      apache::thrift::sbe::decodeMessageHeader<MessageHeader>(res);
  EXPECT_EQ(
      messageHeader.templateId(), MultipleCustomerResponse::sbeTemplateId());

  auto multipleCustomerResponse =
      MessageWrapper<MultipleCustomerResponse, MessageHeader>();
  multipleCustomerResponse.wrapForDecode(res);
  auto customerResponses = multipleCustomerResponse->customerResponses();
  EXPECT_EQ(customerResponses.count(), n);

  auto it = customerLookupHandler->getCustomers().begin();
  for (int i = 0; i < n; ++i, ++it) {
    customerResponses.next();
    auto view = customerResponses.getCustomerResponseAsStringView();
    auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
    customer.wrapForDecode(view);
    EXPECT_EQ(customer->getCustomerIdAsStringView(), it->first);
  }
}
