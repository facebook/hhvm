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

#include <random>
#include <folly/Function.h>
#include <folly/GLog.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/perf/cpp2/if/gen-cpp2/ApiBase_types.h>
#include <thrift/perf/cpp2/util/QPSStats.h>

using apache::thrift::ClientReceiveState;
using apache::thrift::RequestCallback;
using facebook::thrift::benchmarks::QPSStats;
using facebook::thrift::benchmarks::TwoInts;

class RequestCallbackWithValidator : public RequestCallback {
 public:
  folly::Function<void(ClientReceiveState&)> validator;
};

template <typename AsyncClient>
class Noop {
 public:
  explicit Noop(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~Noop() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    client->noop(std::move(cb));
  }

  void onewayAsync(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    client->onewayNoop(std::move(cb));
  }

  void asyncReceived(AsyncClient* client, ClientReceiveState&& rstate) {
    try {
      client->recv_noop(rstate);
      stats_->add(op_name_);
    } catch (const apache::thrift::TApplicationException& ex) {
      if (ex.getType() ==
          apache::thrift::TApplicationException::TApplicationExceptionType::
              TIMEOUT) {
        stats_->add(timeout_);
      } else {
        FB_LOG_EVERY_MS(ERROR, 1000)
            << "Error should have caused error() function to be called: "
            << ex.what();
        stats_->add(error_);
      }
    } catch (const std::exception& ex) {
      FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
      stats_->add(fatal_);
    }
  }

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

  void onewaySent() { stats_->add(op_name_); }

 private:
  QPSStats* stats_;
  std::string op_name_ = "noop";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
};

template <typename AsyncClient>
class Sum {
 public:
  explicit Sum(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~Sum() = default;

  void async(
      AsyncClient* client, std::unique_ptr<RequestCallbackWithValidator> cb) {
    request_.x() = gen_();
    request_.y() = gen_();

    cb->validator = [request = request_](ClientReceiveState& rstate) {
      try {
        TwoInts response;
        AsyncClient::recv_sum(response, rstate);
        CHECK_EQ(
            static_cast<uint32_t>(*request.x()) + *request.y(), *response.x());
        CHECK_EQ(
            static_cast<uint32_t>(*request.x()) - *request.y(), *response.y());
      } catch (...) {
        // handled by asyncReceived
      }
    };

    client->sum(std::move(cb), request_);
  }

  void asyncReceived(AsyncClient* client, ClientReceiveState&& rstate) {
    try {
      client->recv_sum(response_, rstate);
      // validated by callback
      stats_->add(op_name_);
    } catch (const apache::thrift::TApplicationException& ex) {
      if (ex.getType() ==
          apache::thrift::TApplicationException::TApplicationExceptionType::
              TIMEOUT) {
        stats_->add(timeout_);
      } else {
        FB_LOG_EVERY_MS(ERROR, 1000)
            << "Error should have caused error() function to be called: "
            << ex.what();
        stats_->add(error_);
      }
    } catch (const std::exception& ex) {
      FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
      stats_->add(fatal_);
    }
  }

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "sum";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  TwoInts request_;
  TwoInts response_;
  std::mt19937 gen_{std::random_device()()};
};

template <typename AsyncClient>
class Timeout {
 public:
  explicit Timeout(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~Timeout() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setQueueTimeout(std::chrono::milliseconds(3));
    rpcOptions.setTimeout(std::chrono::milliseconds(3));
    client->timeout(rpcOptions, std::move(cb));
  }

  void asyncReceived(AsyncClient* client, ClientReceiveState&& rstate) {
    try {
      client->recv_timeout(rstate);
      stats_->add(op_name_);
    } catch (const apache::thrift::TApplicationException& ex) {
      if (ex.getType() ==
          apache::thrift::TApplicationException::TApplicationExceptionType::
              TIMEOUT) {
        stats_->add(timeout_);
      } else {
        FB_LOG_EVERY_MS(ERROR, 1000)
            << "Error should have caused error() function to be called: "
            << ex.what();
        stats_->add(error_);
      }
    } catch (const std::exception& ex) {
      FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
      stats_->add(fatal_);
    }
  }

