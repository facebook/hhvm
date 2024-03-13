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

#if __SBE_DEBUG_MODE == 1
#define SBE_ENABLE_PRECEDENCE_CHECKS
#else
#define SBE_NO_BOUNDS_CHECK
#endif

#include <folly/Benchmark.h>
#include <folly/FixedString.h>
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

namespace facebook::sbe::test {

using apache::thrift::Client;
using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::sbe::MessageWrapper;

constexpr auto customerId = folly::makeFixedString("DF401a92F5cCd5c");
constexpr int kNumCustomers = 500;
const size_t concurrency = 256;

const auto kPath =
    "thrift/lib/cpp2/test/sbe/integration_test/resources/test_data.txt";

class BenchmarkService {
 public:
  explicit BenchmarkService() {
    handler_ = facebook::sbe::test::createCustomerLookupHandler(kPath);
    runner_ =
        std::make_unique<ScopedServerInterfaceThread>(handler_, [](auto& ts) {
          ts.setNumIOWorkerThreads(10);
          ts.setNumAcceptThreads(1);
          ts.setNumCPUWorkerThreads(10);
        });
    client_ = runner_->newClient<Client<CustomerLookupService>>(
        nullptr, [&](auto socket) mutable {
          return apache::thrift::RocketClientChannel::newChannel(
              std::move(socket));
        });

    auto it = handler_->getCustomers().begin();
    for (int i = 0; i < kNumCustomers; ++i, ++it) {
      customerIds_.push_back(it->first);
    }
  }
  ~BenchmarkService() { runner_->getThriftServer().stop(); }

