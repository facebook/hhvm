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

#include <sys/resource.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <folly/init/Init.h>

#include <folly/experimental/io/IoUringBackend.h>
#include <folly/io/async/AsyncSignalHandler.h>

#include <folly/experimental/io/MuxIOThreadPoolExecutor.h>

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

DEFINE_int32(zc_rx_num_entries, -1024, "ZC RX num entries");
DEFINE_int32(zc_rx_entry_size, 128 * 1024, "ZC RX entry size");

DEFINE_bool(napi_id_assign, false, "Use NAPI ID based socket assignment");
DEFINE_int32(io_threads, 0, "Number of IO threads (0 == number of cores)");

DEFINE_bool(io_uring, false, "Enables io_uring if available when set to true");

DEFINE_int32(size, 0, "Payload size");

// io_uring related
DEFINE_bool(use_iouring_event_eventfd, true, "");
DEFINE_int32(io_capacity, 0, "");
DEFINE_int32(io_submit_sqe, 0, "");
DEFINE_int32(io_max_get, 0, "");
DEFINE_bool(set_iouring_defer_taskrun, true, "");
DEFINE_int32(io_max_submit, 0, "");
DEFINE_int32(io_registers, 2048, "");
DEFINE_int32(io_prov_buffs_size, 2048, "");
DEFINE_int32(io_prov_buffs, 2000, "");

// mux thread pool related
DEFINE_bool(mux_io_tp_enable, false, "enable mux I/O thread pool");
DEFINE_int32(mux_io_tp_num_evbs, 16, "");
DEFINE_int32(mux_io_tp_num_threads, 16, "");
DEFINE_int32(mux_io_tp_num_max_events, 64, "");
DEFINE_int32(mux_io_tp_num_wakeup_us, 200, "");

using namespace thrift::zerocopy::cpp2;

namespace {
folly::IoUringBackend::Options getIoUringOptions() {
  folly::IoUringBackend::Options options;
  options.setRegisterRingFd(FLAGS_use_iouring_event_eventfd);

  if (FLAGS_io_prov_buffs_size > 0 && FLAGS_io_prov_buffs > 0) {
    options.setInitialProvidedBuffers(
        FLAGS_io_prov_buffs_size, FLAGS_io_prov_buffs);
  }

  if (FLAGS_io_registers > 0) {
    options.setUseRegisteredFds(static_cast<size_t>(FLAGS_io_registers));
  }

  if (FLAGS_io_capacity > 0) {
    options.setCapacity(static_cast<size_t>(FLAGS_io_capacity));
  }

  if (FLAGS_io_submit_sqe > 0) {
    options.setSqeSize(FLAGS_io_submit_sqe);
  }

  if (FLAGS_io_max_get > 0) {
    options.setMaxGet(static_cast<size_t>(FLAGS_io_max_get));
  }

  if (FLAGS_io_max_submit > 0) {
    options.setMaxSubmit(static_cast<size_t>(FLAGS_io_max_submit));
  }

  if (FLAGS_set_iouring_defer_taskrun) {
    if (folly::IoUringBackend::kernelSupportsDeferTaskrun()) {
      options.setDeferTaskRun(FLAGS_set_iouring_defer_taskrun);
    } else {
      LOG(ERROR) << "not setting DeferTaskRun as not supported on this kernel";
    }
  }
  return options;
}

std::unique_ptr<folly::EventBaseBackendBase> getEventBaseBackendFunc() {
  try {
    // TODO numa node affinitization
    // static int sqSharedCore = 0;
    // LOG(INFO) << "Sharing eb sq poll on core: " << sqSharedCore;
    // options.setSQGroupName("fast_eb").setSQCpu(sqSharedCore);
    return std::make_unique<folly::IoUringBackend>(getIoUringOptions());
  } catch (const std::exception& ex) {
    LOG(FATAL) << "Failed to create io_uring backend: "
               << folly::exceptionStr(ex);
  }
}

std::shared_ptr<folly::IOThreadPoolExecutorBase> getIOThreadPool(
    const std::string& name, size_t numThreads) {
  LOG(INFO) << "mux_io_tp_enable = " << FLAGS_mux_io_tp_enable;
  if (FLAGS_mux_io_tp_enable) {
    LOG(INFO) << "numThreads = " << numThreads;
    LOG(INFO) << "mux_io_tp_num_threads = " << FLAGS_mux_io_tp_num_threads;
    LOG(INFO) << "mux_io_tp_num_evbs = " << FLAGS_mux_io_tp_num_evbs;
    LOG(INFO) << "mux_io_tp_num_wakeup_us = " << FLAGS_mux_io_tp_num_wakeup_us;
    LOG(INFO) << "mux_io_tp_num_max_events = "
              << FLAGS_mux_io_tp_num_max_events;

    folly::MuxIOThreadPoolExecutor::Options options;
    options.setNumEventBases(FLAGS_mux_io_tp_num_evbs);
    options.setWakeUpInterval(
        std::chrono::microseconds(FLAGS_mux_io_tp_num_wakeup_us));
    options.setMaxEvents(FLAGS_mux_io_tp_num_max_events);
    auto pool = std::make_shared<folly::MuxIOThreadPoolExecutor>(
        (numThreads > 0) ? numThreads : FLAGS_mux_io_tp_num_threads, options);

    return pool;
  }

  auto threadFactory = std::make_shared<folly::NamedThreadFactory>(name);
  if (FLAGS_io_uring) {
    LOG(INFO) << "using io_uring EventBase backend";
    folly::EventBaseBackendBase::FactoryFunc func(getEventBaseBackendFunc);
    static folly::EventBaseManager ebm(
        folly::EventBase::Options().setBackendFactory(std::move(func)));

    auto* evb = folly::EventBaseManager::get()->getEventBase();
    // use the same EventBase for the main thread
    if (evb) {
      ebm.setEventBase(evb, false);
    }
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, threadFactory, &ebm);
  } else {
    return std::make_shared<folly::IOThreadPoolExecutor>(
        numThreads, threadFactory);
  }
}

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

  void async_eb_echo(
      std::unique_ptr<apache::thrift::HandlerCallback<
          std::unique_ptr<::thrift::zerocopy::cpp2::IOBuf>>> callback,
      std::unique_ptr<::thrift::zerocopy::cpp2::IOBuf> data) override {
    std::unique_ptr<::thrift::zerocopy::cpp2::IOBuf> ret;
    if (FLAGS_size <= 0) {
      ret = data->clone();
    } else {
      ret = ::thrift::zerocopy::cpp2::IOBuf::create(FLAGS_size);
      ret->append(FLAGS_size);
    }

    if (FLAGS_debug_logs) {
      LOG(INFO) << "[" << num_ << "]: data = " << data->countChainElements()
                << ":" << data->computeChainDataLength()
                << " ret = " << ret->countChainElements() << ":"
                << ret->computeChainDataLength();

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

    callback->result(std::move(data));
  }
  facebook::fb303::cpp2::fb_status getStatus() override {
    return facebook::fb303::cpp2::fb_status::ALIVE;
  }

 private:
  size_t num_{0};
};

