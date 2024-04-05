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

#pragma once
#include <memory>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/if/gen-cpp2/CustomerLookupService.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/Customer.h>

namespace facebook::sbe::test {

class CustomerLookupHandler
    : public apache::thrift::ServiceHandler<CustomerLookupService> {
 public:
  explicit CustomerLookupHandler(
      folly::F14FastMap<std::string, Customer>&& customers);

  folly::coro::Task<std::unique_ptr<folly::IOBuf>> co_lookupOne(
      std::unique_ptr<folly::IOBuf> p_request) override {
    co_return doLookupOne(std::move(p_request));
  }

  folly::coro::Task<std::unique_ptr<folly::IOBuf>> co_lookupMany(
      std::unique_ptr<folly::IOBuf> p_request) override {
    co_return doLookupMany(std::move(p_request));
  }

  folly::coro::Task<std::unique_ptr<TCustomerResponse>> co_lookupOneT(
      std::unique_ptr<TSingleCustomerLookup> p_request) override {
    co_return doLookupOneT(std::move(p_request));
  }

  folly::coro::Task<std::unique_ptr<TMultipleCustomerResponse>> co_lookupManyT(
      std::unique_ptr<TMultipleCustomerLookup> p_request) override {
    co_return doLookupManyT(std::move(p_request));
  }

  void async_eb_lookupOneE(
      std::unique_ptr<apache::thrift::HandlerCallback<
          std::unique_ptr<folly::IOBuf>>> callback,
      std::unique_ptr<::facebook::sbe::test::IOBuf> p_request) override {
    auto res = doLookupOne(std::move(p_request));
    callback->complete(folly::Try<decltype(res)>{std::move(res)});
  }

  void async_eb_lookupManyE(
      std::unique_ptr<apache::thrift::HandlerCallback<
          std::unique_ptr<folly::IOBuf>>> callback,
      std::unique_ptr<::facebook::sbe::test::IOBuf> p_request) override {
    auto res = doLookupMany(std::move(p_request));
    callback->complete(folly::Try<decltype(res)>{std::move(res)});
  }

  void async_eb_lookupOneTE(
      std::unique_ptr<apache::thrift::HandlerCallback<
          std::unique_ptr<::facebook::sbe::test::TCustomerResponse>>> callback,
      std::unique_ptr<::facebook::sbe::test::TSingleCustomerLookup> p_request)
      override {
    auto res = doLookupOneT(std::move(p_request));
    callback->complete(folly::Try<decltype(res)>{std::move(res)});
  }

  void async_eb_lookupManyTE(
      std::unique_ptr<apache::thrift::HandlerCallback<std::unique_ptr<
          ::facebook::sbe::test::TMultipleCustomerResponse>>> callback,
      std::unique_ptr<::facebook::sbe::test::TMultipleCustomerLookup> p_request)
      override {
    auto res = doLookupManyT(std::move(p_request));
    callback->complete(folly::Try<decltype(res)>{std::move(res)});
  }

  folly::F14FastMap<std::string, Customer>& getCustomers() {
    return customers_;
  }

 private:
  folly::F14FastMap<std::string, Customer> customers_;
  std::unique_ptr<folly::IOBuf> customerNotFound_;

  std::unique_ptr<folly::IOBuf> doLookupOne(std::unique_ptr<folly::IOBuf>);

  std::unique_ptr<folly::IOBuf> doLookupMany(std::unique_ptr<folly::IOBuf>);

  std::unique_ptr<TCustomerResponse> doLookupOneT(
      std::unique_ptr<TSingleCustomerLookup>);

  std::unique_ptr<TMultipleCustomerResponse> doLookupManyT(
      std::unique_ptr<TMultipleCustomerLookup>);

  size_t calculateCustomerReponseSize(const Customer& customer);

  inline void customerToCustomerResponse(
      apache::thrift::sbe::MessageWrapper<CustomerResponse, MessageHeader>&
          customerResponse,
      const Customer& customer);

  inline void customerToCustomerResponse(
      folly::IOBuf& buf, const Customer& customer);

  inline void customerToCustomerResponse(
      char* buffer, size_t length, const Customer& customer);

  inline std::unique_ptr<folly::IOBuf> customerToCustomerResponse(
      const Customer& customer);

  inline TCustomerResponse customerToTCustomerResponse(
      const Customer& customer);
};

} // namespace facebook::sbe::test
