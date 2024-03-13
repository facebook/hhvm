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
#include <folly/String.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Task.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerNotFound.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerLookup.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/SingleCustomerLookup.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/CustomerLookupHandler.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/DataLoader.h>

using namespace facebook::sbe::test;
using apache::thrift::sbe::MessageWrapper;

const auto kPath =
    "thrift/lib/cpp2/test/sbe/integration_test/resources/test_data.txt";

class CustomerLookupHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    folly::F14FastMap<std::string, Customer> customers;
    DataLoader loader;
    loader.loadIntoMap(kPath, customers);
    EXPECT_EQ(customers.size(), 20000);

    handler_ = std::make_unique<CustomerLookupHandler>(std::move(customers));
  }

  std::unique_ptr<CustomerLookupHandler> handler_;
};

static void do_TestSingleCustomerLookup(
    CustomerLookupHandler& handler,
    std::int64_t index,
    std::string customerId,
    std::string name) {
  auto req = folly::IOBuf::create(256);
  auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
  lookup.wrapForEncode(*req);
  lookup->putCustomerId(customerId);
  lookup.completeEncoding(*req);

  auto res = folly::coro::blockingWait(handler.co_lookupOne(std::move(req)));

  auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
  customer.wrapForDecode(*res);
  EXPECT_EQ(customer->index(), index);
  EXPECT_EQ(customer->getCustomerIdAsStringView(), customerId);
  EXPECT_EQ(customer->getFirstNameAsStringView(), name);
}

TEST_F(CustomerLookupHandlerTest, TestSingleCustomerLookup) {
  auto& handler = *handler_;

  auto it = handler.getCustomers().begin();
  for (int i = 0; i < 10; ++it, ++i) {
    const auto& customer = it->second;
    do_TestSingleCustomerLookup(
        handler, customer.index, customer.customerId, customer.firstName);
  }
}

TEST_F(CustomerLookupHandlerTest, TestMultipleCustomerLookup) {
  auto& handler = *handler_;

  auto req = folly::IOBuf::create(256);
  auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
  lookup.wrapForEncode(*req);
  auto c = lookup->customerIdsCount(3);
  c.next();
  c.putCustomerId(std::string{"c57Dc5cF465d279"});

  c.next();
  c.putCustomerId(std::string{"DF401a92F5cCd5c"});

  c.next();
  c.putCustomerId(std::string{"4bb2dFeedd567Bb"});

  lookup.completeEncoding(*req);

  auto res = folly::coro::blockingWait(handler.co_lookupMany(std::move(req)));

  auto multipleCustomerResponse =
      MessageWrapper<MultipleCustomerResponse, MessageHeader>();
  multipleCustomerResponse.wrapForDecode(*res);
  auto customerResponses = multipleCustomerResponse->customerResponses();

  EXPECT_EQ(customerResponses.count(), 3);

  {
    auto cr = customerResponses.next();
    auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
    auto customerResponse = cr.getCustomerResponseAsStringView();
    customer.wrapForDecode(customerResponse);
    EXPECT_EQ(
        customer->getCustomerIdAsStringView(), std::string{"c57Dc5cF465d279"});
  }

  {
    auto cr = customerResponses.next();
    auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
    auto customerResponse = cr.getCustomerResponseAsStringView();
    customer.wrapForDecode(customerResponse);
    EXPECT_EQ(
        customer->getCustomerIdAsStringView(), std::string{"DF401a92F5cCd5c"});
  }

  {
    auto cr = customerResponses.next();
    auto customer = MessageWrapper<CustomerResponse, MessageHeader>();
    auto customerResponse = cr.getCustomerResponseAsStringView();
    customer.wrapForDecode(customerResponse);
    EXPECT_EQ(
        customer->getCustomerIdAsStringView(), std::string{"4bb2dFeedd567Bb"});
  }
}
