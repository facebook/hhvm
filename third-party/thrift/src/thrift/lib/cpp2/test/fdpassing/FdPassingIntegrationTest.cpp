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

#ifdef __linux__

#include <linux/kcmp.h>

#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/fdsock/AsyncFdSocket.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>

#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/fdpassing/gen-cpp2/DemoService.h>
#include <thrift/lib/cpp2/test/fdpassing/gen-cpp2/DemoServiceAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

namespace {

int kcmp(pid_t pid1, pid_t pid2, int type, unsigned long i1, unsigned long i2) {
  return syscall(SYS_kcmp, pid1, pid2, type, i1, i2);
}

// Returns the number of FDs echoed back to the client.
size_t serverEchoFds(apache::thrift::Cpp2RequestContext* reqCtx) {
  LOG(INFO) << "Echoing FDs for req context " << reqCtx;
  auto& fds = reqCtx->getHeader()->fds;
  CHECK(!fds.empty()) << "This server always expects to get FDs";
  auto inFds = fds.releaseReceived();

  folly::SocketFds::ToSend outFds;
  for (auto& fd : inFds) {
    LOG(INFO) << " |--> FD: " << fd.fd();
    outFds.emplace_back(std::make_shared<folly::File>(std::move(fd)));
  }

  // Include FDs with the response to the client.
  reqCtx->getHeader()->fds.dcheckEmpty() = folly::SocketFds{std::move(outFds)};

  return inFds.size();
}

// Checks that the server echoed back the FD we needed, and returns the
// `goodbye()` string -- checked in the test body to ensure the client
// callback actually ran.
std::string clientVerifyServerFdEcho(
    int expectedFd,
    const facebook::demo::DemoResponse& res,
    apache::thrift::transport::THeader& header) {
  LOG(INFO) << "server said: " << *res.goodbye();

  auto my_pid = getpid();
  // We'll read procfs symlinks for each received file, and compare them to
  // the value seen for `expectedFd`.
  std::array<char, PATH_MAX> expectedMagic{}; // readlink doesn't NUL-terminate
  PCHECK(
      -1 !=
      ::readlink(
          fmt::format("/proc/{}/fd/{}", my_pid, expectedFd).c_str(),
          expectedMagic.data(),
          PATH_MAX));

  auto fds = header.fds.releaseReceived();
  CHECK_EQ(1, fds.size());
  for (const auto& fd : fds) {
    std::array<char, PATH_MAX> fdMagic{}; // readlink doesn't NUL-terminate
    PCHECK(
        -1 !=
        ::readlink(
            fmt::format("/proc/{}/fd/{}", my_pid, fd.fd()).c_str(),
            fdMagic.data(),
            PATH_MAX));
    LOG(INFO) << " |--> FD: " << fd.fd() << " (" << fdMagic.data() << " vs "
              << expectedMagic.data() << ")";
    CHECK_EQ(std::string(fdMagic.data()), std::string(expectedMagic.data()));

    // Comparing proc symlinks is neat, but to be Technically Correct (TM),
    // check that the FDs refer to the same kernel file description object.
    long ret = ::kcmp(my_pid, my_pid, KCMP_FILE, fd.fd(), expectedFd);
    if (ret == -1) {
      // The current kernel may not support CONFIG_KCMP
      PCHECK(errno == ENOSYS);
    } else {
      CHECK_EQ(ret, 0) << "File description objects are not equal";
    }
  }
  return *res.goodbye();
}

} // namespace

class DemoServiceHandler final
    : virtual public facebook::demo::DemoServiceSvIf {
 public:
  folly::coro::Task<std::unique_ptr<facebook::demo::DemoResponse>> co_demo(
      apache::thrift::RequestParams params,
      std::unique_ptr<facebook::demo::DemoRequest> request) override {
    // NB: We must NOT access `reqCtx->getConnectionContext()` in a handler,
    // since the underlying connection object may been destroyed by now
    // (e.g. due to idle timeout, or similar), making that pointer invalid.
    auto reqCtx = params.getRequestContext();

    LOG(INFO) << "demo(hello='" << *request->hello() << "')";
    size_t numFds = serverEchoFds(reqCtx);
    auto res = std::make_unique<facebook::demo::DemoResponse>();
    res->goodbye() = fmt::format("{}bye/numFds={}", *request->hello(), numFds);

    co_return res;
  }
  // TODO: Add coverage for stream & one-way (fnf) request types.
};

auto makeServerThread(const std::string& sockPath) {
  auto thread = std::make_shared<apache::thrift::util::ScopedServerThread>();

  auto server = std::make_shared<apache::thrift::ThriftServer>();
  auto handler = std::make_shared<DemoServiceHandler>();
  server->setInterface(handler);
  {
    folly::AsyncServerSocket::UniquePtr sock{new folly::AsyncServerSocket};
    folly::SocketAddress addr;
    addr.setFromPath(sockPath);
    sock->bind(addr);
    server->useExistingSocket(std::move(sock));
  }
  server->setNumIOWorkerThreads(1);
  server->setNumCPUWorkerThreads(1);

  thread->start(server, [=]() {});
  thread->setServeThreadName("fd-demo-server");

  return thread;
}

TEST(FdPassingIntegrationTest, clientToServerAndBack) {
  folly::test::TemporaryDirectory tempDir;
  auto sockPath = (tempDir.path() / "fd-passing-test-socket").native();
  auto serverThread = makeServerThread(sockPath);

  auto evb = folly::EventBaseManager::get()->getEventBase();

  folly::SocketAddress addr;
  addr.setFromPath(sockPath);

  auto client = apache::thrift::Client<facebook::demo::DemoService>(
      apache::thrift::RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr{new folly::AsyncFdSocket(evb, addr)}));

  facebook::demo::DemoRequest req;
  req.hello() = "kthx";

  const int fdToPass = 2;
  apache::thrift::RpcOptions rpcOptions;
  {
    folly::SocketFds::ToSend fds;
    fds.emplace_back(
        std::make_shared<folly::File>(folly::File(fdToPass, /*owns*/ false)));
    rpcOptions.setFdsToSend(std::move(fds)); // Send FDs to server
  }
  std::string serverSaid;

  // This requires `@cpp.GenerateDeprecatedHeaderClientMethods` on the method.
  client.header_semifuture_demo(rpcOptions, req)
      .via(evb)
      .thenTry([&serverSaid, evb](auto maybeRet) {
        CHECK(!maybeRet.hasException()) << maybeRet.exception().what();
        auto [res, header] = std::move(*maybeRet);
        serverSaid = clientVerifyServerFdEcho(fdToPass, res, *header);
        evb->terminateLoopSoon();
      });
  evb->loopForever();
  serverThread->stop();

  // At test scope to ensure that the callback actually ran.
  EXPECT_EQ(serverSaid, "kthxbye/numFds=1");
}

#endif
