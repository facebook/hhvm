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

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/python/test/interactions/gen-cpp2/BlankService.h>
#include <thrift/lib/python/test/interactions/gen-cpp2/Calculator.h>

namespace interactions::test::thrift {
struct SemiCalculatorHandler : apache::thrift::ServiceHandler<Calculator> {
  struct SemiAdditionHandler
      : apache::thrift::ServiceHandler<Calculator>::AdditionIf {
    int acc_{0};
    Point pacc_;

    folly::SemiFuture<folly::Unit> semifuture_accumulatePrimitive(
        int32_t a) override {
      acc_ += a;
      return folly::makeSemiFuture();
    }
    folly::SemiFuture<folly::Unit> semifuture_noop() override {
      return folly::makeSemiFuture();
    }
    folly::SemiFuture<folly::Unit> semifuture_accumulatePoint(
        std::unique_ptr<::interactions::test::thrift::Point> a) override {
      *pacc_.x() += *a->x();
      *pacc_.y() += *a->y();
      return folly::makeSemiFuture();
    }
    folly::SemiFuture<int32_t> semifuture_getPrimitive() override {
      return acc_;
    }
    folly::SemiFuture<std::unique_ptr<::interactions::test::thrift::Point>>
    semifuture_getPoint() override {
      return folly::copy_to_unique_ptr(pacc_);
    }
  };

  std::unique_ptr<AdditionIf> createAddition() override {
    return std::make_unique<SemiAdditionHandler>();
  }

  folly::SemiFuture<int32_t> semifuture_addPrimitive(
      int32_t a, int32_t b) override {
    return a + b;
  }

  void async_tm_newAddition(
      apache::thrift::HandlerCallbackPtr<
          apache::thrift::TileAndResponse<AdditionIf, void>> cb) override {
    auto handler =
        std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
    cb->result({std::move(handler)});
  }

  void async_tm_initializedAddition(
      apache::thrift::HandlerCallbackPtr<
          apache::thrift::TileAndResponse<AdditionIf, int>> cb,
      int x) override {
    auto handler =
        std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
    handler->acc_ = x;
    cb->result({std::move(handler), x});
  }

  void async_tm_stringifiedAddition(
      apache::thrift::HandlerCallbackPtr<apache::thrift::TileAndResponse<
          AdditionIf,
          std::unique_ptr<std::string>>> cb,
      int x) override {
    auto handler =
        std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
    handler->acc_ = x;
    cb->result(
        {std::move(handler), folly::copy_to_unique_ptr(std::to_string(x))});
  }
};
struct SemiBlankServiceHandler
    : apache::thrift::ServiceHandler<BlankServiceRenamed> {};
} // namespace interactions::test::thrift
