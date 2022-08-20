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

#include <thrift/lib/cpp/transport/THeader.h>
#include <wangle/channel/Handler.h>
#include <wangle/channel/StaticPipeline.h>

namespace apache {
namespace thrift {

class FramingHandler
    : public wangle::Handler<
          folly::IOBufQueue&,
          std::pair<
              std::unique_ptr<folly::IOBuf>,
              std::unique_ptr<apache::thrift::transport::THeader>>,
          std::pair<
              std::unique_ptr<folly::IOBuf>,
              apache::thrift::transport::THeader*>,
          std::unique_ptr<folly::IOBuf>> {
 public:
  ~FramingHandler() override {}

  /**
   * If q contains enough data, read it (removing it from q, but retaining
   * following data), unframe it and pass it to the next pipeline stage.
   *
   * If q doesn't contain enough data, set the read buffer settings
   * to the data remaining to get a full frame, then return *without*
   * calling the next pipeline stage.
   */
  void read(Context* ctx, folly::IOBufQueue& q) override;

  folly::Future<folly::Unit> write(
      Context* ctx,
      std::pair<
          std::unique_ptr<folly::IOBuf>,
          apache::thrift::transport::THeader*> bufAndHeader) override;

  virtual std::tuple<
      std::unique_ptr<folly::IOBuf>,
      size_t,
      std::unique_ptr<apache::thrift::transport::THeader>>
  removeFrame(folly::IOBufQueue* q) = 0;

  /**
   * Wrap an IOBuf in any headers/footers
   */
  virtual std::unique_ptr<folly::IOBuf> addFrame(
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::transport::THeader* header) = 0;

  /**
   * Set read buffer size.
   *
   * @param readBufferSize   The read buffer size to set
   * @param strict           True means given size will always be used; false
   *                         means given size may not be used if it is too small
   */
  void setReadBufferSize(size_t readBufferSize, bool strict = false) {
    readBufferSize_ = strict
        ? readBufferSize
        : std::max<size_t>(readBufferSize, DEFAULT_BUFFER_SIZE);
  }

  folly::Future<folly::Unit> close(Context* ctx) override;

 private:
  enum BUFFER_SIZE {
    DEFAULT_BUFFER_SIZE = 2048,
  };

  size_t readBufferSize_{DEFAULT_BUFFER_SIZE};

  bool closing_{false};
};

} // namespace thrift
} // namespace apache
