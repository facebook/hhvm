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

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <folly/init/Init.h>

#include <common/services/cpp/ServiceFramework.h>
#include <thrift/test/integration/cpp2/gen-cpp2/ZeroCopyService.h>

DEFINE_int32(port, 7878, "Port for the thrift server");
DEFINE_int32(threshold, 32 * 1024, "Zerocopy threshold");
DEFINE_bool(debug_logs, false, "Debug logs");

DEFINE_int32(read_buffer_allocation_size, -1, "readBufferAllocationSize");
DEFINE_int32(read_buffer_min_read_size, -1, "readBufferMinReadSize");

DEFINE_int32(read_mode, -1, "readMode - ReadBuffer = 0, ReadVec = 1");

DEFINE_int32(read_vec_block_size, 64 * 1024, "readVecBlockSize");
DEFINE_int32(read_vec_read_size, 32 * 1024, "readVecReadSize");

using namespace thrift::zerocopy::cpp2;

namespace {
class ServerIOVecQueue : public fizz::AsyncFizzBase::IOVecQueueOps {
 public:
  ServerIOVecQueue(size_t readVecBlockSize, size_t readVecReadSize)
      : readVecBlockSize_(readVecBlockSize),
        readVecReadSize_(readVecReadSize) {}
  ~ServerIOVecQueue() override = default;
  void allocateBuffers(folly::IOBufIovecBuilder::IoVecVec& iovs) override {
    if (FOLLY_UNLIKELY(!ioVecQueue_)) {
      ioVecQueue_.reset(new folly::IOBufIovecBuilder(
          folly::IOBufIovecBuilder::Options().setBlockSize(readVecBlockSize_)));
    }
    if (FLAGS_debug_logs) {
      LOG(INFO) << this << " allocateBuffers(" << readVecReadSize_ << ")";
    }
    ioVecQueue_->allocateBuffers(iovs, readVecReadSize_);
  }

  std::unique_ptr<folly::IOBuf> extractIOBufChain(size_t len) override {
    DCHECK(!!ioVecQueue_);
    if (FLAGS_debug_logs) {
      LOG(INFO) << this << "extractIOBufChain(" << len << ")";
    }
    return ioVecQueue_->extractIOBufChain(len);
  }

 private:
  size_t readVecBlockSize_;
  size_t readVecReadSize_;
  folly::ThreadLocalPtr<folly::IOBufIovecBuilder> ioVecQueue_;
};
class ZeroCopyServiceImpl
    : public apache::thrift::ServiceHandler<ZeroCopyService>,
      public ::facebook::fb303::FacebookBase2DeprecationMigration {
 public:
  ZeroCopyServiceImpl()
      : ::facebook::fb303::FacebookBase2DeprecationMigration("Zerocopy") {}
  ~ZeroCopyServiceImpl() override = default;

  void echo(IOBuf& ret, std::unique_ptr<IOBuf> data) override {
    ret = data->cloneAsValue();
    if (FLAGS_debug_logs) {
      LOG(INFO) << "[" << num_ << "]: data = " << data->countChainElements()
                << ":" << data->computeChainDataLength()
                << " ret = " << ret.countChainElements() << ":"
                << ret.computeChainDataLength();

      size_t i = 0;

      IOBuf* current = data.get();
      do {
        LOG(INFO) << i << ":" << static_cast<const void*>(current->buffer())
                  << ":" << current->length() << ":" << current->capacity();
        current = current->next();
        ++i;
      } while (current != data.get());
    }
    num_++;
  }
  facebook::fb303::cpp2::fb_status getStatus() override {
    return facebook::fb303::cpp2::fb_status::ALIVE;
  }

 private:
  size_t num_{0};
};
} // namespace

int main(int argc, char* argv[]) {
  struct rlimit rlim = {
      .rlim_cur = RLIM_INFINITY,
      .rlim_max = RLIM_INFINITY,
  };
  setrlimit(RLIMIT_MEMLOCK, &rlim); // best effort

  folly::init(&argc, &argv);

  LOG(INFO) << "Running on port " << FLAGS_port;

  auto handler = std::make_shared<ZeroCopyServiceImpl>();

  auto server = std::make_shared<apache::thrift::ThriftServer>();
  facebook::services::TLSConfig::applyDefaultsToThriftServer(*server);

  server->setSSLPolicy(apache::thrift::SSLPolicy::PERMITTED);

  fizz::AsyncFizzBase::TransportOptions transportOptions;

  if (FLAGS_read_buffer_allocation_size > 0) {
    transportOptions.readBufferAllocationSize =
        FLAGS_read_buffer_allocation_size;
  }

  if (FLAGS_read_buffer_min_read_size > 0) {
    transportOptions.readBufferMinReadSize = FLAGS_read_buffer_min_read_size;
  }

  if (FLAGS_read_mode ==
      static_cast<int>(folly::AsyncReader::ReadCallback::ReadMode::ReadVec)) {
    auto ioVecQueue = std::make_shared<ServerIOVecQueue>(
        FLAGS_read_vec_block_size, FLAGS_read_vec_read_size);
    transportOptions.ioVecQueue = ioVecQueue;
    transportOptions.readMode =
        folly::AsyncReader::ReadCallback::ReadMode::ReadVec;
  }

  if (FLAGS_threshold > 0) {
    LOG(INFO) << "Adding zerocopy enable func with threshold = "
              << FLAGS_threshold;
    server->setZeroCopyEnableFunc([](const std::unique_ptr<folly::IOBuf>& buf) {
      auto len = static_cast<int>(buf->computeChainDataLength());
      if (FLAGS_debug_logs) {
        LOG(INFO) << len << "-" << FLAGS_threshold;
      }
      return len >= FLAGS_threshold;
    });
  }
  server->setInterface(handler);
  server->setPort(FLAGS_port);

  facebook::services::ServiceFramework instance("ZeroCopyServer");

  facebook::services::ServiceFramework::ServerOptions options;
  options.transportOptions = transportOptions;

  // TODO(T123377436) CodeFrameworks Migration - Binary Contract
  instance.addPrimaryThriftService(server, handler.get(), options);
  instance.go();

  return 0;
}