class ShutdownSignalHandler : public folly::AsyncSignalHandler {
 public:
  explicit ShutdownSignalHandler(facebook::services::ServiceFramework& instance)
      : AsyncSignalHandler(nullptr), instance_(instance) {
    signalThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
      attachEventBase(signalThread_.getEventBase());
      registerSignalHandler(SIGTERM);
      registerSignalHandler(SIGINT);
    });
    LOG(INFO) << "Installed shutdown signal handlers";
  }

 protected:
  void signalReceived(int signum) noexcept override {
    LOG(INFO) << "Starting shutdown because of signal " << signum << "..";
    unregisterSignalHandler(SIGTERM);
    unregisterSignalHandler(SIGINT);
    instance_.stop();
  }

 private:
  folly::ScopedEventBaseThread signalThread_{"SignalThread"};
  facebook::services::ServiceFramework& instance_;
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

  std::unique_ptr<folly::AsyncReader::ReadCallback::ZeroCopyMemStore>
      zeroCopyMemStore;

  if ((FLAGS_zc_rx_num_entries) > 0 && (FLAGS_zc_rx_entry_size > 0)) {
    zeroCopyMemStore = folly::AsyncSocket::createDefaultZeroCopyMemStore(
        FLAGS_zc_rx_num_entries, FLAGS_zc_rx_entry_size);
  }

  LOG(INFO) << "zeroCopyMemStore(" << FLAGS_zc_rx_num_entries << ","
            << FLAGS_zc_rx_entry_size << ") = " << zeroCopyMemStore.get();

  auto handler = std::make_shared<ZeroCopyServiceImpl>();

  auto server = std::make_shared<apache::thrift::ThriftServer>();

  if (FLAGS_napi_id_assign) {
    LOG(INFO) << "Enabling NAPI ID based assignment";

    folly::AsyncServerSocket::CallbackAssignFunction callbackAssignFunc =
        [](folly::AsyncServerSocket*, folly::NetworkSocket sock) -> int {
      int id = -1;
      socklen_t len = sizeof(id);
      auto ret =
          ::getsockopt(sock.toFd(), SOL_SOCKET, SO_INCOMING_NAPI_ID, &id, &len);

      LOG(INFO) << "NAPI ID for " << sock.toFd() << " = " << id
                << " ret = " << ret;

      if (ret < 0) {
        return -1;
      } else {
        return id;
      }
    };
    server->setCallbackAssignFunc(callbackAssignFunc);
  }

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

  transportOptions.zeroCopyMemStore = zeroCopyMemStore.get();

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
  server->setPreferIoUring(FLAGS_io_uring);
  server->setIOThreadPool(
      getIOThreadPool("thrift_eventbase", FLAGS_io_threads));

  facebook::services::ServiceFramework instance("ZeroCopyServer");

  facebook::services::ServiceFramework::ServerOptions options;
  options.transportOptions = transportOptions;

  // TODO(T123377436) CodeFrameworks Migration - Binary Contract
  instance.addPrimaryThriftService(server, handler.get(), options);
  ShutdownSignalHandler shutdownHandler(instance);
  instance.go();

  return 0;
}
