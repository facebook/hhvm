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

#include <folly/MoveWrapper.h>
#include <folly/portability/GMock.h>
#include <wangle/channel/Handler.h>

namespace wangle {

template <class Rin, class Rout = Rin, class Win = Rout, class Wout = Rin>
class MockHandler : public Handler<Rin, Rout, Win, Wout> {
 public:
  using Context = typename Handler<Rin, Rout, Win, Wout>::Context;

  MockHandler() = default;
  MockHandler(MockHandler&&) = default;

  MOCK_METHOD2_T(read_, void(Context*, Rin&));
  MOCK_METHOD1_T(readEOF, void(Context*));
  MOCK_METHOD2_T(readException, void(Context*, folly::exception_wrapper));

  MOCK_METHOD2_T(write_, void(Context*, Win&));
  MOCK_METHOD1_T(close_, void(Context*));
  MOCK_METHOD2_T(writeException_, void(Context*, folly::exception_wrapper));

  MOCK_METHOD1_T(attachPipeline, void(Context*));
  MOCK_METHOD1_T(attachTransport, void(Context*));
  MOCK_METHOD1_T(detachPipeline, void(Context*));
  MOCK_METHOD1_T(detachTransport, void(Context*));

  void read(Context* ctx, Rin msg) override {
    read_(ctx, msg);
  }

  folly::Future<folly::Unit> write(Context* ctx, Win msg) override {
    return folly::makeFutureWith([&]() { write_(ctx, msg); });
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    return folly::makeFutureWith([&]() { close_(ctx); });
  }

  folly::Future<folly::Unit> writeException(
      Context* ctx,
      folly::exception_wrapper ex) override {
    return folly::makeFutureWith([&]() { writeException_(ctx, ex); });
  }
};

template <class R, class W = R>
using MockHandlerAdapter = MockHandler<R, R, W, W>;

class MockBytesToBytesHandler : public BytesToBytesHandler {
 public:
  folly::MoveWrapper<folly::Future<folly::Unit>> defaultFuture() {
    return folly::MoveWrapper<folly::Future<folly::Unit>>();
  }

  MOCK_METHOD1(transportActive, void(Context*));
  MOCK_METHOD1(transportInactive, void(Context*));
  MOCK_METHOD2(read, void(Context*, folly::IOBufQueue&));
  MOCK_METHOD1(readEOF, void(Context*));
  MOCK_METHOD2(readException, void(Context*, folly::exception_wrapper));
  MOCK_METHOD2(
      write,
      folly::MoveWrapper<folly::Future<folly::Unit>>(
          Context*,
          std::shared_ptr<folly::IOBuf>));
  MOCK_METHOD1(
      mockClose,
      folly::MoveWrapper<folly::Future<folly::Unit>>(Context*));
  MOCK_METHOD2(
      mockWriteException,
      folly::MoveWrapper<folly::Future<folly::Unit>>(
          Context*,
          folly::exception_wrapper));

  folly::Future<folly::Unit> write(
      Context* ctx,
      std::unique_ptr<folly::IOBuf> buf) override {
    std::shared_ptr<folly::IOBuf> sbuf(buf.release());
    return write(ctx, sbuf).move();
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    return mockClose(ctx).move();
  }

  folly::Future<folly::Unit> writeException(
      Context* ctx,
      folly::exception_wrapper ex) override {
    return mockWriteException(ctx, ex).move();
  }
};

} // namespace wangle