  void error(AsyncClient*, ClientReceiveState&& state) {
    if (state.isException()) {
      FB_LOG_EVERY_MS(INFO, 1000) << "Error is: " << state.exception().what();
    }
    stats_->add(error_);
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "timeout_success";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  TwoInts request_;
  TwoInts response_;
};

template <typename AsyncClient>
class SemiFutureSum {
 public:
  explicit SemiFutureSum(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~SemiFutureSum() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    request_.x() = gen_();
    request_.y() = gen_();

    client->semifuture_sum(request_)
        .via(client->getChannel()->getEventBase())
        .thenTry([this, cb = std::move(cb), request = request_](auto response) {
          try {
            CHECK_EQ(
                static_cast<uint32_t>(*request.x()) + *request.y(),
                *response->x_ref());
            CHECK_EQ(
                static_cast<uint32_t>(*request.x()) - *request.y(),
                *response->y_ref());
            stats_->add(op_name_);
            cb->replyReceived({});
          } catch (const apache::thrift::TApplicationException& ex) {
            if (ex.getType() ==
                apache::thrift::TApplicationException::
                    TApplicationExceptionType::TIMEOUT) {
              stats_->add(timeout_);
            } else {
              FB_LOG_EVERY_MS(ERROR, 1000)
                  << "Error should have caused error() function to be called: "
                  << ex.what();
              stats_->add(error_);
            }
            cb->replyReceived({});
          } catch (const std::exception& ex) {
            FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
            stats_->add(fatal_);
            cb->requestError({});
          }
        });
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "semifuture_sum";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  TwoInts request_;
  TwoInts response_;
  std::mt19937 gen_{std::random_device()()};
};

template <typename AsyncClient>
class CoSum {
 public:
  explicit CoSum(QPSStats* stats) : stats_(stats) {
    stats_->registerCounter(op_name_);
    stats_->registerCounter(timeout_);
    stats_->registerCounter(error_);
    stats_->registerCounter(fatal_);
  }
  ~CoSum() = default;

  void async(AsyncClient* client, std::unique_ptr<RequestCallback> cb) {
    request_.x() = gen_();
    request_.y() = gen_();

    client->co_sum(request_)
        .scheduleOn(client->getChannel()->getEventBase())
        .startInlineUnsafe([this, cb = std::move(cb), request = request_](
                               auto response) {
          try {
            CHECK_EQ(
                static_cast<uint32_t>(*request.x()) + *request.y(),
                *response->x_ref());
            CHECK_EQ(
                static_cast<uint32_t>(*request.x()) - *request.y(),
                *response->y_ref());
            stats_->add(op_name_);
            cb->replyReceived({});
          } catch (const apache::thrift::TApplicationException& ex) {
            if (ex.getType() ==
                apache::thrift::TApplicationException::
                    TApplicationExceptionType::TIMEOUT) {
              stats_->add(timeout_);
            } else {
              FB_LOG_EVERY_MS(ERROR, 1000)
                  << "Error should have caused error() function to be called: "
                  << ex.what();
              stats_->add(error_);
            }
            cb->replyReceived({});
          } catch (const std::exception& ex) {
            FB_LOG_EVERY_MS(ERROR, 1000) << "Critical error: " << ex.what();
            stats_->add(fatal_);
            cb->requestError({});
          }
        });
  }

 private:
  QPSStats* stats_;
  std::string op_name_ = "co_sum";
  std::string timeout_ = "timeout";
  std::string error_ = "error";
  std::string fatal_ = "fatal";
  TwoInts request_;
  TwoInts response_;
  std::mt19937 gen_{std::random_device()()};
};