  std::shared_ptr<CustomerLookupHandler> handler_;
  std::unique_ptr<ScopedServerInterfaceThread> runner_;
  std::unique_ptr<Client<::facebook::sbe::test::CustomerLookupService>> client_;
  std::vector<std::string> customerIds_;
};

std::unique_ptr<BenchmarkService> service_;

inline static void readCustomer(
    MessageWrapper<CustomerResponse, MessageHeader>& customerResponse) {
  folly::doNotOptimizeAway(customerResponse->index());
  folly::doNotOptimizeAway(customerResponse->getCustomerIdAsStringView());
  folly::doNotOptimizeAway(customerResponse->getFirstNameAsStringView());
  folly::doNotOptimizeAway(customerResponse->getLastNameAsStringView());
  folly::doNotOptimizeAway(customerResponse->getCompanyAsStringView());
  folly::doNotOptimizeAway(customerResponse->getCityAsStringView());
  folly::doNotOptimizeAway(customerResponse->getPhone1AsStringView());
  folly::doNotOptimizeAway(customerResponse->getPhone2AsStringView());
  folly::doNotOptimizeAway(customerResponse->getEmailAsStringView());
  folly::doNotOptimizeAway(customerResponse->getSubscriptionDateAsStringView());
  folly::doNotOptimizeAway(customerResponse->getWebSiteAsStringView());
}

void lookupOneS_wrk(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(
          SingleCustomerLookup::computeLength(customerId.size()) +
          MessageHeader::encodedLength());
      auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      lookup->putCustomerId(customerId.data(), customerId.size());
      lookup.completeEncoding(*req);

      auto res = co_await service_->client_->co_lookupOne(*req);
      auto customerResponse = MessageWrapper<CustomerResponse, MessageHeader>();
      customerResponse.wrapForDecode(res);
      readCustomer(customerResponse);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupOneS_wrk, 1)
BENCHMARK_PARAM(lookupOneS_wrk, 10)
BENCHMARK_PARAM(lookupOneS_wrk, 100)
BENCHMARK_PARAM(lookupOneS_wrk, 1000)
BENCHMARK_PARAM(lookupOneS_wrk, 10000)
BENCHMARK_PARAM(lookupOneS_wrk, 100000)

void lookupOneS_eb(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(
          SingleCustomerLookup::computeLength(customerId.size()) +
          MessageHeader::encodedLength());
      auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      lookup->putCustomerId(customerId.data(), customerId.size());
      lookup.completeEncoding(*req);

      auto res = co_await service_->client_->co_lookupOneE(*req);
      auto customerResponse = MessageWrapper<CustomerResponse, MessageHeader>();
      customerResponse.wrapForDecode(res);
      readCustomer(customerResponse);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupOneS_eb, 1)
BENCHMARK_PARAM(lookupOneS_eb, 10)
BENCHMARK_PARAM(lookupOneS_eb, 100)
BENCHMARK_PARAM(lookupOneS_eb, 1000)
BENCHMARK_PARAM(lookupOneS_eb, 10000)
BENCHMARK_PARAM(lookupOneS_eb, 100000)

void lookOneS_noRPC(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(
          SingleCustomerLookup::computeLength(customerId.size()) +
          MessageHeader::encodedLength());
      auto lookup = MessageWrapper<SingleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      lookup->putCustomerId(customerId.data(), customerId.size());
      lookup.completeEncoding(*req);

      auto res = co_await service_->handler_->co_lookupOne(std::move(req));
      auto customerResponse = MessageWrapper<CustomerResponse, MessageHeader>();
      customerResponse.wrapForDecode(*res);
      readCustomer(customerResponse);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookOneS_noRPC, 1)
BENCHMARK_PARAM(lookOneS_noRPC, 10)
BENCHMARK_PARAM(lookOneS_noRPC, 100)
BENCHMARK_PARAM(lookOneS_noRPC, 1000)
BENCHMARK_PARAM(lookOneS_noRPC, 10000)
BENCHMARK_PARAM(lookOneS_noRPC, 100000)

void lookupManyS_wrk(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(16000);
      auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      auto customerId = lookup->customerIdsCount(kNumCustomers);
      for (auto i : service_->customerIds_) {
        customerId.next();
        customerId.putCustomerId(i);
      }
      lookup.completeEncoding(*req);

      auto res = co_await service_->client_->co_lookupMany(*req);
      auto multipleCustomerResponse =
          MessageWrapper<MultipleCustomerResponse, MessageHeader>();
      multipleCustomerResponse.wrapForDecode(res);
      auto customerResponses = multipleCustomerResponse->customerResponses();
      while (customerResponses.hasNext()) {
        customerResponses.next();
        auto customerResponse =
            MessageWrapper<CustomerResponse, MessageHeader>();
        auto s = customerResponses.getCustomerResponseAsStringView();
        customerResponse.wrapForDecode(s);
        readCustomer(customerResponse);
      }

      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyS_wrk, 1)
BENCHMARK_PARAM(lookupManyS_wrk, 10)
BENCHMARK_PARAM(lookupManyS_wrk, 100)
BENCHMARK_PARAM(lookupManyS_wrk, 1000)
BENCHMARK_PARAM(lookupManyS_wrk, 10000)
BENCHMARK_PARAM(lookupManyS_wrk, 100000)

void lookupManyS_eb(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(16000);
      auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      auto customerId = lookup->customerIdsCount(kNumCustomers);
      for (auto i : service_->customerIds_) {
        customerId.next();
        customerId.putCustomerId(i);
      }
      lookup.completeEncoding(*req);

      auto res = co_await service_->client_->co_lookupManyE(*req);
      auto multipleCustomerResponse =
          MessageWrapper<MultipleCustomerResponse, MessageHeader>();
      multipleCustomerResponse.wrapForDecode(res);
      auto customerResponses = multipleCustomerResponse->customerResponses();
      while (customerResponses.hasNext()) {
        customerResponses.next();
        auto customerResponse =
            MessageWrapper<CustomerResponse, MessageHeader>();
        auto s = customerResponses.getCustomerResponseAsStringView();
        customerResponse.wrapForDecode(s);
        readCustomer(customerResponse);
      }

      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyS_eb, 1)
BENCHMARK_PARAM(lookupManyS_eb, 10)
BENCHMARK_PARAM(lookupManyS_eb, 100)
BENCHMARK_PARAM(lookupManyS_eb, 1000)
BENCHMARK_PARAM(lookupManyS_eb, 10000)
BENCHMARK_PARAM(lookupManyS_eb, 100000)

void lookupManyS_noRPC(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      auto req = folly::IOBuf::create(16000);
      auto lookup = MessageWrapper<MultipleCustomerLookup, MessageHeader>();
      lookup.wrapForEncode(*req);
      auto customerId = lookup->customerIdsCount(kNumCustomers);
      for (auto custId : service_->customerIds_) {
        customerId.next();
        customerId.putCustomerId(custId);
      }
      lookup.completeEncoding(*req);

      auto res = co_await service_->handler_->co_lookupMany(std::move(req));
      auto customerResponse =
          MessageWrapper<MultipleCustomerResponse, MessageHeader>();
      customerResponse.wrapForDecode(*res);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyS_noRPC, 1)
BENCHMARK_PARAM(lookupManyS_noRPC, 10)
BENCHMARK_PARAM(lookupManyS_noRPC, 100)
BENCHMARK_PARAM(lookupManyS_noRPC, 1000)
BENCHMARK_PARAM(lookupManyS_noRPC, 10000)
BENCHMARK_PARAM(lookupManyS_noRPC, 100000)

void lookupOneT_wrk(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TSingleCustomerLookup lookup;
      lookup.customerId() = customerId;
      co_await service_->client_->co_lookupOneT(lookup);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(folly::coro::collectAllWindowed(
      std::move(tasks), std::thread::hardware_concurrency()));
}

BENCHMARK_PARAM(lookupOneT_wrk, 1)
BENCHMARK_PARAM(lookupOneT_wrk, 10)
BENCHMARK_PARAM(lookupOneT_wrk, 100)
BENCHMARK_PARAM(lookupOneT_wrk, 1000)
BENCHMARK_PARAM(lookupOneT_wrk, 10000)
BENCHMARK_PARAM(lookupOneT_wrk, 100000)

void lookupOneT_eb(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TSingleCustomerLookup lookup;
      lookup.customerId() = customerId;
      co_await service_->client_->co_lookupOneTE(lookup);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupOneT_eb, 1)
BENCHMARK_PARAM(lookupOneT_eb, 10)
BENCHMARK_PARAM(lookupOneT_eb, 100)
BENCHMARK_PARAM(lookupOneT_eb, 1000)
BENCHMARK_PARAM(lookupOneT_eb, 10000)
BENCHMARK_PARAM(lookupOneT_eb, 100000)

void lookupOneT_noRPC(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TSingleCustomerLookup lookup;
      lookup.customerId() = customerId;
      co_await service_->handler_->co_lookupOneT(
          std::make_unique<TSingleCustomerLookup>(lookup));
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupOneT_noRPC, 1)
BENCHMARK_PARAM(lookupOneT_noRPC, 10)
BENCHMARK_PARAM(lookupOneT_noRPC, 100)
BENCHMARK_PARAM(lookupOneT_noRPC, 1000)
BENCHMARK_PARAM(lookupOneT_noRPC, 10000)
BENCHMARK_PARAM(lookupOneT_noRPC, 100000)

void lookupManyT_wrk(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TMultipleCustomerLookup lookup;
      std::vector<std::string> customerIds;
      for (auto i : service_->customerIds_) {
        customerIds.push_back(i);
      }
      lookup.customerIds() = std::move(customerIds);
      co_await service_->client_->co_lookupManyT(lookup);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyT_wrk, 1)
BENCHMARK_PARAM(lookupManyT_wrk, 10)
BENCHMARK_PARAM(lookupManyT_wrk, 100)
BENCHMARK_PARAM(lookupManyT_wrk, 1000)
BENCHMARK_PARAM(lookupManyT_wrk, 10000)
BENCHMARK_PARAM(lookupManyT_wrk, 100000)

void lookupManyT_eb(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TMultipleCustomerLookup lookup;
      std::vector<std::string> customerIds;
      for (auto i : service_->customerIds_) {
        customerIds.push_back(i);
      }
      lookup.customerIds() = std::move(customerIds);
      co_await service_->client_->co_lookupManyTE(lookup);
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyT_eb, 1)
BENCHMARK_PARAM(lookupManyT_eb, 10)
BENCHMARK_PARAM(lookupManyT_eb, 100)
BENCHMARK_PARAM(lookupManyT_eb, 1000)
BENCHMARK_PARAM(lookupManyT_eb, 10000)
BENCHMARK_PARAM(lookupManyT_eb, 100000)

void lookupManyT_noRPC(int, int iterations) {
  std::vector<folly::coro::Task<void>> tasks;
  tasks.reserve(iterations);
  for (int i = 0; i < iterations; ++i) {
    auto task = [&]() -> folly::coro::Task<void> {
      TMultipleCustomerLookup lookup;
      std::vector<std::string> customerIds;
      for (auto i : service_->customerIds_) {
        customerIds.push_back(i);
      }
      lookup.customerIds() = std::move(customerIds);
      co_await service_->handler_->co_lookupManyT(
          std::make_unique<TMultipleCustomerLookup>(lookup));
      co_return;
    };
    tasks.push_back(task());
  }
  folly::coro::blockingWait(
      folly::coro::collectAllWindowed(std::move(tasks), concurrency));
}

BENCHMARK_PARAM(lookupManyT_noRPC, 1)
BENCHMARK_PARAM(lookupManyT_noRPC, 10)
BENCHMARK_PARAM(lookupManyT_noRPC, 100)
BENCHMARK_PARAM(lookupManyT_noRPC, 1000)
BENCHMARK_PARAM(lookupManyT_noRPC, 10000)
BENCHMARK_PARAM(lookupManyT_noRPC, 100000)

} // namespace facebook::sbe::test

using facebook::sbe::test::BenchmarkService;

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

  folly::init(&argc, &argv);

  THRIFT_FLAG_SET_MOCK(rocket_allocating_strategy_parser, true);

  facebook::sbe::test::service_ = std::make_unique<BenchmarkService>();
  folly::runBenchmarks();
  return 0;
}
