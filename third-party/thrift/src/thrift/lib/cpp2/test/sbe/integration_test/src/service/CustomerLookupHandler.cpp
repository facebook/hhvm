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

#include <cstddef>
#include <memory>
#include <vector>
#include <glog/logging.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/CustomerLookupHandler.h>

#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerNotFound.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MultipleCustomerLookup.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/SingleCustomerLookup.h>

namespace facebook::sbe::test {

using apache::thrift::sbe::MessageWrapper;
using std::unique_ptr;

CustomerLookupHandler::CustomerLookupHandler(
    folly::F14FastMap<std::string, Customer>&& customers)
    : customers_(std::move(customers)) {
  customerNotFound_ = folly::IOBuf::create(16);
  auto customerNotFound = MessageWrapper<CustomerNotFound, MessageHeader>();
  customerNotFound.wrapForEncode(*customerNotFound_);
}

inline size_t CustomerLookupHandler::calculateCustomerReponseSize(
    const Customer& customer) {
  return MessageHeader::encodedLength() +
      CustomerResponse::computeLength(
             customer.customerId.size(),
             customer.firstName.size(),
             customer.lastName.size(),
             customer.company.size(),
             customer.city.size(),
             customer.country.size(),
             customer.phone1.size(),
             customer.phone2.size(),
             customer.email.size(),
             customer.subscriptionDate.size(),
             customer.website.size());
}

inline void CustomerLookupHandler::customerToCustomerResponse(
    MessageWrapper<CustomerResponse, MessageHeader>& customerResponse,
    const Customer& customer) {
  customerResponse->index(customer.index);
  customerResponse->putCustomerId(customer.customerId);
  customerResponse->putFirstName(customer.firstName);
  customerResponse->putLastName(customer.lastName);
  customerResponse->putCompany(customer.company);
  customerResponse->putCity(customer.city);
  customerResponse->putCountry(customer.country);
  customerResponse->putPhone1(customer.phone1);
  customerResponse->putPhone2(customer.phone2);
  customerResponse->putEmail(customer.email);
  customerResponse->putSubscriptionDate(customer.subscriptionDate);
  customerResponse->putWebSite(customer.website);
}

inline void CustomerLookupHandler::customerToCustomerResponse(
    char* buffer, size_t length, const Customer& customer) {
  auto customerResponse = MessageWrapper<CustomerResponse, MessageHeader>();
  customerResponse.wrapForEncode(buffer, length);
  customerToCustomerResponse(customerResponse, customer);
  customerResponse.completeEncoding();
}

inline void CustomerLookupHandler::customerToCustomerResponse(
    folly::IOBuf& buf, const Customer& customer) {
  auto customerResponse = MessageWrapper<CustomerResponse, MessageHeader>();
  customerResponse.wrapForEncode(buf);
  customerToCustomerResponse(customerResponse, customer);
  customerResponse.completeEncoding(buf);
}

inline std::unique_ptr<folly::IOBuf>
CustomerLookupHandler::customerToCustomerResponse(const Customer& customer) {
  auto size = calculateCustomerReponseSize(customer);
  auto buf = folly::IOBuf::create(size);
  customerToCustomerResponse(*buf, customer);
  return buf;
}

inline std::unique_ptr<folly::IOBuf> CustomerLookupHandler::doLookupOne(
    std::unique_ptr<folly::IOBuf> request) {
  auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
  lookup.wrapForDecode(*request);
  auto customerId = lookup->getCustomerIdAsStringView();
  DLOG(INFO) << "Looking up customer " << customerId;
  auto it = customers_.find(customerId);
  if (it != customers_.end()) {
    DLOG(INFO) << "Found customer " << customerId;
    const auto& customer = it->second;
    return customerToCustomerResponse(customer);
  } else {
    DLOG(INFO) << "Customer " << customerId << " not found";
    return customerNotFound_->cloneOne();
  }
}

inline std::unique_ptr<folly::IOBuf> CustomerLookupHandler::doLookupMany(
    std::unique_ptr<folly::IOBuf> request) {
  DLOG(INFO) << "Looking up many customers";
  auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
  lookup.wrapForDecode(*request);
  auto customerIds = lookup->customerIds();
  const auto count = customerIds.count();
  std::vector<std::tuple<std::size_t>> sizes;
  sizes.reserve(count);
  std::vector<Customer*> customerPtrs;
  customerPtrs.reserve(count);
  while (customerIds.hasNext()) {
    auto next = customerIds.next();
    auto customerId = next.getCustomerIdAsStringView();
    DLOG(INFO) << "Looking up customer " << customerId;
    auto it = customers_.find(customerId);
    if (it != customers_.end()) {
      DLOG(INFO) << "Found customer " << customerId;
      auto* customer = &customers_[customerId];
      sizes.push_back(calculateCustomerReponseSize(*customer));
      customerPtrs.push_back(customer);
    } else {
      DLOG(INFO) << "Customer " << customerId << " not found";
    }
  }

  if (customerPtrs.empty()) {
    DLOG(INFO) << "No customers found";
    return customerNotFound_->cloneOne();
  } else {
    DLOG(INFO) << "Found " << customerPtrs.size() << " customers";

    auto size = MultipleCustomerResponse::computeLength(sizes) +
        MessageHeader::encodedLength();

    DLOG(INFO) << "Creating response buffer size " << size;
    auto res = folly::IOBuf::create(size);
    auto mutipleCustomerResponse =
        MessageWrapper<MultipleCustomerResponse, MessageHeader>();
    mutipleCustomerResponse.wrapForEncode(*res);
    DLOG(INFO) << "Creating a group with " << customerPtrs.size()
               << " customers";
    auto customerResponses =
        mutipleCustomerResponse->customerResponsesCount(customerPtrs.size());

    for (size_t i = 0; i < count; ++i) {
      DLOG(INFO) << "Writing customer response number " << i << std::endl;
      customerResponses.next();
      auto& customer = *customerPtrs[i];
      size_t custRespSize = std::get<0>(sizes[i]);
      char* buffer = customerResponses.putCustomerResponse(custRespSize);
      customerToCustomerResponse(buffer, custRespSize, customer);
    }
    mutipleCustomerResponse.completeEncoding(*res);
    DLOG(INFO) << "Done encoding response";
    return res;
  }
}

inline TCustomerResponse CustomerLookupHandler::customerToTCustomerResponse(
    const Customer& customer) {
  TCustomerResponse response;
  response.index() = customer.index;
  response.customerId() = customer.customerId;
  response.firstName() = customer.firstName;
  response.lastName() = customer.lastName;
  response.company() = customer.company;
  response.city() = customer.city;
  response.country() = customer.country;
  response.phone1() = customer.phone1;
  response.phone2() = customer.phone2;
  response.email() = customer.email;
  response.subscriptionDate() = customer.subscriptionDate;
  response.webSite() = customer.website;
  return response;
}

std::unique_ptr<TCustomerResponse> CustomerLookupHandler::doLookupOneT(
    std::unique_ptr<TSingleCustomerLookup> request) {
  const auto& customerId = *request->customerId();
  DLOG(INFO) << "Looking up customer " << customerId;
  auto it = customers_.find(customerId);
  if (it != customers_.end()) {
    DLOG(INFO) << "Found customer " << customerId;
    const auto& customer = it->second;
    return std::make_unique<TCustomerResponse>(
        customerToTCustomerResponse(customer));
  } else {
    DLOG(INFO) << "Customer " << customerId << " not found";
    throw std::runtime_error("Customer not found");
  }
}

std::unique_ptr<TMultipleCustomerResponse> CustomerLookupHandler::doLookupManyT(
    std::unique_ptr<TMultipleCustomerLookup> lookup) {
  DLOG(INFO) << "Looking up many customers";

  std::vector<TCustomerResponse> responses;
  for (const auto& customerId : *lookup->customerIds()) {
    DLOG(INFO) << "Looking up customer " << customerId;
    auto it = customers_.find(customerId);
    if (it != customers_.end()) {
      DLOG(INFO) << "Found customer " << customerId;
      const auto& customer = it->second;
      responses.push_back(customerToTCustomerResponse(customer));
    } else {
      DLOG(INFO) << "Customer " << customerId << " not found";
      continue;
    }
  }

  TMultipleCustomerResponse response;
  response.customerResponses() = std::move(responses);
  return std::make_unique<TMultipleCustomerResponse>(response);
}

} // namespace facebook::sbe::test
